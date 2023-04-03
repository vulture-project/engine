/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file imgui_implementation.hpp
 * @date 2023-03-29
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

#include <imgui.h>

#include <vulture/platform/window.hpp>
#include <vulture/renderer/graphics_api/render_device.hpp>

namespace vulture {

using ImGuiTextureHandle = void*;

/**
 * @brief ImGui implementation interface.
 * 
 * Valid usage is the following:
 * 
 * 1. Create Window and RenderDevice
 * 2. auto imgui_impl = ImGuiImplementation::Create();
 * 3. imgui_impl->Init(window, device);
 * 4. Each frame:
 *     a. ... do app logic and rendering
 *     b. imgui_impl->FrameStart();
 *     c. ... render ui using ImGui API
 *     d. imgui_impl->FrameEnd();
 */
class ImGuiImplementation {
 public:
  virtual ~ImGuiImplementation() = default;

  virtual void OnResize(SwapchainHandle new_swapchain) = 0;

  virtual void UpdateFonts(const char* filename, float size) = 0;

  virtual void FrameStart() = 0;
  virtual void Render(CommandBuffer& command_buffer, uint32_t swapchain_texture_idx) = 0;
  virtual void FrameEnd() = 0;

  virtual ImGuiTextureHandle AddTextureUI(TextureHandle texture) = 0;
  virtual void RemoveTextureUI(ImGuiTextureHandle imgui_texture) = 0;

  static UniquePtr<ImGuiImplementation> Create(Window& window, RenderDevice& device, SwapchainHandle swapchain);
};

}  // namespace vulture