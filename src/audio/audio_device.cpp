/**
 * @author Viktor Baranov (github.com/baranov-V-V)
 * @file audio_device.cpp
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

#include "audio/audio_device.hpp"

#include <cstring>
#include <iostream>

#include "audio/audio_context.hpp"
#include "core/logger.hpp"

namespace vulture {

AudioDevice::AudioDevice() : al_device_(nullptr), context_count_(0) {}

AudioDevice::~AudioDevice() {}

void AudioDevice::Open(const char* device_name) {
  al_device_ = alcOpenDevice(device_name);

  const ALCchar* name = nullptr;
  if (alcIsExtensionPresent(al_device_, "ALC_ENUMERATE_ALL_EXT")) {
    name = alcGetString(al_device_, ALC_ALL_DEVICES_SPECIFIER);
  }
  if (!name || alcGetError(al_device_) != AL_NO_ERROR) {
    name = alcGetString(al_device_, ALC_DEVICE_SPECIFIER);
  }

  alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);

  LOG_INFO(AudioDevice, "Opened {}", name);
}

std::vector<std::string> AudioDevice::GetAvailableDevices() {
  const char* device_names = alcGetString(nullptr, ALC_CAPTURE_DEVICE_SPECIFIER);
  char* curr_device = const_cast<char*>(device_names);

  std::vector<std::string> device_list;

  while (*curr_device != '\0') {
    device_list.emplace_back(curr_device);
    curr_device = std::strchr(curr_device, '\0') + 1;
  }

  return device_list;
}

void AudioDevice::DumpAvailableDevices() {
  std::vector<std::string> device_list = GetAvailableDevices();
  LOG_INFO(AudioDevice, "Available devices:");
  for (const auto& device_name : device_list) {
    LOG_INFO(AudioDevice, "{}", device_name);
  }
}

void AudioDevice::Close() {
  if (context_count_ != 0) {
    const char* device_name = alcGetString(al_device_, ALC_DEVICE_SPECIFIER);
    LOG_ERROR(AudioDevice, "At Close Device: {} context_count is not 0: {}", device_name, context_count_);
  }
  LOG_INFO(AudioDevice, "Close device");

  alcCloseDevice(al_device_);
}

AudioContext AudioDevice::CreateContext() { return AudioContext(this); }

}  // namespace vulture