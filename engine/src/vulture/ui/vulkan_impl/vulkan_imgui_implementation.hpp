/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file vulkan_imgui_implementation.hpp
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

#include <vulture/renderer/graphics_api/vulkan/vulkan_command_buffer.hpp>
#include <vulture/renderer/graphics_api/vulkan/vulkan_render_device.hpp>
#include <vulture/ui/imgui_implementation.hpp>

namespace vulture {

class VulkanImGuiImplementation : public ImGuiImplementation {
 public:
  VulkanImGuiImplementation(Window& window, VulkanRenderDevice& device, SwapchainHandle swapchain);

  ~VulkanImGuiImplementation() override;

  void OnResize(SwapchainHandle new_swapchain) override;

  void UpdateFonts(const char* filename, float size) override;

  void FrameStart() override;
  void Render(CommandBuffer& command_buffer, uint32_t swapchain_texture_idx) override;
  void FrameEnd() override;

  ImGuiTextureHandle AddTextureUI(TextureHandle texture, uint32_t layer) override;
  void RemoveTextureUI(ImGuiTextureHandle imgui_texture) override;

 private:
  void InitImplementation();
  void CreateDescriptorPool();
  void CreateRenderPass();
  void CreateTextureUISampler();

  void RecreateFramebuffers();

 private:
  Window& window_;
  VulkanRenderDevice& device_;
  VulkanSwapchain* swapchain_{nullptr};

  /* Vulkan objects */
  VkDescriptorPool descriptor_pool_{VK_NULL_HANDLE};
  VkRenderPass render_pass_{VK_NULL_HANDLE};
  Vector<VkFramebuffer> framebuffers_;

  VkSampler texture_ui_sampler_{VK_NULL_HANDLE};

  /* ImGui */
  bool dockspace_open_{true};
  bool opt_fullscreen_{true};
  ImGuiDockNodeFlags dockspace_flags_{ImGuiDockNodeFlags_None};
};

}  // namespace vulture