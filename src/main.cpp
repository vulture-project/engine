/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file main.cpp
 * @date 2022-04-26
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

#include "sandbox/sandbox_app.hpp"

#include "audio/audio_device.hpp"
#include "audio/audio_source.hpp"
#include "audio/audio_context.hpp"
#include "audio/buffer_manager.hpp"

#include <stdio.h>
#include <cassert>
#include <filesystem>
#include <iostream>

#include <fcntl.h>
#include <unistd.h>

using namespace vulture;

int main() {
  /*
  AudioDevice device;
  device.DumpAvailableDevices();
  device.Open();

  {
    AudioContext context = device.CreateContext();
    context.CreateSource("s1");

    BufferManager buffer_manager;
    buffer_manager.LoadAudioFile("../res/sounds/woof.wav", "5");

    { //work with handle SEGV
      vulture::AudioSource::Handle s1_h = context.GetSource("s1").value();
      s1_h.SetBuf(buffer_manager.GetBuffer("5").value());
      s1_h.Play();
      sleep(3);
    }

  }
  
  device.Close();

  */
 
  SandboxApp app{};
  app.Init();
  app.Run();
  return 0;
}
