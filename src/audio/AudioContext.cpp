#include "audio/AudioContext.h"
#include <AL/al.h>

namespace sound {

    AudioContext::AudioContext() : pool_(nullptr), context_(nullptr) {}

    AudioContext::~AudioContext() {}

    bool AudioContext::Create(ALCdevice* device) {
        context_ = alcCreateContext(device, nullptr);  // create context
        assert(context_ != nullptr);
        ALCcontext* prev_context_ = alcGetCurrentContext();
        alcMakeContextCurrent(context_);
        pool_ = new SourcePool(4);
        alcMakeContextCurrent(prev_context_);
        return (context_ != nullptr);
    }

    bool AudioContext::Destroy() {
        delete pool_;
        alcDestroyContext(context_);
        return false;
    }

    bool AudioContext::MakeCurrentPlaying() {
        return alcMakeContextCurrent(context_);
    }

    void AudioContext::GetLocation(Vec3f& loc) {
        alGetListener3f(AL_POSITION, &loc.x, &loc.y, &loc.z);
    }

    void AudioContext::GetOrientation(Vec3f& at, Vec3f& up) {
        float buf[6] = {};
        alGetListenerfv(AL_ORIENTATION, buf);
        at = {buf[0], buf[1], buf[2]};
        up = {buf[3], buf[4], buf[5]};
    }

    float AudioContext::GetVolume() {
        float volume;
        alGetListenerf(AL_GAIN, &volume);
        return volume;
    }

    void AudioContext::SetLocation(const Vec3f& location) {
        alListener3f(AL_POSITION, location.x, location.y, location.z);
    }

    void AudioContext::SetOrientation(const Vec3f& at, const Vec3f& up) {
        float buf[6] = {};
        buf[0] = at.x;
        buf[1] = at.y;
        buf[2] = at.z;
        buf[3] = up.x;
        buf[4] = up.y;
        buf[5] = up.z;
        alListenerfv(AL_ORIENTATION, buf);
    }

    void AudioContext::SetVolume(const float& volume) {
        float new_volume = volume;
        if (new_volume < 0.f) {
            new_volume = 0.f;
        }
        else if (new_volume > 5.f) {
            new_volume = 5.f;
        }

        alListenerf(AL_GAIN, new_volume);
    }

    Source* AudioContext::CreateSource() {
        return pool_->Acquire();
    }

    void AudioContext::DestroySource(Source* source) {
        pool_->Release(source);
    }

} //namespace sound
