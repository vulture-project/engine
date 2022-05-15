/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file sandbox_app.hpp
 * @date 2022-05-15
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
#include "platform/event.hpp"
#include "platform/window.hpp"
#include "renderer/3d/scene3d.hpp"

class SandboxApp : public vulture::Application {
 public:
  SandboxApp();

  virtual int Init() override;
  virtual void Run() override;

  void ProcessEvent(vulture::Event* event, bool* running);
  void ProcessMoveEvent(vulture::Event* event);
  void ProcessKeyEvent(vulture::Event* event);

 private:
  vulture::Window window_;

  vulture::Scene3D scene_;

  vulture::LightSourceNode3D* directional_light_node_{nullptr};
  vulture::LightSourceNode3D* spot_light_node_{nullptr};
  vulture::ModelNode3D* skybox_node_{nullptr};
};