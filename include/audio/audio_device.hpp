/**
 * @author Viktor Baranov (github.com/baranov-V-V)
 * @file api.hpp
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

#pragma once

#include <set>
#include <vector>
#include <string>

#include <AL/alc.h>
#include <AL/al.h>

namespace vulture {

class AudioContext;

//----------------------------------------TODO---------------------------------------
//1)rewrite device class:
//2)add list of available devices
//3)more convinient name to it

class AudioDevice {
 public:
  friend class AudioContext;

  AudioDevice();
  ~AudioDevice();

  void Open(const char* device_name = nullptr);

  std::vector<std::string> GetAvailableDevices();
  void DumpAvailableDevices();

  void Close();

  AudioContext CreateContext();

private:
  ALCdevice* al_device_;
  size_t context_count_;
};

} // namespace vulture
