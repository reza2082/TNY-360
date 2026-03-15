#pragma once
#include "common/utils.hpp"
#include "common/geometry.hpp"
#include "locomotion/GaitPlanner.hpp"

namespace IPC
{
    constexpr const char* TAG = "IPC";
    constexpr int NB_LEGS = 4;
    constexpr int NB_JOINTS = 14;
    
    enum class OverrideMode {
        None,      // No override
        Absolute,  // Replace the IK result with the given angle
        Relative,  // Add the given angle to the IK result
    };
    
    struct JointOverride {
        OverrideMode mode = OverrideMode::None;
        float value_rad = 0.0f;
    };

    struct LegOverride {
        OverrideMode mode = OverrideMode::None;
        Vec3f value_pos;
    };

    struct BodyOverride {
        OverrideMode mode = OverrideMode::None;
        Vec3f value_pos;
        Vec3f value_rot;
    };
    
    struct ControlIntent {
        uint32_t timestamp_ms; // timestamp for safeguard watchdog

        // movement planning infos
        GaitPlanner::GaitType gait;
        Vec3f body_vel;
        Vec3f body_rot;
        Vec3f body_pos;

        // cartesian override for animation
        LegOverride leg_overrides[NB_LEGS];
        BodyOverride body_override;
    
        // joint control override
        JointOverride joint_overrides[NB_JOINTS];
    };

    Error Init();

    Error Deinit();

    Error setIntent(ControlIntent& intent);

    bool getIntent(ControlIntent* intent);
};
