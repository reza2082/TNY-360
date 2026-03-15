#pragma once
#include "locomotion/Joint.hpp"
#include "locomotion/LegKinematics.hpp"
#include "locomotion/utils.hpp"
#include "common/geometry.hpp"

class Leg
{
public:
    constexpr static const char* TAG = "Leg";

    /// @brief Leg index enum
    enum class Index {
        /// @brief  Front Left Leg
        FRONT_LEFT = 0,
        /// @brief  Front Right Leg
        FRONT_RIGHT = 1,
        /// @brief  Back Left Leg
        BACK_LEFT = 2,
        /// @brief  Back Right Leg
        BACK_RIGHT = 3,
        /// @brief  Total number of legs in the body
        COUNT = 4
    };

    Leg();

    Leg(Joint hip_roll, Joint hip_pitch, Joint knee_pitch, AnalogDriver::Channel contact_channel, bool y_inverted = false);

    /**
     * @brief Initialize the leg.
     * @return Error code indicating success or failure.
     */
    Error init();

    /**
     * @brief Deinitialize the leg.
     * @return Error code indicating success or failure.
     */
    Error deinit();

    /**
     * @brief Estimate the leg state.
     * @note This method should not be called manually, it is called internally in the control loop.
     * @return Error code indicating success or failure.
     */
    Error estimateState(float dt);

    /**
     * @brief Apply a new command.
     * @note This method should not be called manually, it is called internally in the control loop.
     * @return Error code indicating success or failure.
     */
    Error applyCommand(LegJointState jointState, float dt);

    /**
     * @brief Enable all joints of the leg.
     * @return Error code indicating success or failure.
     */
    Error enable();

    /**
     * @brief Disable all joints of the leg.
     * @return Error code indicating success or failure.
     */
    Error disable();

    /**
     * @brief Get the hip roll joint.
     * @return Reference to the hip roll Joint.
     */
    inline Joint& getHipRoll() { return hip_roll; }

    /**
     * @brief Get the hip pitch joint.
     * @return Reference to the hip pitch Joint.
     */
    inline Joint& getHipPitch() { return hip_pitch; }

    /**
     * @brief Get the knee pitch joint.
     * @return Reference to the knee pitch Joint.
     */
    inline Joint& getKneePitch() { return knee_pitch; }

    /**
     * @brief Get if the leg is inverted along the y axis
     * @return Boolean true if inverted
     */
    inline bool isInverted() { return y_inverted; }

    /**
     * @brief Get if the leg touches ground or not
     * @return Boolean true if grounded
     */
    inline bool isGrounded() { return grounded; }

private:
    Joint hip_roll;
    Joint hip_pitch;
    Joint knee_pitch;
    AnalogDriver::Channel contact_channel;
    bool y_inverted;

    bool grounded;
};