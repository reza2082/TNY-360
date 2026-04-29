#include <freertos/FreeRTOS.h>
#include "common/Log.hpp"
#include "Robot.hpp"
#include "common/config.hpp"
#include "audio/WavProvider.hpp"
#include "audio/SineProvider.hpp"
#include "drivers/CameraDriver.hpp"
#include "boot/BootManager.hpp"

static const char* TAG = "Main";

static Robot robot;
CameraDriver cam;
WiFiManager wifi;

#ifdef __cplusplus
extern "C" {
#endif
void app_main()
{
    // wifi.init();
    
    // vTaskDelay(pdMS_TO_TICKS(2000));

    // cam.init();

    // return;

    // Check for special boot state (zero-calibration, pending update, etc.)
    if (BootManager::CheckForSpecialBoot())
    {
        // Special boot state detected.
        // The corresponding boot will be handled by the BootManager.
        // just return to avoid booting as normal (should not happen anyway)
        return;
    }

    LOG_INFO(TAG, "Initializing robot (FIRMWARE_VERSION=%s) ...", FIRMWARE_VERSION);

    if (Error err = robot.init(); err != Error::None)
    {
        LOG_ERROR(TAG, "Failed to initialize robot. Error : %s", ErrorToString(err));
        return;
    }

    LOG_INFO(TAG, "Robot initialized successfully.");
    
    LOG_INFO(TAG, "Starting robot ...");

    if (Error err = robot.start(); err != Error::None)
    {
        LOG_ERROR(TAG, "Failed to start robot. Error : %s", ErrorToString(err));
        return;
    }

    LOG_INFO(TAG, "Robot started successfully !");

    LOG_INFO(TAG, "Enabling body motors.");
    if (Error err = robot.getBody().enable(); err != Error::None)
    {
        LOG_ERROR(TAG, "Failed to enable body motors. Error: %s", ErrorToString(err));
        return;
    }

    // clamping joint velocity to stand up (safety measure)
    Joint::ClampVelocity(0.5f); // rad/s

    Transformf stand_position{
        Vec3f{0.f, 0.f, DEFAULT_BODY_HEIGHT_M},
        Quatf{}
    };
    Robot::GetInstance().getDecisionLoop().setBodyTransform(stand_position);
    robot.getBody().getLeftEar().applyCommand(DEG_TO_RAD(45.f), 0); // TODO : Use joint override
    robot.getBody().getRightEar().applyCommand(DEG_TO_RAD(45.f), 0); // TODO : Use joint override

    // wait a bit for the robot to stand up
    vTaskDelay(pdMS_TO_TICKS(4000));

    // Reset joint velocity clamping and we're ready to go!
    Joint::ClampVelocity(Joint::MAX_VELOCITY_RAD_S); // could also set to 0 to disable clamping, either way works
    
    LOG_INFO(TAG, ">>> Robot is operational.");
}
#ifdef __cplusplus
}
#endif