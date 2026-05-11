#pragma once
#include "common/utils.hpp"

namespace PowerDriver
{
    constexpr const char* TAG = "PowerDriver";

    using Value = float;

    struct PowerData
    {
        Value voltage_v; // in volts
        Value current_a; // in amperes
        Value power_w;   // in watts
    };

    namespace internal
    {
        Error read_voltage(Value& voltage_v);
        Error read_current(Value& current_a);
        Error read_power(Value& power_w);
    }

    /**
    * @brief Initializes the Power driver.
    * @return Error code indicating success or failure.
    */
    Error Init();

    /**
    * @brief Deinitializes the Power driver.
    * @return Error code indicating success or failure.
    */
    Error Deinit();

    /**
     * @brief Internal function to read data from the INA219.
     * @note YOU SHOULD NOT CALL THIS FUNCTION DIRECTLY.
     * @return void.
     */
    Error ReadData();

    /**
     * @brief Get the last read values from the INA219.
     * @return most recent PowerData object.
     */
    PowerData& GetData();
}
