/**
 * @author Viktor Baranov (github.com/baranov-V-V)
 * @file audio_listener.cpp
 * @date 2022-05-23
 * 
 * The MIT License (MIT)
 * Copyright (c) vulture-project
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <AL/al.h>

#include <audio/audio_listener.hpp>
#include <audio/audio_context.hpp>

namespace vulture {
  
AudioListener::AudioListener(AudioContext* context) : context_(context) {}

bool AudioListener::IsCurrent() {
  return context_->IsCurrent();
}

void AudioListener::GetLocation(Vec3f& loc) {
  if (context_->IsCurrent()) {
    alGetListener3f(AL_POSITION, &loc.x, &loc.y, &loc.z);
  }
}

void AudioListener::GetOrientation(Vec3f& at, Vec3f& up) {
  if (context_->IsCurrent()) {
    float buf[6] = {};
    alGetListenerfv(AL_ORIENTATION, buf);
    at = {buf[0], buf[1], buf[2]};
    up = {buf[3], buf[4], buf[5]};
  }
}

float AudioListener::GetVolume() {
  if (context_->IsCurrent()) {
    float volume;
    alGetListenerf(AL_GAIN, &volume);
    return volume;
  } else {
    return 0;
  }
}

void AudioListener::SetLocation(const Vec3f& location) {
  if (context_->IsCurrent()) {
    alListener3f(AL_POSITION, location.x, location.y, location.z);
  }
}

void AudioListener::SetOrientation(const Vec3f& at, const Vec3f& up) {
  if (context_->IsCurrent()) {
    float buf[6] = {};
    buf[0] = at.x;
    buf[1] = at.y;
    buf[2] = at.z;
    buf[3] = up.x;
    buf[4] = up.y;
    buf[5] = up.z;
    alListenerfv(AL_ORIENTATION, buf);
  }
}

void AudioListener::SetVolume(const float& volume) {
  if (context_->IsCurrent()) {
    float new_volume = volume;
    if (new_volume < 0.f) {
        new_volume = 0.f;
    }
    else if (new_volume > 1.f) {
        new_volume = 1.f;
    }

    alListenerf(AL_GAIN, new_volume);
  }
}

} // namespace vulture
