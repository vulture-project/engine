/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file vulkan_command_buffer.cpp
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

#include <vulture/renderer/graphics_api/vulkan/vulkan_command_buffer.hpp>
#include <vulture/renderer/graphics_api/vulkan/vulkan_render_device.hpp>
#include <vulture/renderer/graphics_api/vulkan/vulkan_utils.hpp>

using namespace vulture;

VulkanCommandBuffer::VulkanCommandBuffer(CommandBufferType type, VulkanRenderDevice& device, bool temporary)
    : CommandBuffer(type), device_(device), temporary_(temporary) {
  if (temporary) {
    vk_command_buffer_ = device_.CreateCommandBuffer(device_.transient_command_pool_);
  } else {
    vk_command_buffer_ = device_.CreateCommandBuffer(device_.main_command_pool_);
  }
}

VulkanCommandBuffer::~VulkanCommandBuffer() {
  if (temporary_) {
    vkFreeCommandBuffers(device_.device_, device_.transient_command_pool_, 1, &vk_command_buffer_);
  } else {
    vkFreeCommandBuffers(device_.device_, device_.main_command_pool_, 1, &vk_command_buffer_);
  }

  vk_command_buffer_ = VK_NULL_HANDLE;
}

RenderDevice& VulkanCommandBuffer::GetDevice() {
  return device_;
}

void VulkanCommandBuffer::Begin() {
  VkCommandBufferBeginInfo vk_command_buffer_begin_info{};
  vk_command_buffer_begin_info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  vk_command_buffer_begin_info.flags            = 0;
  vk_command_buffer_begin_info.pInheritanceInfo = nullptr;  // For secondary command buffers

  VkResult result = vkBeginCommandBuffer(vk_command_buffer_, &vk_command_buffer_begin_info);
  assert(result == VK_SUCCESS);
}

void VulkanCommandBuffer::End() {
  VkResult result = vkEndCommandBuffer(vk_command_buffer_);
  assert(result == VK_SUCCESS);
}

void VulkanCommandBuffer::Submit() {
  VkSubmitInfo submit_info{};
  submit_info.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount   = 1;
  submit_info.pCommandBuffers      = &vk_command_buffer_;
  submit_info.signalSemaphoreCount = 0;
  submit_info.pSignalSemaphores    = nullptr;

  VkQueue vk_queue{VK_NULL_HANDLE};
  switch (type_) {
    case CommandBufferType::kGraphics: { vk_queue = device_.graphics_queue_;   break; }
    default:                           { assert(!"Invalid CommandBufferType"); break; }
  }

  VkResult result = vkQueueSubmit(vk_queue, /*submitCount=*/1, &submit_info, /*fence=*/VK_NULL_HANDLE);
  assert(result == VK_SUCCESS);
}

void VulkanCommandBuffer::Reset() {
  assert(!temporary_);

  VkResult result = vkResetCommandBuffer(vk_command_buffer_, /*flags=*/0);
  assert(result == VK_SUCCESS);
}

