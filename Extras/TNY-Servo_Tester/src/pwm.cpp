#include "pwm.hpp"

#include <algorithm>
#include "driver/ledc.h"
#include "esp_err.h"

namespace PWM
{
    namespace
    {
        constexpr gpio_num_t PWM_PIN = GPIO_NUM_42;
        constexpr ledc_mode_t LEDC_MODE = LEDC_LOW_SPEED_MODE;
        constexpr ledc_timer_t LEDC_TIMER = LEDC_TIMER_0;
        constexpr ledc_channel_t LEDC_CHANNEL = LEDC_CHANNEL_0;
        constexpr ledc_timer_bit_t LEDC_RESOLUTION = LEDC_TIMER_14_BIT; // 0..16383
        constexpr uint32_t PWM_FREQ_HZ = 200;

        constexpr uint32_t DUTY_MAX = (1u << LEDC_RESOLUTION) - 1u;
    }

    void Init()
    {
        ledc_timer_config_t timer_cfg = {};
        timer_cfg.speed_mode = LEDC_MODE;
        timer_cfg.timer_num = LEDC_TIMER;
        timer_cfg.duty_resolution = LEDC_RESOLUTION;
        timer_cfg.freq_hz = PWM_FREQ_HZ;
        timer_cfg.clk_cfg = LEDC_AUTO_CLK;
        ledc_timer_config(&timer_cfg);

        ledc_channel_config_t ch_cfg = {};
        ch_cfg.gpio_num = PWM_PIN;
        ch_cfg.speed_mode = LEDC_MODE;
        ch_cfg.channel = LEDC_CHANNEL;
        ch_cfg.intr_type = LEDC_INTR_DISABLE;
        ch_cfg.timer_sel = LEDC_TIMER;
        ch_cfg.duty = 0;
        ch_cfg.hpoint = 0;
        ledc_channel_config(&ch_cfg);
    }

    void SetPWM(float value)
    {
        value = std::clamp(value, 0.0f, 1.0f);
        const uint32_t duty = static_cast<uint32_t>(value * static_cast<float>(DUTY_MAX));

        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
    }
}