#include "locomotion/ControlLoop.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log_timestamp.h"
#include "Robot.hpp"
#include "common/Log.hpp"
#include "common/config.hpp"
#include "common/RPC.hpp"
#include "locomotion/IPC.hpp"
#include "drivers/AnalogDriver.hpp"
#include "drivers/MotorDriver.hpp"
#include "drivers/IMUDriver.hpp"

TaskHandle_t timer_task_handle;
bool running = false;

static bool IRAM_ATTR timer_on_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
    BaseType_t high_task_awoken = pdFALSE;
    vTaskNotifyGiveFromISR(timer_task_handle, &high_task_awoken);
    return (high_task_awoken == pdTRUE); // return true if a higher priority task was awoken
}

ControlLoop::ControlLoop()
    : initialized(false)
{
    kinematics_engine = KinematicsEngine(KinematicsEngine::KinematicsConfig{
        .hip_shift_x = HIP_POS_X_M,
        .hip_shift_y = HIP_POS_Y_M,
        .hip_offset = HIP_OFFSET_M,
        .length_thigh = LEG_THIGH_LENGTH_M,
        .length_calf = LEG_CALF_LENGTH_M,
        .leg_inverted = { true, false, true, false },
    });
}

Error ControlLoop::init()
{
    if (initialized) return Error::None;

    // Create the control loop task ON REFLEX CORE
    BaseType_t err = xTaskCreatePinnedToCore([](void* PvParams){
        ControlLoop* control_loop = static_cast<ControlLoop*>(PvParams);
        running = true;
        
        // Main loop
        while (running)
        {
            // wait for timer interrupt notification
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

            control_loop->control_task();
        }

        // clean up and delete task
        running = false;
        if (timer_task_handle != nullptr)
        {
            vTaskDelete(nullptr);
            timer_task_handle = nullptr;
        }
    },  "timer_task", 8192, this, tskIDLE_PRIORITY + 10, &timer_task_handle, CORE_REFLEX);

    if (err != pdPASS)
    {
        Log::Add(Log::Level::Error, TAG, "Failed to create timer task");
        return Error::SoftwareFailure;
    }

    // Configure the timer
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = TIMER_RESOLUTION,
    };
    if (esp_err_t err = gptimer_new_timer(&timer_config, &timer); err != ESP_OK)
    {
        Log::Add(Log::Level::Error, TAG, "gptimer_new_timer failed");
        return Error::SoftwareFailure;
    }

    gptimer_event_callbacks_t cbs = {
        .on_alarm = timer_on_alarm_cb, // link the alarm callback
    };
    if (esp_err_t err = gptimer_register_event_callbacks(timer, &cbs, NULL); err != ESP_OK)
    {
        Log::Add(Log::Level::Error, TAG, "gptimer_register_event_callbacks failed");
        return Error::SoftwareFailure;
    }

    gptimer_alarm_config_t alarm_config = {
        .alarm_count = TIMER_ALARM_COUNT,
        .reload_count = 0,
        .flags = {
            .auto_reload_on_alarm = true,
        },
    };
    if (esp_err_t err = gptimer_set_alarm_action(timer, &alarm_config); err != ESP_OK)
    {
        Log::Add(Log::Level::Error, TAG, "gptimer_set_alarm_action failed");
        return Error::SoftwareFailure;
    }

    // Enable the timer
    if (esp_err_t err = gptimer_enable(timer); err != ESP_OK)
    {
        Log::Add(Log::Level::Error, TAG, "gptimer_enable failed");
        return Error::SoftwareFailure;
    }

    initialized = true;
    return Error::None;
}

Error ControlLoop::start()
{
    if (esp_err_t err = gptimer_start(timer); err != ESP_OK)
    {
        Log::Add(Log::Level::Error, TAG, "Error starting Control Loop timer : 0x%0X", err);
        return Error::HardwareFailure;
    }

    return Error::None;
}


Error ControlLoop::stop()
{
    if (esp_err_t err = gptimer_stop(timer); err != ESP_OK)
    {
        Log::Add(Log::Level::Error, TAG, "Error stopping Control Loop timer : 0x%0X", err);
        return Error::HardwareFailure;
    }

    return Error::None;
}


Error ControlLoop::deinit()
{
    if (!initialized) return Error::None;

    running = false; // ask the control loop to end

    // wait 2 control cycle to be sure the control loop is stopped
    vTaskDelay(pdMS_TO_TICKS(CONTROL_LOOP_DT_MS*2));

    if (timer_task_handle != nullptr)
    {
        vTaskDelete(timer_task_handle);
        timer_task_handle = nullptr;
    }

    if (esp_err_t err = gptimer_disable(timer); err != ESP_OK)
    {
        Log::Add(Log::Level::Error, TAG, "Error disabling Control Loop gptimer ; 0x%0X", err);
        return Error::HardwareFailure;
    }

    initialized = false;
    return Error::None;
}

