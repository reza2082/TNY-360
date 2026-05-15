#include <freertos/FreeRTOS.h>
#include <esp_log.h>
#include <cmath>
#include "LED.hpp"
#include "pwm.hpp"
#include "adc.hpp"

#define TAG "main"

#define SERVO_PWM_PERIOD 5000 // 20ms period
#define SERVO_PERIOD_MIN 500 // 0.5ms pulse
#define SERVO_PERIOD_MAX 2500 // 2.5ms pulse

#define SERVO_DUTYCYCLE_MIN (float)SERVO_PERIOD_MIN / (float)SERVO_PWM_PERIOD
#define SERVO_DUTYCYCLE_MAX (float)SERVO_PERIOD_MAX / (float)SERVO_PWM_PERIOD
#define SERVO_DUTYCYCLE_MID (SERVO_DUTYCYCLE_MIN + SERVO_DUTYCYCLE_MAX) / 2.0f

#ifdef __cplusplus
extern "C" {
#endif
void app_main()
{
    LED::Init();
    PWM::Init();
    ADC::Init();

    ///////////// Main testing loop ////////////
    ESP_LOGI(TAG, "\n\n<==================== Starting Servo Tester ===================>\n");

    float voltages[3];

    // send servo 90 degree position
    ESP_LOGI(TAG, "Setting servo to 0 degree");
    PWM::SetPWM(SERVO_DUTYCYCLE_MIN);
    LED::SetColor(0, {0, 0, 10}, 0.05); // Blue
    vTaskDelay(pdMS_TO_TICKS(1000)); // wait

    // read voltage from ADC and log it
    voltages[0] = ADC::read();
    ESP_LOGI(TAG, "===> ADC Voltage: %.2f V", voltages[0]);
    LED::SetColor(0, {0, 0, 0}, 0.05); // Turn Off
    vTaskDelay(pdMS_TO_TICKS(200)); // wait

    // send servo 0 degree position
    ESP_LOGI(TAG, "Setting servo to 180 degree");
    PWM::SetPWM(SERVO_DUTYCYCLE_MAX);
    LED::SetColor(0, {0, 0, 10}, 0.05); // Blue
    vTaskDelay(pdMS_TO_TICKS(1500)); // wait
    
    // read voltage from ADC and log it
    voltages[1] = ADC::read();
    ESP_LOGI(TAG, "===> ADC Voltage: %.2f V", voltages[1]);
    LED::SetColor(0, {0, 0, 0}, 0.05); // Turn Off
    vTaskDelay(pdMS_TO_TICKS(200)); // wait

    // send servo 180 degree position
    ESP_LOGI(TAG, "Setting servo to 90 degree");
    PWM::SetPWM(SERVO_DUTYCYCLE_MID);
    LED::SetColor(0, {0, 0, 10}, 0.05); // Blue
    vTaskDelay(pdMS_TO_TICKS(1000)); // wait
    
    // read voltage from ADC and log it
    voltages[2] = ADC::read();
    ESP_LOGI(TAG, "===> ADC Voltage: %.2f V", voltages[2]);
    LED::SetColor(0, {0, 0, 0}, 0.05); // Turn Off
    vTaskDelay(pdMS_TO_TICKS(200)); // wait
    

    PWM::SetPWM(0.0f); // turn off motor


    // Check voltages to confirm feedback is working
    float min2mid = voltages[0] - voltages[2];
    float mid2max = voltages[2] - voltages[1];
    float min2max = voltages[0] - voltages[1];
    if (min2mid * mid2max < 0.0f) {
        ESP_LOGE(TAG, "Center voltage is not between min and max, feedback might be incorrect");
        LED::SetColor(0, {10, 0, 0}, 0.05); // Red
        return;
    }
    if (std::abs(min2mid) < 0.5f || std::abs(mid2max) < 0.5f || std::abs(min2max) < 0.5f) {
        ESP_LOGW(TAG, "Voltage difference too small, feedback might be weak");
        LED::SetColor(0, {10, 0, 0}, 0.05); // Red
        return;
    }

    if (min2max > 0.0f) {
        ESP_LOGI(TAG, "Feedback looks good, voltage is DECREASING with servo position");
    } else {
        ESP_LOGI(TAG, "Feedback looks good, voltage is INCREASING with servo position");
    }
    LED::SetColor(0, {0, 10, 0}, 0.05); // Green
}
#ifdef __cplusplus
}
#endif