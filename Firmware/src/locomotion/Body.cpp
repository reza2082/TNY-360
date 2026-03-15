#include "locomotion/Body.hpp"
#include "common/config.hpp"
#include "common/Log.hpp"
#include "common/RPC.hpp"
#include "locomotion/IPC.hpp"

Body::Body()
{
    // Create the 4 legs
    legs[static_cast<size_t>(Leg::Index::FRONT_LEFT)] = Leg(
        Joint(1, MotorController( 7,  9), DEG_TO_RAD( -45.0f), DEG_TO_RAD( 45.0f), false),  // Hip Roll
        Joint(2, MotorController( 5, 11), DEG_TO_RAD(-135.0f), DEG_TO_RAD( 45.0f), true ),  // Hip Pitch
        Joint(3, MotorController( 6, 10), DEG_TO_RAD(   0.0f), DEG_TO_RAD(150.0f), false),  // Knee Pitch
        3, true
    );
    legs[static_cast<size_t>(Leg::Index::FRONT_RIGHT)] = Leg(
        Joint(4, MotorController( 4, 13), DEG_TO_RAD( -45.0f), DEG_TO_RAD( 45.0f), true ),  // Hip Roll
        Joint(5, MotorController( 2, 15), DEG_TO_RAD(-135.0f), DEG_TO_RAD( 45.0f), false),  // Hip Pitch
        Joint(6, MotorController( 3, 14), DEG_TO_RAD(   0.0f), DEG_TO_RAD(150.0f), true ),  // Knee Pitch
        7, false
    );
    legs[static_cast<size_t>(Leg::Index::BACK_LEFT)] = Leg(
        Joint(7, MotorController(10,  5), DEG_TO_RAD( -45.0f), DEG_TO_RAD( 45.0f), true ),  // Hip Roll
        Joint(8, MotorController( 8,  7), DEG_TO_RAD(-135.0f), DEG_TO_RAD( 45.0f), true ),  // Hip Pitch
        Joint(9, MotorController( 9,  6), DEG_TO_RAD(   0.0f), DEG_TO_RAD(150.0f), false),  // Knee Pitch
        11, true
    );
    legs[static_cast<size_t>(Leg::Index::BACK_RIGHT)] = Leg(
        Joint(10, MotorController(13,  1), DEG_TO_RAD( -45.0f), DEG_TO_RAD( 45.0f), false),  // Hip Roll
        Joint(11, MotorController(11,  3), DEG_TO_RAD(-135.0f), DEG_TO_RAD( 45.0f), false),  // Hip Pitch
        Joint(12, MotorController(12,  2), DEG_TO_RAD(   0.0f), DEG_TO_RAD(150.0f), true ),  // Knee Pitch
        15, false
    );

    // Create the ears (default calib is for MG996R, using SG90 for ears)
    ear_l = Joint(13, MotorController(1, 0, MotorController::DEFAULT_CALIBRATION_SG90), DEG_TO_RAD(0.0f), DEG_TO_RAD(180.0f), true, false);
    ear_r = Joint(14, MotorController(0, 0, MotorController::DEFAULT_CALIBRATION_SG90), DEG_TO_RAD(0.0f), DEG_TO_RAD(180.0f), false, false);
}

Error Body::init()
{
    Error err;

    // Initialize the legs
    for (size_t i = 0; i < static_cast<size_t>(Leg::Index::COUNT); i++)
    {
        if ((err = legs[i].init()) != Error::None)
        {
            return err;
        }
    }
    
    // Initialize the ears
    if ((err = ear_l.init()) != Error::None)
    {
        return err;
    }
    if ((err = ear_r.init()) != Error::None)
    {
        return err;
    }

    // Initialize the IMU
    if ((err = imu.init()) != Error::None)
    {
        return err;
    }

    // Initialize the IPC layer (for inter-core infos)
    if (Error err = IPC::Init(); err != Error::None)
    {
        return err;
    }

    // Initialize the RPC layer (for inter-core calls)
    if (Error err = RPC::Init(); err != Error::None)
    {
        return err;
    }

    return Error::None;
}

Error Body::deinit()
{
    // Deinitialize the RPC layer
    if (Error err = RPC::DeInit(); err != Error::None)
    {
        return err;
    }

    // Deinitialize the IPC layer (for inter-core infos)
    if (Error err = IPC::Deinit(); err != Error::None)
    {
        return err;
    }

    // Deinitialize the legs
    for (size_t i = 0; i < static_cast<size_t>(Leg::Index::COUNT); i++)
    {
        if (Error err = legs[i].deinit(); err != Error::None)
        {
            return err;
        }
    }
    
    // Deinitialize the ears
    if (Error err = ear_l.deinit(); err != Error::None)
    {
        return err;
    }
    if (Error err = ear_r.deinit(); err != Error::None)
    {
        return err;
    }

    // Deinitialize the IMU
    if (Error err = imu.deinit(); err != Error::None)
    {
        return err;
    }

    return Error::None;
}

Error Body::estimateState(float dt)
{
    // update legs
    for (size_t i = 0; i < static_cast<size_t>(Leg::Index::COUNT); i++)
    {
        if (Error err = legs[i].estimateState(dt); err != Error::None)
        {
            return err;
        }
    }
    return Error::None;
}

Error Body::applyCommand(BodyJointState jointState, float dt)
{
    Error err;

    // update legs
    for (size_t i = 0; i < static_cast<size_t>(Leg::Index::COUNT); i++)
    {
        if ((err = legs[i].applyCommand(jointState.leg_joints[i], dt)) != Error::None)
        {
            return err;
        }
    }
    
    // update ears
    if ((err = ear_l.applyCommand(jointState.ear_l_rad, dt)) != Error::None)
    {
        return err;
    }
    if ((err = ear_r.applyCommand(jointState.ear_r_rad, dt)) != Error::None)
    {
        return err;
    }
    return Error::None;
}

Error Body::enable()
{
    Error err;

    // enable legs
    for (size_t i = 0; i < static_cast<size_t>(Leg::Index::COUNT); i++)
    {
        if ((err = legs[i].enable()) != Error::None)
        {
            return err;
        }
    }
    
    // enable ears
    if ((err = ear_l.enable()) != Error::None)
    {
        return err;
    }
    if ((err = ear_r.enable()) != Error::None)
    {
        return err;
    }
    return Error::None;
}

Error Body::disable()
{
    Error err;

    // disable legs
    for (size_t i = 0; i < static_cast<size_t>(Leg::Index::COUNT); i++)
    {
        if ((err = legs[i].disable()) != Error::None)
        {
            return err;
        }
    }
    
    // disable ears
    if ((err = ear_l.disable()) != Error::None)
    {
        return err;
    }
    if ((err = ear_r.disable()) != Error::None)
    {
        return err;
    }
    return Error::None;
}
