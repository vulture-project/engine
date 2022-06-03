/**
 * @author Viktor Baranov (github.com/baranov-V-V)
 * @file audio_context.cpp
 * @date 2022-05-19
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

#include "audio/audio_context.hpp"
#include "audio/audio_listener.hpp"
#include "audio/audio_device.hpp"

namespace vulture {

AudioContext::AudioContext(AudioDevice* device) : pool_(), context_(nullptr), device_owner_(device) {
  context_ = alcCreateContext(device->al_device_, nullptr);
  
  //dont fall on assert
  assert(context_ != nullptr);

  ++device_owner_->context_count_;

  //mb skip this line?
  MakeCurrent();
}

AudioContext::~AudioContext() {
  if (device_owner_ != nullptr) {
    pool_.~SourcePool();
    alcDestroyContext(context_);
    --device_owner_->context_count_;
  }
}

AudioContext::AudioContext(AudioContext&& context) :
  pool_(std::move(context.pool_)),
  context_(std::move(context.context_)),
  device_owner_(std::move(context.device_owner_)) {
  context.device_owner_ = nullptr;
}

bool AudioContext::MakeCurrent() {
  return alcMakeContextCurrent(context_);
}

bool AudioContext::IsCurrent() {
  return alcGetCurrentContext() == context_;
}

AudioListener AudioContext::GetNewListener() {
  return AudioListener(this);
}

bool AudioContext::CreateSource(const char* name) {
  return pool_.CreateSource(this, name);
}

std::optional<AudioSource::Handle> AudioContext::GetSource(const char* name) {
  return pool_.GetSource(name);
}
	
bool AudioContext::DestroySource(const char* name) {
  return pool_.Destroy(name);
}

void AudioContext::StopAllSounds() {
  for (SourcePool::Iterator it = pool_.Begin(); it != pool_.End(); ++it) {
    (*it).Stop();
  }
}

void AudioContext::PauseAllSounds() {
  for (SourcePool::Iterator it = pool_.Begin(); it != pool_.End(); ++it) {
    (*it).Pause();
  }
}

void AudioContext::ResumeAllSounds() {
  for (SourcePool::Iterator it = pool_.Begin(); it != pool_.End(); ++it) {
    (*it).Resume();
  }
}

} // namespace vulture