void VulkanCommandBuffer::GenerateMipmaps(TextureHandle handle, TextureLayout final_layout) {
  VulkanTexture& texture         = device_.GetVulkanTexture(handle);
  VkImageLayout  vk_final_layout = GetVKImageLayout(final_layout);

  /* Check if format supports linear blitting */
  VkFormatProperties format_properties{};
  vkGetPhysicalDeviceFormatProperties(device_.physical_device_, GetVKFormat(texture.specification.format),
                                      &format_properties);
  if (!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
    assert(!"Texture image format does not support linear blitting!");
  }

  VkImageMemoryBarrier barrier{};
  barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.image                           = texture.vk_image;
  barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
  barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount     = 1;
  barrier.subresourceRange.levelCount     = 1;

  uint32_t mip_width  = texture.specification.width;
  uint32_t mip_height = texture.specification.height;
  for (uint32_t i = 1; i < texture.specification.mip_levels; ++i) {
    /* Transition level (i - 1) to VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL */
    barrier.subresourceRange.baseMipLevel = i - 1;
    // barrier.oldLayout                     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.oldLayout                     = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout                     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcAccessMask                 = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask                 = VK_ACCESS_TRANSFER_READ_BIT;

    vkCmdPipelineBarrier(vk_command_buffer_,
                         /*srcStageMask=*/VK_PIPELINE_STAGE_TRANSFER_BIT,
                         /*dstStageMask=*/VK_PIPELINE_STAGE_TRANSFER_BIT,
                         /*dependencyFlags=*/0,
                         /*memoryBarrierCount=*/0, /*pMemoryBarriers=*/nullptr,
                         /*bufferMemoryBarrierCount=*/0, /*pBufferMemoryBarriers=*/nullptr,
                         /*imageMemoryBarrierCount=*/1, /*pImageMemoryBarriers=*/&barrier);

    /* Blit from level (i - 1) to level i */
    int32_t src_offset_x = static_cast<int32_t>(mip_width);
    int32_t src_offset_y = static_cast<int32_t>(mip_height);
    int32_t dst_offset_x = static_cast<int32_t>((mip_width  > 1) ? (mip_width  / 2) : 1);
    int32_t dst_offset_y = static_cast<int32_t>((mip_height > 1) ? (mip_height / 2) : 1);

    VkImageBlit blit{};
    blit.srcOffsets[0]                 = {0, 0, 0};
    blit.srcOffsets[1]                 = {src_offset_x, src_offset_y, 1};
    blit.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    blit.srcSubresource.mipLevel       = i - 1;
    blit.srcSubresource.baseArrayLayer = 0;
    blit.srcSubresource.layerCount     = 1;

    blit.dstOffsets[0]                 = {0, 0, 0};
    blit.dstOffsets[1]                 = {dst_offset_x, dst_offset_y, 1};
    blit.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    blit.dstSubresource.mipLevel       = i;
    blit.dstSubresource.baseArrayLayer = 0;
    blit.dstSubresource.layerCount     = 1;

    // Should be submitted to a queue with graphics capabilities!
    vkCmdBlitImage(vk_command_buffer_, /*srcImage=*/texture.vk_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   /*dstImage=*/texture.vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, /*regionCount=*/1,
                   /*pRegions=*/&blit, VK_FILTER_LINEAR);

    /* Transition level (i - 1) to vk_final_layout */
    barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout     = vk_final_layout;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(vk_command_buffer_,
                         /*srcStageMask=*/VK_PIPELINE_STAGE_TRANSFER_BIT,
                         /*dstStageMask=*/VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         /*dependencyFlags=*/0,
                         /*memoryBarrierCount=*/0, /*pMemoryBarriers=*/nullptr,
                         /*bufferMemoryBarrierCount=*/0, /*pBufferMemoryBarriers=*/nullptr,
                         /*imageMemoryBarrierCount=*/1, /*pImageMemoryBarriers=*/&barrier);

    /* Next mip size */
    if (mip_width  > 1) { mip_width  /= 2; }
    if (mip_height > 1) { mip_height /= 2; }
  }

  /* Transition last level (not handled in the loop) */
  barrier.subresourceRange.baseMipLevel = texture.specification.mip_levels - 1;
  barrier.oldLayout                     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  barrier.newLayout                     = vk_final_layout;
  barrier.srcAccessMask                 = VK_ACCESS_TRANSFER_READ_BIT;
  barrier.dstAccessMask                 = VK_ACCESS_SHADER_READ_BIT;

  vkCmdPipelineBarrier(vk_command_buffer_,
                       /*srcStageMask=*/VK_PIPELINE_STAGE_TRANSFER_BIT,
                       /*dstStageMask=*/VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                       /*dependencyFlags=*/0,
                       /*memoryBarrierCount=*/0, /*pMemoryBarriers=*/nullptr,
                       /*bufferMemoryBarrierCount=*/0, /*pBufferMemoryBarriers=*/nullptr,
                       /*imageMemoryBarrierCount=*/1, /*pImageMemoryBarriers=*/&barrier);

}

