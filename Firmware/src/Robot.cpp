#include "Robot.hpp"
#include "common/Log.hpp"
#include "common/LED.hpp"

Robot* Robot::instance = nullptr;

Robot::Robot()
{
    instance = this;
}

Error Robot::init()
{
    LOG_SCOPE(TAG, "Robot::init");

    // Initialize the LED module to diplay status
    if (Error err = LED::Init(); err != Error::None)
    {
        LOG_ERROR(TAG, "Couldn't initialize LED module");
        return err;
    }

    // Set yellow color (init color)
    LED::SetColor(0, LED::Color(16, 16, 0), 0.1f);

    if (Error err = ui_manager.init(); err != Error::None)
    {
        return err;
    }

    if (Error err = audio_manager.init(); err != Error::None)
    {
        return err;
    }

    if (Error err = network_manager.init(); err != Error::None)
    {
        return err;
    }

    if (Error err = body.init(); err != Error::None)
    {
        return err;
    }

    // Initialize the decision loop
    if (Error err = decision_loop.init(); err != Error::None)
    {
        return err;
    }

    // Initialize the control loop
    if (Error err = control_loop.init(); err != Error::None)
    {
        return err;
    }

    // all good, turn orange for startup
    LED::SetColor(0, LED::Color(16, 8, 0), 0.1f);

    return Error::None;
}

Error Robot::start()
{
    // Start the decision loop
    if (Error err = decision_loop.start(); err != Error::None)
    {
        return err;
    }

    // Start the control loop
    if (Error err = control_loop.start(); err != Error::None)
    {
        return err;
    }

    // NOTE : THIS DELAY IS IMPORTANT!
    // allow some time for systems to stabilize before enabling motors and everything
    // (to get motor feedback data and all that stuff)
    vTaskDelay(pdMS_TO_TICKS(500));

    // robot is ready ! Turn green (low intensity to avoid using power for nothing)
    LED::SetColor(0, LED::Color(0, 1, 0), 0.1f);

    // Set the menu to face (face only displays when everything is working)
    Menus::SetCurrentMenu(Menus::GetMenuFace());

    return Error::None;
}

Error Robot::stop()
{
    // Stop the control loop
    if (Error err = control_loop.stop(); err != Error::None)
    {
        return err;
    }

    // Stop the decision loop
    if (Error err = decision_loop.stop(); err != Error::None)
    {
        return err;
    }

    return Error::None;
}

Error Robot::deinit()
{
    // Deinitialize the control loop
    if (Error err = control_loop.init(); err != Error::None)
    {
        return err;
    }

    // Deinitialize the decision loop
    if (Error err = decision_loop.deinit(); err != Error::None)
    {
        return err;
    }

    if (Error err = body.deinit(); err != Error::None)
    {
        return err;
    }

    if (Error err = network_manager.deinit(); err != Error::None)
    {
        return err;
    }

    if (Error err = audio_manager.deinit(); err != Error::None)
    {
        return err;
    }

    if (Error err = ui_manager.deinit(); err != Error::None)
    {
        return err;
    }

    return Error::None;
}