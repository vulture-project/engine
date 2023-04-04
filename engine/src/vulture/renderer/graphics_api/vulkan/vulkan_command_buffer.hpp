/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file vulkan_command_buffer.hpp
 * @date 2023-03-18
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

#ifdef __APPLE__
#define VK_USE_PLATFORM_MACOS_MVK
#define VK_USE_PLATFORM_METAL_EXT
#define VK_ENABLE_BETA_EXTENSIONS
#endif
#include <vulkan/vulkan.h>

#include <vulture/renderer/graphics_api/command_buffer.hpp>

namespace vulture {

class VulkanRenderDevice;

class VulkanCommandBuffer final : public CommandBuffer {
 public:
  VulkanCommandBuffer(CommandBufferType type, VulkanRenderDevice& device, bool temporary = false);
  ~VulkanCommandBuffer() override;

  RenderDevice& GetDevice() override;

  void Begin() override;
  void End() override;
  void Submit(FenceHandle signal_fence, SemaphoreHandle signal_semaphore) override;

  void Reset() override;

  void GenerateMipmaps(TextureHandle texture, TextureLayout final_layout) override;
  void TransitionLayout(TextureHandle texture, TextureLayout old_layout, TextureLayout new_layout) override;

  void CopyBuffer(BufferHandle src_buffer, BufferHandle dst_buffer, uint32_t size, uint32_t src_offset,
                  uint32_t dst_offset) override;

  void CopyBufferToTexture(BufferHandle buffer, TextureHandle texture, uint32_t width, uint32_t height,
                           uint32_t layer, uint32_t layers_count) override;
  void CopyTextureToBuffer(TextureHandle texture, BufferHandle buffer, uint32_t width, uint32_t height,
                           uint32_t layer, uint32_t layers_count) override;
  void CopyTexture(TextureHandle src_texture, TextureHandle dst_texture, uint32_t width, uint32_t height) override;

  /************************************************************************************************
   * Graphics/Compute Commands (depending on the usage)
   ************************************************************************************************/
  void RenderPassBegin(const RenderPassBeginInfo& begin_info) override;
  void RenderPassEnd() override;

  void CmdNextSubpass() override;

  void CmdBindDescriptorSets(PipelineHandle pipeline, uint32_t first_set_idx, uint32_t count,
                                     const DescriptorSetHandle* descriptor_sets) override;

  void CmdPushConstants(PipelineHandle pipeline, const void* data, uint32_t offset, uint32_t size,
                                ShaderStageFlags shader_stages) override;

  /************************************************************************************************
   * Graphics Commands
   ************************************************************************************************/
  void CmdBindGraphicsPipeline(PipelineHandle pipeline) override;

  void CmdSetViewports(uint32_t viewports_count, const Viewport* viewports) override;

  void CmdBindVertexBuffers(uint32_t first_binding, uint32_t count, const BufferHandle* vertex_buffers,
                                    const uint64_t* offsets) override;

  void CmdBindIndexBuffer(BufferHandle index_buffer, uint64_t offset) override;

  void CmdDraw(uint32_t vertices_count,
                       uint32_t first_vertex,
                       uint32_t instances_count,
                       uint32_t first_instance) override;

  void CmdDrawIndexed(uint32_t indices_count,
                              uint32_t first_index,
                              int32_t  vertex_offset,
                              uint32_t instances_count,
                              uint32_t first_instance) override;

 private:
  VulkanRenderDevice& device_;
  VkCommandBuffer     vk_command_buffer_{VK_NULL_HANDLE};
  bool                temporary_{false};

  friend class VulkanImGuiImplementation;
};

}  // namespace vulture
