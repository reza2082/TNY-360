#include "locomotion/Leg.hpp"
#include "common/Log.hpp"
#include "common/config.hpp"

Leg::Leg() {}

Leg::Leg(Joint hip_roll, Joint hip_pitch, Joint knee_pitch, AnalogDriver::Channel contact_channel, bool y_inverted)
    : hip_roll(hip_roll), hip_pitch(hip_pitch), knee_pitch(knee_pitch), contact_channel(contact_channel), y_inverted(y_inverted)
{
    grounded = true; // by default on the ground
}

Error Leg::init()
{
    Error err;

    if ((err = hip_roll.init()) != Error::None)
    {
        return err;
    }
    if ((err = hip_pitch.init()) != Error::None)
    {
        return err;
    }
    if ((err = knee_pitch.init()) != Error::None)
    {
        return err;
    }
    return Error::None;
}

Error Leg::deinit()
{
    Error err;

    if ((err = hip_roll.deinit()) != Error::None)
    {
        return err;
    }
    if ((err = hip_pitch.deinit()) != Error::None)
    {
        return err;
    }
    if ((err = knee_pitch.deinit()) != Error::None)
    {
        return err;
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
    grounded = voltage > LEG_GROUNDED_THRESHOLD_MV;

    // pass the call to all motors
    if ((err = hip_roll.estimateState(dt)) != Error::None)
    {
        return err;
    }
    if ((err = hip_pitch.estimateState(dt)) != Error::None)
    {
        return err;
    }
    if ((err = knee_pitch.estimateState(dt)) != Error::None)
    {
        return err;
    }

    return Error::None;
}

Error Leg::applyCommand(LegJointState jointState, float dt)
{
    Error err;

    // pass the call to all motors
    if ((err = hip_roll.applyCommand(jointState.hipRoll_rad, dt)) != Error::None)
    {
        return err;
    }
    if ((err = hip_pitch.applyCommand(jointState.hipPitch_rad, dt)) != Error::None)
    {
        return err;
    }
    if ((err = knee_pitch.applyCommand(jointState.kneePitch_rad, dt)) != Error::None)
    {
        return err;
    }

    return Error::None;
}

Error Leg::enable()
{
    Error err;

    // pass the call to all motors
    if ((err = hip_roll.enable()) != Error::None)
    {
        return err;
    }
    if ((err = hip_pitch.enable()) != Error::None)
    {
        return err;
    }
    if ((err = knee_pitch.enable()) != Error::None)
    {
        return err;
    }
    return Error::None;
}

Error Leg::disable()
{
    Error err;

    // pass the call to all motors
    if ((err = hip_roll.disable()) != Error::None)
    {
        return err;
    }
    if ((err = hip_pitch.disable()) != Error::None)
    {
        return err;
    }
    if ((err = knee_pitch.disable()) != Error::None)
    {
        return err;
    }
    return Error::None;
}
