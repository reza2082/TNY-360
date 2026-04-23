#include "adc.hpp"
#include <esp_adc/adc_oneshot.h>
#include <esp_log.h>

namespace ADC
{
    namespace
    {
        static adc_oneshot_unit_handle_t adc_handle = nullptr;
        static adc_cali_handle_t cali_handle = nullptr;
    }

    void Init()
    {
        // Create adc oneshot handle
        adc_oneshot_unit_init_cfg_t init_config = {
            .unit_id = ADC_UNIT_1,
            .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
            .ulp_mode = ADC_ULP_MODE_DISABLE,
        };
        if (adc_oneshot_new_unit(&init_config, &adc_handle) != ESP_OK)
        {
            ESP_LOGI("adc", "Failed to create ADC oneshot handle");
            return;
        }

        // Configure adc channel
        adc_oneshot_chan_cfg_t config = {
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        if (adc_oneshot_config_channel(adc_handle, ADC_CHANNEL_1, &config) != ESP_OK)
        {
            ESP_LOGI("adc", "Failed to configure ADC oneshot channel");
            return;
        }

        // configure calibration handle
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = ADC_UNIT_1,
            .chan = ADC_CHANNEL_1,
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        adc_cali_create_scheme_curve_fitting(&cali_config, &cali_handle);
    }

    float read()
    {
        int voltages[9] = {};
        for (int i = 0; i < 9; ++i)
        {
            int raw = 0;
            if (adc_oneshot_read(adc_handle, ADC_CHANNEL_1, &raw) != ESP_OK)
            {
            ESP_LOGI("adc", "Failed to read ADC");
            return 0.0f;
            }

            if (adc_cali_raw_to_voltage(cali_handle, raw, &voltages[i]) != ESP_OK)
            {
            ESP_LOGI("adc", "Failed to convert ADC raw to voltage");
            return 0.0f;
            }
        }

        // Sort 9 values and return the median
        for (int i = 0; i < 8; ++i)
        {
            for (int j = i + 1; j < 9; ++j)
            {
            if (voltages[j] < voltages[i])
            {
                int tmp = voltages[i];
                voltages[i] = voltages[j];
                voltages[j] = tmp;
            }
            }
        }

        return static_cast<float>(voltages[4]) / 1000.0f; // median mV to V
    }
}