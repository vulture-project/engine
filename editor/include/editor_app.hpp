/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file editor_app.hpp
 * @date 2022-08-02
 *
 * The MIT License (MIT)
 * Copyright (c) 2022 Nikita Mochalov
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
#include "panels/preview_panel.hpp"
#include "platform/event.hpp"
#include "platform/window.hpp"
#include "renderer/framebuffer.hpp"
#include "scene/scene.hpp"

struct ImGuiIO;

namespace vulture {

class EditorApp : public Application {
 public:
  EditorApp();
  virtual ~EditorApp() override;

  virtual int Init() override;
  virtual void Run() override;

 private:
  void OnGuiRender();
  void OnQuit(const QuitEvent&);

  void OnInitImGui();
  void OnFrameStartImGui();
  void OnFrameFinishImGui();
  void OnCloseImGui();

 private:
  Window window_;
  Dispatcher event_dispatcher_;
  bool running_{false};

  ImGuiIO* imgui_io_{nullptr};

  /* Scene */
  Scene scene_;
  
  /* Panels */
  PreviewPanel preview_panel_;
};

}  // namespace vulture