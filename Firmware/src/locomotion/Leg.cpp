#include "locomotion/Leg.hpp"
#include "common/Log.hpp"
#include "common/config.hpp"

Leg::Leg() {}

Leg::Leg(Joint hip_roll, Joint hip_pitch, Joint knee_pitch, AnalogDriver::Channel contact_channel, bool y_inverted)
    : contact_channel(contact_channel), y_inverted(y_inverted)
{
    joints[(int)JointId::HipRoll] = hip_roll;
    joints[(int)JointId::HipPitch] = hip_pitch;
    joints[(int)JointId::KneePitch] = knee_pitch;
    grounded = true; // by default on the ground
}

Error Leg::init()
{
    Error err;

    for (int i = 0; i < (int)JointId::Count; i++)
    {
        if ((err = joints[i].init()) != Error::None)
        {
            return err;
        }
    }
    return Error::None;
}

Error Leg::deinit()
{
    Error err;

    for (int i = 0; i < (int)JointId::Count; i++)
    {
        if ((err = joints[i].deinit()) != Error::None)
        {
            return err;
        }
    }
    return Error::None;
}

Error Leg::estimateState(float dt)
{
    Error err;

    // Check if grounded
    AnalogDriver::Value voltage;
    if (Error err = AnalogDriver::GetVoltage(this->contact_channel, voltage); err != Error::None)
    {
        return err;
    }
    grounded = voltage < LEG_GROUNDED_THRESHOLD_V;

    // pass the call to all motors
    for (int i = 0; i < (int)JointId::Count; i++)
    {
        if ((err = joints[i].estimateState(dt)) != Error::None)
        {
            return err;
        }
    }

    return Error::None;
}

Error Leg::applyCommand(LegJointState jointState, float dt)
{
    for (int i = 0; i < (int)JointId::Count; i++)
    {
        if (Error err = joints[i].applyCommand(jointState.joint_angles_rad[i], dt); err != Error::None)
        {
            return err;
        }
    }

    return Error::None;
}

Error Leg::enable()
{
    for (int i = 0; i < (int)JointId::Count; i++)
    {
        if (Error err = joints[i].enable(); err != Error::None)
        {
            return err;
        }
    }
    return Error::None;
}

Error Leg::disable()
{
    for (int i = 0; i < (int)JointId::Count; i++)
    {
        if (Error err = joints[i].disable(); err != Error::None)
        {
            return err;
        }
    }
    return Error::None;
}
