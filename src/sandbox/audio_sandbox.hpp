/**
 * @author Viktor Baranov (github.com/baranov-V-V)
 * @file audio_sandbox.hpp
 * @date 2022-06-02
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

#include "app/app.hpp"
#include "event_system/event_system.hpp"
#include "platform/event.hpp"
#include "platform/window.hpp"

#include "audio/audio_device.hpp"
#include "audio/audio_context.hpp"
#include "audio/audio_source.hpp"
#include "audio/audio_listener.hpp"
#include "audio/buffer_manager.hpp"

using namespace vulture;

void KeyEventProsess(const vulture::KeyEvent& event);

class AudioSandbox : public vulture::Application {
 public:
  AudioSandbox();
  ~AudioSandbox();

  virtual int Init() override;
  virtual void Run() override;

 public:
  vulture::Window window_;

  bool running = true;

  vulture::Dispatcher dispatcher;

  AudioDevice* device;
  AudioContext* context;
  BufferManager* manager;
  AudioListener* listener;

  glm::vec3 listener_pos = {0, 0, 0};

  float x = 0;
  float y = 0;
};