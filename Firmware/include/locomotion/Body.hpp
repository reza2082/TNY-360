#pragma once
#include "locomotion/Leg.hpp"
#include "locomotion/Joint.hpp"
#include "common/geometry.hpp"
#include "locomotion/IMU.hpp"
#include "locomotion/ControlLoop.hpp"

class Body
{
public:
    constexpr static const char* TAG = "Body";

    Body();

    /**
     * @brief Initialize the body.
     * @return Error code indicating success or failure.
     */
    Error init();

    /**
     * @brief Deinitialize the body.
     * @return Error code indicating success or failure.
     */
    Error deinit();
    
    /**
     * @brief Estimate the body state.
     * @note This method should not be called manually, it is called internally in the control loop.
     * @return Error code indicating success or failure.
     */
    Error estimateState(float dt);

    /**
     * @brief Apply a new command to the entire body.
     * @note This method should not be called manually, it is called internally in the control loop.
     * @return Error code indicating success or failure.
     */
    Error applyCommand(BodyJointState jointState, float dt);

    /**
     * @brief Enable all legs and ears of the body.
     * @return Error code indicating success or failure.
     */
    Error enable();

    /**
     * @brief Disable all legs and ears of the body.
     * @return Error code indicating success or failure.
     */
    Error disable();

    /**
     * @brief Get a given leg from its index
     * @param index The leg index, see Leg::Index.
     * @return Reference to the Leg.
     */
    inline Leg& getLeg(Leg::Index index) { return legs[static_cast<size_t>(index)]; }

    /**
     * @brief Get the front left leg.
     * @return Reference to the front left Leg.
     */
    inline Leg& getFrontLeftLeg() { return legs[static_cast<size_t>(Leg::Index::FRONT_LEFT)]; }

    /**
     * @brief Get the front right leg.
     * @return Reference to the front right Leg.
     */
    inline Leg& getFrontRightLeg() { return legs[static_cast<size_t>(Leg::Index::FRONT_RIGHT)]; }

    /**
     * @brief Get the back left leg.
     * @return Reference to the back left Leg.
     */
    inline Leg& getBackLeftLeg() { return legs[static_cast<size_t>(Leg::Index::BACK_LEFT)]; }

    /**
     * @brief Get the back right leg.
     * @return Reference to the back right Leg.
     */
    inline Leg& getBackRightLeg() { return legs[static_cast<size_t>(Leg::Index::BACK_RIGHT)]; }

    /**
     * @brief Get the left ear joint.
     * @return Reference to the left ear Joint.
     */
    inline Joint& getLeftEar() { return ear_l; }

    /**
     * @brief Get the right ear joint.
     * @return Reference to the right ear Joint.
     */
    inline Joint& getRightEar() { return ear_r; }

    /**
     * @brief Get the IMU controller.
     * @return Reference to the IMU controller.
     */
    inline IMU& getIMU() { return imu; }

private:
    Leg legs[4]; // Array of 4 legs
    Joint ear_l; // Left Ear Joint
    Joint ear_r; // Right Ear Joint

    IMU imu;

    // Kinematic parameters and state
    Vec3f local_hip_positions_mm[4]; // local to base_link (body center)
    Vec3f global_feet_positions_mm[4]; // global positions in world frame (ground reference)
    Transformf posture; // Body posture (position relative to world, orientation relative to body frame)

    Error apply_posture();
};