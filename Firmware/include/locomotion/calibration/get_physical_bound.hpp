#pragma once
#include <freertos/FreeRTOS.h>
#include "common/Log.hpp"
#include "common/utils.hpp"
#include "common/analysis/ArrayStats.hpp"
#include "common/analysis/FastRegression.hpp"
#include "drivers/AnalogDriver.hpp"
#include "drivers/MotorDriver.hpp"
#include <cmath>

struct PhysicalBoundParams
{
    /// @brief PWM Value to use to set servomotor to its center position
    MotorDriver::Value pwm_center = MotorDriver::MS_TO_PWM(1.5);
    /// @brief Maximum PWM value to test (to avoid going too far)
    MotorDriver::Value pwm_max = MotorDriver::MS_TO_PWM(2.6);
    /// @brief Minimum PWM value to test (to avoid going too far backwards)
    MotorDriver::Value pwm_min = MotorDriver::MS_TO_PWM(0.4);
    /// @brief Safe zone in PWM value around the center position where we are sure to be within bounds 
    MotorDriver::Value pwm_safe = MotorDriver::MS_TO_PWM(0.5);
    /// @brief PWM step gap to use for the fast approach phase
    MotorDriver::Value pwm_faststep = MotorDriver::MS_TO_PWM(0.01); 
    /// @brief PWM step gap to use for the slow approach phase
    MotorDriver::Value pwm_slowstep = MotorDriver::MS_TO_PWM(0.002);
    /// @brief PWM backoff to apply after detecting the bound with the fast approach, before starting the slow approach
    MotorDriver::Value pwm_backoff = MotorDriver::MS_TO_PWM(0.15);
    /// @brief Direction in which to search for the bound (1 for max bound, -1 for min bound)
    int direction = 1;
    /// @brief Number of subsamples to take for each feedback reading in the fast approach
    uint32_t feedback_subsamples_fast = 40;
    /// @brief Number of subsamples to take for each feedback reading in the slow approach
    uint32_t feedback_subsamples_slow = 80;
    /// @brief Motor's feedback noise
    AnalogDriver::Value feedback_noise;
    /// @brief Motor's feedback latency in ms
    float feedback_latency_ms = 20.f;
    /// @brief Base delay in ms for centering the motor at the start
    uint32_t base_delay_ms = 800;
    /// @brief Backoff delay in ms after detecting the bound with the fast approach
    uint32_t backoff_delay_ms = 300;
};

struct BoundDescription
{
    MotorDriver::Value pwm_value; // PWM value at which the bound is detected
    AnalogDriver::Value feedback_value; // Feedback value at which the bound is detected
};

/**
 * @brief Get a physical bound (min or max) of a motor by moving it to the limit
 * @param params Physical bound estimation parameters
 * @param motor_channel Channel used for motor control
 * @param analog_channel Channel used for motor feedback
 * @param out_value [out] Description of the detected bound
 * @note Returned value is the median of the sampled deadband size values
 */
