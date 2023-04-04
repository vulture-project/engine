/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file command_buffer.hpp
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

#include <vulture/renderer/graphics_api/pipeline.hpp>
#include <vulture/renderer/graphics_api/render_pass.hpp>
#include <vulture/renderer/graphics_api/render_resource_handles.hpp>

namespace vulture {

class RenderDevice;

/* Viewport */
struct Viewport {
  float x         {0.0f};  ///< In pixels
  float y         {0.0f};  ///< In pixels
  float width     {0.0f};  ///< In pixels
  float height    {0.0f};  ///< In pixels
  float min_depth {0.0f};  ///< Normalized
  float max_depth {1.0f};  ///< Normalized
};

/* Render Area */
struct RenderArea {
  uint32_t x      {0};  ///< In pixels
  uint32_t y      {0};  ///< In pixels
  uint32_t width  {0};  ///< In pixels
  uint32_t height {0};  ///< In pixels
};

enum class CommandBufferType {
  kInvalid,
  kGraphics,
  // kTransfer,
  // kCompute  // TODO: (tralf-strues) Compute pipeline is not supported at the moment
};

class CommandBuffer {
 public:
  virtual ~CommandBuffer() = default;

  virtual RenderDevice& GetDevice() = 0;

  virtual void Begin() = 0;
  virtual void End() = 0;
  virtual void Submit(FenceHandle signal_fence = kInvalidRenderResourceHandle,
                      SemaphoreHandle signal_semaphore = kInvalidRenderResourceHandle) = 0;

  virtual void Reset() = 0;

  virtual void GenerateMipmaps(TextureHandle texture, TextureLayout final_layout = TextureLayout::kShaderReadOnly) = 0;
  virtual void TransitionLayout(TextureHandle texture, TextureLayout old_layout, TextureLayout new_layout) = 0;

  virtual void CopyBuffer(BufferHandle src_buffer, BufferHandle dst_buffer, uint32_t size, uint32_t src_offset = 0,
                          uint32_t dst_offset = 0) = 0;

  /**
   * @brief Copy data from the buffer to the texture.
   * 
   * @param buffer 
   * @param texture 
   * @param width 
   * @param height
   * @param start_layer Start layer to write to (for regular 2D images it is always 0, for cube maps can be 0..5)
   * @param layers_count How many layers to copy
   * 
   * @warning Texture must be in either @ref{TextureLayout::kTransferDst} or @ref{TextureLayout::kGeneral} layouts.
   */
  virtual void CopyBufferToTexture(BufferHandle buffer, TextureHandle texture, uint32_t width, uint32_t height,
                                   uint32_t start_layer = 0, uint32_t layers_count = 1) = 0;

  /**
   * @brief Copy data from the texture to the buffer.
   * 
   * @param texture 
   * @param buffer 
   * @param width 
   * @param height
   * @param start_layer Layer to write to (for regular 2D images it is always 0, for cube maps can be from 0 to 5)
   * @param layers_count How many layers to copy
   * 
   * @warning Texture must be in either @ref{TextureLayout::kTransferSrc} or @ref{TextureLayout::kGeneral} layouts.
   */
  virtual void CopyTextureToBuffer(TextureHandle texture, BufferHandle buffer, uint32_t width, uint32_t height,
                                   uint32_t start_layer = 0, uint32_t layers_count = 1) = 0;

  /**
   * @brief Copy data from the src to the dst texture.
   * 
   * @param src_texture 
   * @param dst_texture 
   * @param width 
   * @param height 
   * 
   * @warning src_texture must be in either @ref{TextureLayout::kTransferSrc} or @ref{TextureLayout::kGeneral} layouts.
   * @warning dst_texture must be in either @ref{TextureLayout::kTransferDst} or @ref{TextureLayout::kGeneral} layouts.
   */
  virtual void CopyTexture(TextureHandle src_texture, TextureHandle dst_texture, uint32_t width, uint32_t height) = 0;

  /************************************************************************************************
   * Graphics/Compute Commands (depending on the usage)
   ************************************************************************************************/
  struct RenderPassBeginInfo {
    RenderPassHandle  render_pass{kInvalidRenderResourceHandle};
    FramebufferHandle framebuffer{kInvalidRenderResourceHandle};
    RenderArea        render_area{};  ///< Top-left corner has coordinates (0, 0)

    uint32_t clear_values_count{0};

    /**
     * @brief Pointer to an array of clear values indexed by attachment number.
     * @note If attachment i doesn't have load_op set to AttachmentLoadOperation::kClear,
     *       then clear value i is ignored.
     */
    const ClearValue* clear_values{nullptr};
  };

  virtual void RenderPassBegin(const RenderPassBeginInfo& begin_info) = 0;
  virtual void RenderPassEnd() = 0;

  virtual void CmdNextSubpass() = 0;

  virtual void CmdBindDescriptorSets(PipelineHandle pipeline, uint32_t first_set_idx, uint32_t count,
                                     const DescriptorSetHandle* descriptor_sets) = 0;

  void CmdBindDescriptorSet(PipelineHandle pipeline, uint32_t set, DescriptorSetHandle descriptor_set) {
    CmdBindDescriptorSets(pipeline, set, 1, &descriptor_set);
  }

  virtual void CmdPushConstants(PipelineHandle pipeline, const void* data, uint32_t offset, uint32_t size,
                                ShaderStageFlags shader_stages) = 0;

  /************************************************************************************************
   * Graphics Commands
   ************************************************************************************************/
  virtual void CmdBindGraphicsPipeline(PipelineHandle pipeline) = 0;

  virtual void CmdSetViewports(uint32_t viewports_count, const Viewport* viewports) = 0;

  virtual void CmdBindVertexBuffers(uint32_t first_binding, uint32_t count, const BufferHandle* vertex_buffers,
                                    const uint64_t* offsets = nullptr) = 0;

  void CmdBindVertexBuffer(uint32_t binding, BufferHandle vertex_buffer, uint64_t offset = 0) {
    CmdBindVertexBuffers(binding, 1, &vertex_buffer, &offset);
  }

  virtual void CmdBindIndexBuffer(BufferHandle index_buffer, uint64_t offset = 0) = 0;

  virtual void CmdDraw(uint32_t vertices_count,
                       uint32_t first_vertex    = 0,
                       uint32_t instances_count = 1,
                       uint32_t first_instance  = 0) = 0;

  virtual void CmdDrawIndexed(uint32_t indices_count,
                              uint32_t first_index     = 0,
                              int32_t  vertex_offset   = 0,
                              uint32_t instances_count = 1,
                              uint32_t first_instance  = 0) = 0;

 protected:
  CommandBuffer(CommandBufferType type) : type_(type) {}

 protected:
  CommandBufferType type_{CommandBufferType::kInvalid};
};

}  // namespace vulture