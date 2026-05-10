#pragma once
#include "drivers/MotorDriver.hpp"
#include "drivers/AnalogDriver.hpp"
#include "common/NVS.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

class MotorController
{
public:
    enum class State : uint8_t
    {
        ENABLED = 1,
        DISABLED = 0,
        ERROR = 2,
    };

    enum class CalibrationState : uint8_t
    {
        UNCALIBRATED = 0,
        CALIBRATING = 1,
        CALIBRATED = 2,
        ERROR = 3,
    };

    struct MotorAttributes
    {
        MotorDriver::Value pwm_min;
        MotorDriver::Value pwm_max;
        bool has_feedback;
    };

    constexpr static MotorAttributes MOTOR_ATTR_MG996R = {
        .pwm_min = MotorDriver::MS_TO_PWM(0.5f), // 0.5us duty cycle
        .pwm_max = MotorDriver::MS_TO_PWM(2.5f), // 2.5us duty cycle
        .has_feedback = true
    };

    constexpr static MotorAttributes MOTOR_ATTR_SG90 = {
        .pwm_min = MotorDriver::MS_TO_PWM(1.0f), // 1.0us duty cycle
        .pwm_max = MotorDriver::MS_TO_PWM(2.0f), // 2.0us duty cycle
        .has_feedback = false
    };

    struct CalibrationData
    {
        MotorDriver::Value pwm_min = 0;
        MotorDriver::Value pwm_max = 0;
        MotorDriver::Value pwm_deadband = 0;
        AnalogDriver::Value feedback_min = 0000;
        AnalogDriver::Value feedback_max = 3300; // default to 0-3.3V range
        AnalogDriver::Value feedback_noise = 0;
        int feedback_latency_ms = 0;
        float max_speed = 0.f; // in progress per second
    };

    MotorController();
    
    MotorController(MotorDriver::Channel motor_channel, AnalogDriver::Channel analog_channel, MotorAttributes attribs = MOTOR_ATTR_MG996R);

    /**
     * @brief Initialize the motor controller.
     * @return Error code indicating success or failure.
     */
    Error init();

    /**
     * @brief Deinitialize the motor controller.
     * @return Error code indicating success or failure.
     */
    Error deinit();

    /**
     * @brief Turns on the motor.
     * @return Error code indicating success or failure.
     */
    Error enable();

    /**
     * @brief Turns off the motor.
     * @return Error code indicating success or failure.
     */
    Error disable();

    /**
     * @brief Starts the calibration process.
     * @return Error code indicating success or failure.
     */
    Error startCalibration();

    /**
     * @brief Stops the calibration process (no saving).
     * @note This method should not be called to save calibration data, it is done automatically on calibration sequence completion.
     * @return Error code indicating success or failure.
     */
    Error stopCalibration();

    /**
     * @brief Get the calibration progress as a ratio (0.0 to 1.0).
     * @return Calibration progress.
     */
    float getCalibrationProgress() const { return calibration_progress; }

    /**
     * @brief Set the calibration state.
     * @param state New calibration state.
     * @return Error code indicating success or failure.
     * @note Should only be used for manual calibration control. 
     */
    Error setCalibrationState(CalibrationState state);

    /**
     * @brief Get the current calibration data
     * @return Current calibration data
     */
    const CalibrationData& getCalibrationData() const { return calibration_data; }

    /**
     * @brief Set the current calibration data
     * @param data The CalibrationData object to set
     * @param save Should the data be saved in NVS for future use
     * @return If the save was successfull or not
     */
    Error setCalibrationData(CalibrationData& data, bool save = true);

    /**
     * @brief Delete the current calibration data (fallback to default)
     * @param save Should the deletion be saved in NVS or not
     * @return If the deletion was successfull or not
     */
    Error deleteCalibrationData(bool save = true);

    /**
     * @brief Set the target position for the motor.
     * @param position Target position as a ratio (0.0 to 1.0).
     * @note An approximate position can be set even if the motor is uncalibrated.
     * @return Error code indicating success or failure.
     */
    Error setTargetPosition(float position);

    /**
     * @brief Get the target position of the motor.
     * @param result Target position as a ratio (0.0 to 1.0).
     * @return Error code indicating success or failure.
     */
    Error getTargetPosition(float& result) const;

    /**
     * @brief Get the current position of the motor.
     * @param result Current position as a ratio (0.0 to 1.0).
     * @return Error code indicating success or failure.
     * @note An approximate position will be returned until the motor is calibrated.
     */
    Error getCurrentPosition(float& result) const;

    /**
     * @brief Get the current state of the motor controller.
     * @return Current state.
     */
    State getState() const { return state; }

    /**
     * @brief Get the current state of calibration.
     * @return Current calibration state.
     */
    CalibrationState getCalibrationState() const { return calibration_state; }

    /**
     * @brief Get the motor channel.
     * @return Motor channel.
     */
    MotorDriver::Channel getMotorChannel() const { return motor_channel; }

    /**
     * @brief Get the analog channel.
     * @return Analog channel.
     */
    AnalogDriver::Channel getAnalogChannel() const { return analog_channel; }

private:
    MotorDriver::Channel motor_channel;
    AnalogDriver::Channel analog_channel;

    MotorAttributes motor_attributes;
    CalibrationData calibration_data;
    CalibrationState calibration_state;
    float calibration_progress;
    
    State state;
    float target_position = 0;
    NVS::Handle* nvshandle_ptr;

    TaskHandle_t calibration_task_handle;

    Error __send_target_position();
    Error save_calibration_data();
    Error delete_calibration_data();
    Error run_calibration_sequence();
};