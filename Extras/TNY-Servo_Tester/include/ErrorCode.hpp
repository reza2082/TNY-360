#pragma once
#include <cstdint>

namespace ErrorCode
{
    constexpr uint8_t I2cBusPrimaryInitFailed = 0x01;
    constexpr uint8_t I2cBusSecondaryInitFailed = 0x02;
    constexpr uint8_t ScreenInitFailed = 0x03;
    constexpr uint8_t MenusInitFailed = 0x04;
    constexpr uint8_t SpeakerInitFailed = 0x05;
    constexpr uint8_t MixerInitFailed = 0x06;
    constexpr uint8_t WiFiInitFailed = 0x07;
    constexpr uint8_t WebInterfaceInitFailed = 0x08;
    constexpr uint8_t WebSocketInitFailed = 0x09;
    constexpr uint8_t UpdateInitFailed = 0x0A;
    constexpr uint8_t ProtocolInitFailed = 0x0B;
    constexpr uint8_t DriverInitFailed = 0x0C;
    constexpr uint8_t ReaderInitFailed = 0x0D;
    constexpr uint8_t IMUInitFailed = 0x0E;
};