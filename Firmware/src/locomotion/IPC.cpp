#include "locomotion/IPC.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "common/Log.hpp"

namespace IPC
{
    QueueHandle_t intent_queue = nullptr;
    QueueHandle_t state_queue = nullptr;

    Error Init()
    {
        LOG_SCOPE(TAG, "IPC::Init");
        
        intent_queue = xQueueCreate(1, sizeof(ControlIntent));
        state_queue = xQueueCreate(1, sizeof(RobotState));

        if (intent_queue == nullptr || state_queue == nullptr)
        {
            LOG_ERROR(TAG, "Failed to create queues for IPC");
            ErrorHandle(ErrorStruct::IPCInitFailed);
            return Error::SoftwareFailure;
        }

        return Error::None;
    }

    Error Deinit()
    {
        return Error::None;
    }

    Error setIntent(ControlIntent& intent)
    {
        if (xQueueOverwrite(intent_queue, &intent) != pdPASS)
        {
            LOG_ERROR(TAG, "Couldn't override intent in the intent queue");
            return Error::SoftwareFailure;
        }
        return Error::None;
    }

    bool getIntent(ControlIntent* intent)
    {
        return (xQueueReceive(intent_queue, intent, 0) == pdTRUE);
    }

    Error setState(RobotState& state)
    {
        if (xQueueOverwrite(state_queue, &state) != pdPASS)
        {
            LOG_ERROR(TAG, "Couldn't override state in the state queue");
            return Error::SoftwareFailure;
        }
        return Error::None;
    }

    bool getState(RobotState* state)
    {
        return (xQueueReceive(state_queue, state, 0) == pdTRUE);
    }
};