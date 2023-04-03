/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file preview_panel.hpp
 * @date 2022-08-04
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

#include <vulture/platform/event.hpp>
#include <vulture/renderer/texture.hpp>
#include <vulture/scene/scene.hpp>
#include <vulture/ui/imgui_implementation.hpp>
#include <ImGuizmo.h>

namespace vulture {

class PreviewPanel {
 public:
  PreviewPanel(RenderDevice& device, ImGuiImplementation& imgui_implementation, Dispatcher& event_dispatcher);

  void OnInit();
  void OnClose();
  void OnRender(Scene& scene, fennecs::EntityHandle selected_entity);

  SharedPtr<Texture> GetTexture();

  void OnResize();
  bool Resized() const;

  void OnKeyPressed(const KeyEvent& event);

 private:
  void UpdateIsResized(uint32_t width, uint32_t height);

 private:
  RenderDevice& device_;
  ImGuiImplementation& imgui_implementation_;

  SharedPtr<Texture> color_output_{nullptr};
  ImGuiTextureHandle imgui_color_output_{nullptr};

  bool resized_{false};
  uint32_t resized_width_{0};
  uint32_t resized_height_{0};

  ImGuizmo::OPERATION gizmo_operation_{ImGuizmo::TRANSLATE};
};

}  // namespace vulture