void VulkanCommandBuffer::TransitionLayout(TextureHandle handle, TextureLayout old_layout, TextureLayout new_layout) {
  VulkanTexture& texture = device_.GetVulkanTexture(handle);

  VkAccessFlags        src_access_mask = 0;
  VkAccessFlags        dst_access_mask = 0;
  VkPipelineStageFlags src_stage_mask  = 0;
  VkPipelineStageFlags dst_stage_mask  = 0;

  if (old_layout == TextureLayout::kUndefined && new_layout == TextureLayout::kTransferDst) {
    src_access_mask = 0;
    dst_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT;

    src_stage_mask  = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;  // Earliest pipeline stage 
    dst_stage_mask  = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (old_layout == TextureLayout::kTransferDst && new_layout == TextureLayout::kShaderReadOnly) {
    src_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT;
    dst_access_mask = VK_ACCESS_SHADER_READ_BIT;

    src_stage_mask  = VK_PIPELINE_STAGE_TRANSFER_BIT;
    dst_stage_mask  = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else {
    // /* FIXME: */
    src_access_mask = 0;
    dst_access_mask = 0;

    src_stage_mask  = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    dst_stage_mask  = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    // assert(!"Unsupported layout transition");
  }

  VkImageMemoryBarrier barrier{};
  barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  // Use oldLayout=VK_IMAGE_LAYOUT_UNDEFINED if don't care about the existing contents
  barrier.oldLayout                       = GetVKImageLayout(old_layout);
  barrier.newLayout                       = GetVKImageLayout(new_layout);
  // Barriers can be used to transfer queue family ownership:
  barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
  barrier.image                           = texture.vk_image;
  barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel   = 0;
  barrier.subresourceRange.levelCount     = texture.specification.mip_levels;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount     = 1;
  barrier.srcAccessMask                   = src_access_mask;
  barrier.dstAccessMask                   = dst_access_mask;

  vkCmdPipelineBarrier(vk_command_buffer_,
                       /*srcStageMask=*/src_stage_mask, /*dstStageMask=*/dst_stage_mask,
                       /*dependencyFlags=*/0,
                       /*memoryBarrierCount=*/0, /*pMemoryBarriers=*/nullptr,
                       /*bufferMemoryBarrierCount=*/0, /*pBufferMemoryBarriers=*/nullptr,
                       /*imageMemoryBarrierCount=*/1, /*pImageMemoryBarriers=*/&barrier);
}

void VulkanCommandBuffer::CopyBuffer(BufferHandle src_buffer_handle, BufferHandle dst_buffer_handle, uint32_t size,
                                     uint32_t src_offset, uint32_t dst_offset) {
  VulkanBuffer& src_buffer = device_.GetVulkanBuffer(src_buffer_handle);
  VulkanBuffer& dst_buffer = device_.GetVulkanBuffer(dst_buffer_handle);

  VkBufferCopy copy_region{};
  copy_region.srcOffset = src_offset;
  copy_region.dstOffset = dst_offset;
  copy_region.size      = size;
  vkCmdCopyBuffer(vk_command_buffer_, src_buffer.vk_buffer, dst_buffer.vk_buffer, /*regionCount=*/1, &copy_region);
}

void VulkanCommandBuffer::CopyBufferToTexture(BufferHandle buffer_handle, TextureHandle texture_handle, uint32_t width,
                                              uint32_t height) {
  VulkanBuffer&  buffer  = device_.GetVulkanBuffer(buffer_handle);
  VulkanTexture& texture = device_.GetVulkanTexture(texture_handle);

  VkBufferImageCopy region{};
  region.bufferOffset      = 0;
  region.bufferRowLength   = 0;
  region.bufferImageHeight = 0;

  region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;  // FIXME: Add depth buffers support
  region.imageSubresource.mipLevel       = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount     = 1;

  region.imageOffset = {0, 0, 0};
  region.imageExtent = {width, height, 1};

  vkCmdCopyBufferToImage(vk_command_buffer_, buffer.vk_buffer, texture.vk_image,
                         /*dstImageLayout=*/VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void VulkanCommandBuffer::CopyTextureToBuffer(TextureHandle texture_handle, BufferHandle buffer_handle, uint32_t width,
                                              uint32_t height) {
  VulkanTexture& texture = device_.GetVulkanTexture(texture_handle);
  VulkanBuffer&  buffer  = device_.GetVulkanBuffer(buffer_handle);

  VkBufferImageCopy region{};
  region.bufferOffset      = 0;
  region.bufferRowLength   = 0;
  region.bufferImageHeight = 0;

  region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;  // FIXME: Add depth buffers support
  region.imageSubresource.mipLevel       = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount     = 1;

  region.imageOffset = {0, 0, 0};
  region.imageExtent = {width, height, 1};

  vkCmdCopyImageToBuffer(vk_command_buffer_, texture.vk_image, /*srcImageLayout=*/VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                         buffer.vk_buffer, 1, &region);
}

void VulkanCommandBuffer::CopyTexture(TextureHandle src_handle, TextureHandle dst_handle, uint32_t width,
                                      uint32_t height) {
  VulkanTexture& src_texture = device_.GetVulkanTexture(src_handle);
  VulkanTexture& dst_texture = device_.GetVulkanTexture(dst_handle);

  VkImageCopy region{};
  region.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;  // FIXME: Add depth buffers support
  region.srcSubresource.mipLevel       = 0;
  region.srcSubresource.baseArrayLayer = 0;
  region.srcSubresource.layerCount     = 1;
  region.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;  // FIXME: Add depth buffers support
  region.dstSubresource.mipLevel       = 0;
  region.dstSubresource.baseArrayLayer = 0;
  region.dstSubresource.layerCount     = 1;

  region.srcOffset = {0, 0, 0};
  region.dstOffset = {0, 0, 0};
  region.extent    = {width, height, 1};

  vkCmdCopyImage(vk_command_buffer_, src_texture.vk_image, /*srcImageLayout=*/VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                 dst_texture.vk_image, /*dstImageLayout=*/VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

/************************************************************************************************
 * Graphics/Compute Commands (depending on the usage)
 ************************************************************************************************/
void VulkanCommandBuffer::RenderPassBegin(const RenderPassBeginInfo& begin_info) {
  assert(device_.frame_began_);  // FIXME: Support offline rendering as well!

  VulkanRenderPass&  render_pass = device_.GetVulkanRenderPass(begin_info.render_pass);
  VulkanFramebuffer& framebuffer = device_.GetVulkanFramebuffer(begin_info.framebuffer);

  /* Render Pass Begin */
  VkRenderPassBeginInfo vk_render_pass_begin_info{};
  vk_render_pass_begin_info.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  vk_render_pass_begin_info.renderPass        = render_pass.vk_render_pass;
  vk_render_pass_begin_info.framebuffer       = framebuffer.vk_framebuffer;
  vk_render_pass_begin_info.renderArea.offset = {static_cast<int32_t>(begin_info.render_area.x),
                                                 static_cast<int32_t>(begin_info.render_area.y)};
  vk_render_pass_begin_info.renderArea.extent = {begin_info.render_area.width, begin_info.render_area.height};
  vk_render_pass_begin_info.clearValueCount   = begin_info.clear_values_count;
  vk_render_pass_begin_info.pClearValues      = reinterpret_cast<const VkClearValue*>(begin_info.clear_values);
  
  // No secondary command buffers
  vkCmdBeginRenderPass(vk_command_buffer_, &vk_render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
}
void VulkanCommandBuffer::RenderPassEnd() {
  vkCmdEndRenderPass(vk_command_buffer_);
}

void VulkanCommandBuffer::CmdNextSubpass() { vkCmdNextSubpass(vk_command_buffer_, VK_SUBPASS_CONTENTS_INLINE); }

void VulkanCommandBuffer::CmdBindDescriptorSets(PipelineHandle pipeline_handle, uint32_t first_set_idx, uint32_t count,
                                                const DescriptorSetHandle* descriptor_sets) {
  assert(device_.pipelines_.find(pipeline_handle) != device_.pipelines_.end());
  VulkanPipeline& pipeline = device_.pipelines_.at(pipeline_handle);
  
  static std::vector<VkDescriptorSet> vk_descriptor_sets;
  vk_descriptor_sets.resize(count);

  for (uint32_t i = 0; i < count; ++i) {
    assert(device_.descriptor_sets_.find(descriptor_sets[i]) != device_.descriptor_sets_.end());
    vk_descriptor_sets[i] = device_.descriptor_sets_.at(descriptor_sets[i]).vk_set;
  }

  vkCmdBindDescriptorSets(vk_command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.vk_pipeline_layout,
                          first_set_idx, count, vk_descriptor_sets.data(), /*dynamicOffsetCount=*/0,
                          /*pDynamicOffsets=*/nullptr);
}

void VulkanCommandBuffer::CmdPushConstants(PipelineHandle pipeline_handle, const void* data, uint32_t offset,
                                           uint32_t size, ShaderStageFlags shader_stages) {
  assert(device_.pipelines_.find(pipeline_handle) != device_.pipelines_.end());
  vkCmdPushConstants(vk_command_buffer_, device_.pipelines_.at(pipeline_handle).vk_pipeline_layout,
                     reinterpret_cast<VkShaderStageFlags>(shader_stages), offset, size, data);
}

/************************************************************************************************
 * Graphics Commands
 ************************************************************************************************/
void VulkanCommandBuffer::CmdBindGraphicsPipeline(PipelineHandle pipeline_handle) {
  assert(device_.pipelines_.find(pipeline_handle) != device_.pipelines_.end());
  VulkanPipeline& pipeline = device_.pipelines_.at(pipeline_handle);

  vkCmdBindPipeline(vk_command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.vk_pipeline);
}

void VulkanCommandBuffer::CmdSetViewports(uint32_t viewports_count, const Viewport* viewports) {
  vkCmdSetViewport(vk_command_buffer_, 0, viewports_count, reinterpret_cast<const VkViewport*>(viewports));
}

void VulkanCommandBuffer::CmdBindVertexBuffers(uint32_t first_binding, uint32_t count,
                                               const BufferHandle* handles, const uint64_t* offsets) {
  static std::vector<VkBuffer> vk_buffers;
  static std::vector<VkDeviceSize> vk_zero_offsets; 
  vk_buffers.resize(count);
  vk_zero_offsets.resize(count);

  for (uint32_t i = 0; i < count; ++i) {
    assert(device_.buffers_.find(handles[i]) != device_.buffers_.end());
    vk_buffers[i] = device_.buffers_.at(handles[i]).vk_buffer;
  }

  if (offsets != nullptr) {
    vkCmdBindVertexBuffers(vk_command_buffer_, first_binding, count, vk_buffers.data(), offsets);
  } else {
    for (uint32_t i = 0; i < count; ++i) {
      vk_zero_offsets[i] = 0;
    }

    vkCmdBindVertexBuffers(vk_command_buffer_, first_binding, count, vk_buffers.data(), vk_zero_offsets.data());
  }
}

void VulkanCommandBuffer::CmdBindIndexBuffer(BufferHandle handle, uint64_t offset) {
  assert(device_.buffers_.find(handle) != device_.buffers_.end());
  vkCmdBindIndexBuffer(vk_command_buffer_, device_.buffers_.at(handle).vk_buffer, offset, VK_INDEX_TYPE_UINT32);
}

void VulkanCommandBuffer::CmdDraw(uint32_t vertices_count, uint32_t first_vertex, uint32_t instances_count,
                                  uint32_t first_instance) {
  vkCmdDraw(vk_command_buffer_, vertices_count, instances_count, first_vertex, first_instance);
}

void VulkanCommandBuffer::CmdDrawIndexed(uint32_t indices_count, uint32_t first_index, int32_t vertex_offset,
                                         uint32_t instances_count, uint32_t first_instance) {
  vkCmdDrawIndexed(vk_command_buffer_, indices_count, instances_count, first_index, vertex_offset, first_instance);
}