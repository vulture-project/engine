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

#include <veditor/panels/entities_panel.hpp>
#include <veditor/panels/inspector_panel.hpp>
// #include <veditor/panels/renderer_info_panel.hpp>
#include <veditor/panels/preview_panel.hpp>
#include <vulture/app/app.hpp>
#include <vulture/event_system/event_system.hpp>
#include <vulture/platform/event.hpp>
#include <vulture/platform/window.hpp>
#include <vulture/scene/scene.hpp>
#include <vulture/ui/imgui_implementation.hpp>

struct ImGuiIO;

namespace vulture {

class EditorApp : public Application {
 public:
  EditorApp();
  virtual ~EditorApp() override;

  virtual int Init() override;
  virtual void Run() override;

 private:
  void OnQuit(const QuitEvent&);

  void OnResize();

  void CreateSwapchain();
  void DestroySwapchain();

  void CreateFrameData();
  void DestroyFrameData();

  void CreateRenderer();

  void Render();
  void RenderUI(CommandBuffer& command_buffer, uint32_t texture_idx);

  void OnUpdateImGuiStyle();

  // void OnGuiRender();

  // void OnInitImGui();
  // void OnUpdateImGuiStyle();
  // void OnFrameStartImGui();
  // void OnFrameFinishImGui();
  // void OnCloseImGui();

 private:
  struct Frame {
    CommandBuffer* command_buffer{nullptr};
    FenceHandle fence_render_finished{kInvalidRenderResourceHandle};
    SemaphoreHandle semaphore_render_finished{kInvalidRenderResourceHandle};
    SemaphoreHandle semaphore_swapchain_texture_ready{kInvalidRenderResourceHandle};
  };

 private:
  Window window_;
  Dispatcher event_dispatcher_;
  bool running_{true};

  RenderDevice& device_;
  UniquePtr<Renderer> renderer_;

  Array<Frame, kFramesInFlight> frames_;

  /* Swapchain */
  SwapchainHandle swapchain_{kInvalidRenderResourceHandle};
  Vector<TextureHandle> swapchain_textures_;

  /* Scene */
  Scene scene_;
  fennecs::EntityHandle selected_entity_{fennecs::EntityHandle::Null()};

  /* Time */
  Timer timer_;
  float current_time_{0};
  float current_timestep_{0};

  /* UI */
  UniquePtr<ImGuiImplementation> imgui_implementation_;

  /* Panels */
  UniquePtr<PreviewPanel> preview_panel_;
  UniquePtr<EntitiesPanel> entities_panel_;
  UniquePtr<InspectorPanel> inspector_panel_;
  // RendererInfoPanel renderer_info_panel_;
};

}  // namespace vulture