#pragma once
#include "network/protocol/Protocol.hpp"
#include "common/BinaryReader.hpp"
#include "common/SysStats.hpp"
#include "common/Log.hpp"
#include "Robot.hpp"
#include <esp_system.h>

namespace Protocol
{
namespace System
{
    constexpr uint8_t MODULE_ID = 0x00;

    static void Ping(const RequestContext& ctx, const uint8_t* payload)
    {
        ctx.respond(ResponseStatus::Ok);
    }

    static void Reboot(const RequestContext& ctx, const uint8_t* payload)
    {
        ctx.respond(ResponseStatus::Ok);
        esp_restart();
    }

    static void SetSettings(const RequestContext& ctx, const uint8_t* payload)
    {
        ctx.respond(ResponseStatus::Ok);
        // TODO : Implement
    }

    static void GetSettings(const RequestContext& ctx, const uint8_t* payload)
    {
        ctx.respond(ResponseStatus::Ok);
        // TODO : Implement
    }

    static void SetAutolifeLevel(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t level;
        if (reader.read(level) != Error::None)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        Error err = Robot::GetInstance().getDecisionLoop().setAutoLifeLevel(level);

        ctx.respond(err == Error::None ? ResponseStatus::Ok : ResponseStatus::InvalidParameters);
    }

    static void GetAutolifeLevel(const RequestContext& ctx, const uint8_t* payload)
    {
        uint8_t autoLifeLevel = Robot::GetInstance().getDecisionLoop().getAutoLifeLevel();
        ctx.respond(ResponseStatus::Ok, (uint8_t*) &autoLifeLevel, sizeof(autoLifeLevel));
    }

    static void GetStatistics(const RequestContext& ctx, const uint8_t* payload)
    {
        LOG_DEBUG("getstats", "Received request");

        float temp_c = SysStats::GetTemperature();
        SysStats::CPUUsage cpu_usage = SysStats::GetCPUUsage();
        SysStats::RAMUsage ram_usage = SysStats::GetRAMUsage();

        uint8_t result[sizeof(temp_c) + sizeof(cpu_usage) + sizeof(ram_usage)];
        size_t offset = 0;
        memcpy(result + offset, &temp_c, sizeof(temp_c));       offset += sizeof(temp_c);
        memcpy(result + offset, &cpu_usage, sizeof(cpu_usage)); offset += sizeof(cpu_usage);
        memcpy(result + offset, &ram_usage, sizeof(ram_usage)); offset += sizeof(ram_usage);

        ctx.respond(ResponseStatus::Ok, result, sizeof(result));
    }


    static ActionCallback actions[] = {
        Ping,                      // 0x00
        Reboot,                    // 0x01
        SetSettings,               // 0x02
        GetSettings,               // 0x03
        SetAutolifeLevel,          // 0x04
        GetAutolifeLevel,          // 0x05
        GetStatistics,             // 0x06
    };

    static void Register(Dispatcher& dispatcher)
    {
        dispatcher.registerModule(MODULE_ID, actions, sizeof(actions));
    }
}
}