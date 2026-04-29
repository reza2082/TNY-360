#include "audio/AudioManager.hpp"

AudioManager::AudioManager() : mixer(speaker)
{
}

Error AudioManager::init()
{
    if (Error err = speaker.init(); err != Error::None)
    {
        ErrorHandle({ErrorCode::SpeakerInitFailed, "Failed to initialize Speaker"});
        return err;
    }

    if (Error err = mixer.init(); err != Error::None)
    {
        ErrorHandle({ErrorCode::MixerInitFailed, "Failed to initialize Mixer"});
        return err;
    }

    return Error::None;
}

Error AudioManager::deinit()
{
    if (Error err = mixer.deinit(); err != Error::None)
    {
        return err;
    }

    if (Error err = speaker.deinit(); err != Error::None)
    {
        return err;
    }

    return Error::None;
}