Error get_physical_bound(PhysicalBoundParams params, MotorDriver::Channel motor_channel, AnalogDriver::Channel analog_channel, BoundDescription& out_value)
{   
    AnalogDriver::internal::select(analog_channel);
    vTaskDelay(pdMS_TO_TICKS(1));

    // =========================================================================
    // STEP 0 : Profiling inside Safe Zone
    // =========================================================================
    FastRegression regression;
    float stall_threshold;
    float slope, offset, error_std;

    MotorDriver::Value eval_start = params.pwm_center - (params.direction * params.pwm_safe);
    MotorDriver::Value eval_end = params.pwm_center + (params.direction * params.pwm_safe);

    MotorDriver::Value eval_step = params.direction * MotorDriver::MS_TO_PWM(0.05);
    {
        LOG_SCOPE("get_physical_bound", "Step 0: Profiling inside Safe Zone");

        // Send motor to the start of the evaluation range (safe zone)
        RETURN_ERROR(MotorDriver::SetPWM(motor_channel, eval_start));
        RETURN_ERROR(MotorDriver::SendData());
        vTaskDelay(pdMS_TO_TICKS(params.base_delay_ms));

        MotorDriver::Value current_eval_pwm = eval_start;

        // Looping in the right direction (towards the bound)
        while ((params.direction == 1 && current_eval_pwm <= eval_end) || 
               (params.direction == -1 && current_eval_pwm >= eval_end))
        {
            RETURN_ERROR(MotorDriver::SetPWM(motor_channel, current_eval_pwm));
            RETURN_ERROR(MotorDriver::SendData());
            vTaskDelay(pdMS_TO_TICKS(100)); // Wait a bit to get a good feedback reading

            AnalogDriver::Value feedback;
            RETURN_ERROR(AnalogDriver::internal::read_subsampled(feedback, params.feedback_subsamples_fast));
            
            regression.addPoint(current_eval_pwm, feedback);
            current_eval_pwm += eval_step;
        }

        if (!regression.compute(slope, offset, error_std)) {
            LOG_ERROR("get_physical_bound", "Failed to compute motor regression profile");
            return Error::Unknown;
        }

        // Stall threshold is noise x2 too high OR regression error x3 too high
        stall_threshold = std::max(static_cast<float>(params.feedback_noise) * 2.0f, error_std * 3.0f);
        LOG_DEBUG("get_physical_bound", "Profile computed: Slope=%.4f, Threshold=%.3f V", slope, stall_threshold);
    }
    MotorDriver::Value current_pwm = eval_end;

    // =========================================================================
    // STEP 1 : Fast Approach
    // =========================================================================
    int stall_counter = 0;
    MotorDriver::Value fast_bound_pwm = current_pwm;
    bool bound_found = false;
    {
        LOG_SCOPE("get_physical_bound", "Step 1: Fast Approach");

        while ((params.direction == 1 && current_pwm <= params.pwm_max) || 
            (params.direction == -1 && current_pwm >= params.pwm_min))
        {
            current_pwm += params.direction * params.pwm_faststep;
            RETURN_ERROR(MotorDriver::SetPWM(motor_channel, current_pwm));
            RETURN_ERROR(MotorDriver::SendData());
            vTaskDelay(pdMS_TO_TICKS(params.feedback_latency_ms * 2)); // little margin to be sure

            AnalogDriver::Value feedback;
            RETURN_ERROR(AnalogDriver::internal::read_subsampled(feedback, params.feedback_subsamples_fast));

            // Prédiction vs Réalité
            float expected_feedback = slope * current_pwm + offset;
            float error = std::abs(feedback - expected_feedback);

            if (error > stall_threshold) {
                stall_counter++;
                if (stall_counter >= 3) { // 3 iterations debounce
                    fast_bound_pwm = current_pwm;
                    bound_found = true;
                    break;
                }
            } else {
                stall_counter = 0; // Reset if false stall detected
            }
        }

        if (!bound_found) {
            LOG_ERROR("get_physical_bound", "Reached PWM limit without finding bound. Mechanical issue?");
            return Error::HardwareFailure;
        }
    }

    // =========================================================================
    // Step 2 : Back-off
    // =========================================================================
    {
        LOG_SCOPE("get_physical_bound", "Step 2: Back-off");
        current_pwm = fast_bound_pwm - (params.direction * params.pwm_backoff);
        RETURN_ERROR(MotorDriver::SetPWM(motor_channel, current_pwm));
        RETURN_ERROR(MotorDriver::SendData());
        vTaskDelay(pdMS_TO_TICKS(params.backoff_delay_ms));
    }

    // =========================================================================
    // Step 3 : Precision Touch
    // =========================================================================
    {
        LOG_SCOPE("get_physical_bound", "Step 3: Precision Touch");
        stall_counter = 0;

        while ((params.direction == 1 && current_pwm <= params.pwm_max) || 
            (params.direction == -1 && current_pwm >= params.pwm_min))
        {
            current_pwm += params.direction * params.pwm_slowstep;
            RETURN_ERROR(MotorDriver::SetPWM(motor_channel, current_pwm));
            RETURN_ERROR(MotorDriver::SendData());
            vTaskDelay(pdMS_TO_TICKS(params.feedback_latency_ms * 2)); // little margin to be sure

            AnalogDriver::Value feedback;
            RETURN_ERROR(AnalogDriver::internal::read_subsampled(feedback, params.feedback_subsamples_slow));

            float expected_feedback = slope * current_pwm + offset;
            float error = std::abs(feedback - expected_feedback);

            // Little bit more sensitive because we go slower
            if (error > stall_threshold * 0.8f) {
                stall_counter++;
                if (stall_counter >= 2) { // Less debounce needed
                    out_value.pwm_value = current_pwm;
                    out_value.feedback_value = feedback;
                    LOG_DEBUG("get_physical_bound", "Bound precision hit at PWM %d, Feedback %.3f V", current_pwm, feedback);
                    return Error::None;
                }
            } else {
                stall_counter = 0;
            }
        }
    }

    return Error::HardwareFailure;
}