Error ControlLoop::control_task()
{
    // performance tracking
    perf_controltask.start();

    static bool watchdog_active = false;

    /*** UPDATE CONTROL INTENT ***/
    // update from IPC
    static IPC::ControlIntent intent;
    bool new_intent = IPC::getIntent(&intent);

    // security timestamp
    uint32_t current_time = esp_log_timestamp();
    if ((current_time - intent.timestamp_ms) > CONTROL_INTENT_WATCHDOG_MS)
    {
        // Brain is overloaded or crashed. Force robot stop.
        intent.body_vel = Vec3f(0.f, 0.f, 0.f);
        intent.gait = GaitPlanner::GaitType::Walk;

        if (!watchdog_active) {
            Log::Add(Log::Level::Warning, TAG, "Control intent watchdog triggered. Stop overthinking!");
            watchdog_active = true;
        }
    }
    else
    {
        if (watchdog_active) {
            Log::Add(Log::Level::Info, TAG, "Brain is back online!");
            watchdog_active = false;
        }
    }

    /*** 1 - STATE ESTIMATION - READ ALL SENSORS ***/

    // Read the ADC channels and IMU Data
    if (Error err = AnalogDriver::ReadAllChannels(); err != Error::None)
    {
        Log::Add(Log::Level::Error, TAG, "Error reading all ADC channels");
    }
    if (Error err = IMUDriver::ReadData(); err != Error::None)
    {
        Log::Add(Log::Level::Error, TAG, "Error reading data from IMU");
    }

    // Estimate body state from new IMU and Analog data (calls Legs, Joint, IMU estimateState functions)
    if (Error err = Robot::GetInstance().getBody().estimateState(CONTROL_LOOP_DT_S); err != Error::None)
    {
        Log::Add(Log::Level::Error, TAG, "Error estimating body state");
    }

    /*** 2 - RUN CARTESIAN CONTROL (USING BRAIN CONTROL INTENT) ***/

    // build the initial state
    BodyCartesianState cartesian_state;
    cartesian_state.body_pos = intent.body_pos;
    cartesian_state.body_rot = intent.body_rot;
    cartesian_state.legs[0].is_grounded = Robot::GetInstance().getBody().getFrontLeftLeg().isGrounded();
    cartesian_state.legs[1].is_grounded = Robot::GetInstance().getBody().getFrontRightLeg().isGrounded();
    cartesian_state.legs[2].is_grounded = Robot::GetInstance().getBody().getBackLeftLeg().isGrounded();
    cartesian_state.legs[3].is_grounded = Robot::GetInstance().getBody().getBackRightLeg().isGrounded();

    // Gait planner (ideal movement)
    if (new_intent)
    {
        gait_planner.setVelocityCommand(intent.body_vel.x, intent.body_vel.y, intent.body_vel.z);
        GaitPlanner::GaitConfig current_config = gait_planner.getConfig();
        if (current_config.gait_type != intent.gait)
        {
            current_config.gait_type = intent.gait;
            gait_planner.setGaitConfig(current_config);
        }
    }
    gait_planner.update(CONTROL_LOOP_DT_S, cartesian_state);

    // Animation override (breathing and all)
    // TODO

    /*** 3 - CONVERT CARTESIAN CONTROL TO JOINT CONTROL ***/

    BodyJointState joint_state;

    // IK
    if (Error err = kinematics_engine.computeBodyIK(cartesian_state, joint_state); err != Error::None)
    {
        Log::Add(Log::Level::Error, TAG, "KinematicsEngine failed to calculate body IK");
        return err;
    }


    /*** 4 - RUN JOINT CONTROL (USING BRAIN CONTROL INTENT) ***/

    // Joint Override


    /*** 5 - SEND EVERYTHING ***/

    // Update the body (this updates all joints in the body)
    if (Error err = Robot::GetInstance().getBody().applyCommand(joint_state, CONTROL_LOOP_DT_S); err != Error::None)
    {
        Log::Add(Log::Level::Error, TAG, "Failed to apply command in control task with error: %s", ErrorToString(err));
        // return err;
    }
    
    // Send the new motor values
    if (Error err = MotorDriver::SendData(); err != Error::None)
    {
        Log::Add(Log::Level::Error, TAG, "Error sending MotorDriver data");
    }

    /*** 6 - OTHER CORE1 JOBS ***/

    // All done, we can execute pending jobs if there's any (Handle RPC Calls)
    RPC::Process_Core1();
    
    // Performance tracking (as lightweight as possible)
    perf_controltask.stop();
    if (perf_counter++ == CONTROL_LOOP_FREQ_HZ)
    {
        avg_ms = perf_controltask.get_avg_ms(CONTROL_LOOP_FREQ_HZ);
        perf_controltask.reset();
        perf_counter = 0;
    }

    return Error::None;
}