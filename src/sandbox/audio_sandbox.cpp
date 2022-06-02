/**
 * @author Viktor Baranov (github.com/baranov-V-V)
 * @file audio_sandbox.cpp
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

#include "audio_sandbox.hpp"

#include "GLFW/glfw3.h"
#include "core/logger.hpp"
#include "glad/glad.h"
#include "renderer/3d/renderer3d.hpp"

using namespace vulture;

AudioSandbox* this_sandbox;

AudioSandbox::AudioSandbox() : window_(640, 400) {}

int AudioSandbox::Init() {
  this_sandbox = this;

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    LOG_ERROR(AudioSandbox, "Failed to initialize GLAD");
    return -1;
  }

  InputEventManager::SetWindowAndDispatcher(&window_, &dispatcher);

  dispatcher.GetSink<KeyEvent>().Connect<&KeyEventProsess>();

  device = new AudioDevice();
  device->Open();

  context = new AudioContext(device);
  context->MakeCurrent();

  //DELETE THIS//
  alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);

  listener = new AudioListener(context);
  listener->MakeCurrent();
  listener->SetVolume(1);
  listener->SetOrientation({0.f, 1.f, 0.f}, {0.f, 0.f, 1.f});
  listener->SetLocation({0, 0, 0});
  
  manager = new BufferManager();
  manager->LoadAudioFile("res/sounds/woof.wav", "woof");

  context->CreateSource("woof_source");

  vulture::AudioSource::Handle source_handle = context->GetSource("woof_source").value();
  source_handle.SetBuf(manager->GetBuffer("woof").value());
  source_handle.SetLocation({10, 10, 0});
  //source_handle.SetDirection({1, 1, 1});

  return 0;
}

AudioSandbox::~AudioSandbox() {
  delete listener;
  delete context;
  delete manager;

  device->Close();
  delete device;
}

void AudioSandbox::Run() {
  int32_t frame_buffer_width = 0;
  int32_t frame_buffer_height = 0;
  glfwGetFramebufferSize(window_.GetNativeWindow(), &frame_buffer_width, &frame_buffer_height);

  // FIXME:
  glfwSetInputMode(window_.GetNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPos(window_.GetNativeWindow(), frame_buffer_width / 2, frame_buffer_height / 2);

  clock_t time_start = clock();
  while (running) {
    float timestep = (0.0f + clock() - time_start) / CLOCKS_PER_SEC;
    time_start = clock();

    InputEventManager::TriggerEvents();

    glfwSwapBuffers(window_.GetNativeWindow());
    window_.SetFPSToTitle(1 / timestep);
  }
}

void KeyEventProsess(const vulture::KeyEvent& event) {
  int key = event.key;
  int action = (int)event.action;

  if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) {
    this_sandbox->running = false;
  }

  if (key == GLFW_KEY_E && action == GLFW_PRESS) {
    this_sandbox->context->GetSource("woof_source").value().Play();
    //insert sound here
  }

  if (key == GLFW_KEY_W && action == GLFW_PRESS) {
    //this_sandbox->x += 1;
    this_sandbox->listener_pos.x += 1;
    this_sandbox->listener->SetLocation(this_sandbox->listener_pos);
    //LOG_INFO(KeyEvent, "Listener pos w ({}, {}, {})", this_sandbox->listener_pos.x, this_sandbox->listener_pos.y, this_sandbox->listener_pos.z);
  }

  if (key == GLFW_KEY_S && action == GLFW_PRESS) {
    //this_sandbox->x -= 1;
    this_sandbox->listener_pos.x -= 1;
    this_sandbox->listener->SetLocation(this_sandbox->listener_pos);
  }

  if (key == GLFW_KEY_A && action == GLFW_PRESS) {
    //this_sandbox->y -= 1;
    this_sandbox->listener_pos.y -= 1;
    this_sandbox->listener->SetLocation(this_sandbox->listener_pos);
  }

  if (key == GLFW_KEY_D && action == GLFW_PRESS) {
    //this_sandbox->y += 1;
    this_sandbox->listener_pos.y += 1;
    this_sandbox->listener->SetLocation(this_sandbox->listener_pos);
  }

  LOG_INFO(KeyEvent, "Listener pos ({}, {}, {})", this_sandbox->listener_pos.x, this_sandbox->listener_pos.y, this_sandbox->listener_pos.z);
  
}