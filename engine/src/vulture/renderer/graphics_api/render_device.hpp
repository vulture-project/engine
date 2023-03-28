/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file render_device.hpp
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

#include <vulture/renderer/graphics_api/buffer.hpp>
#include <vulture/renderer/graphics_api/command_buffer.hpp>
#include <vulture/renderer/graphics_api/device_features.hpp>
#include <vulture/renderer/graphics_api/pipeline.hpp>
#include <vulture/renderer/graphics_api/render_pass.hpp>
#include <vulture/renderer/graphics_api/render_resource_handles.hpp>
#include <vulture/renderer/graphics_api/shader_module.hpp>
#include <vulture/renderer/graphics_api/texture.hpp>

namespace vulture {

constexpr uint32_t kFramesInFlight = 2;

class Window;

/**
 * @brief Abstraction layer over different graphics APIs.
 *
 * Provides functions for creating and managing buffers, textures, framebuffers and drawing. The API is very similar
 * to Vulkan
 * 
 * @note Coordinate systems are the following:
 *       1) Texture/Viewport: (0, 0) - bottom-left; (width, height) - top-right
 *       2) RenderArea: (0, 0) - top-left; (width, height) - bottom-right
 */
class RenderDevice {
public:
  enum class DeviceFamily {kVulkan, /*kOpenGL*/ /*kMetal*/};

  virtual void WaitIdle() = 0;

  virtual uint32_t CurrentFrame() const = 0;

  virtual void FrameBegin() = 0;
  virtual void FrameEnd() = 0;
  
  /************************************************************************************************
   * INIT
   ************************************************************************************************/
  virtual ~RenderDevice() = default;

  static RenderDevice* Create(DeviceFamily family);

  virtual void Init(Window* window, const DeviceFeatures* required_features,
                    const DeviceProperties* required_properties, bool enable_validation) = 0;

  virtual DeviceFeatures GetDeviceFeatures() = 0;
  virtual DeviceProperties GetDeviceProperties() = 0;

  /************************************************************************************************
   * SYNCHRONIZATION
   ************************************************************************************************/
  virtual FenceHandle CreateFence() = 0;
  virtual void DeleteFence(FenceHandle fence) = 0;

  virtual void WaitForFences(uint32_t count, const FenceHandle* fences) = 0;
  virtual void ResetFence(FenceHandle fence) = 0;

  virtual SemaphoreHandle CreateSemaphore() = 0;
  virtual void DeleteSemaphore(SemaphoreHandle semaphore) = 0;

  /************************************************************************************************
   * SWAPCHAIN
   ************************************************************************************************/
  virtual SwapchainHandle CreateSwapchain(TextureUsageFlags usage = kTextureUsageBitColorAttachment) = 0;
  virtual void DeleteSwapchain(SwapchainHandle swapchain) = 0;

  /**
   * @brief Get swapchain textures.
   * 
   * @note Supposed to be called twice - first to retrieve the count and then textures themselves.
   * @note Textures are deleted automatically when the swapchain is deleted.
   * 
   * @param swapchain
   * @param textures_count Can be nullptr.
   * @param textures       Can be nullptr.
   */
  virtual void GetSwapchainTextures(SwapchainHandle swapchain, uint32_t* textures_count, TextureHandle* textures) = 0;

  virtual bool AcquireNextTexture(SwapchainHandle swapchain, uint32_t* texture_idx) = 0;

  /**
   * @brief Present rendered window surface.
   * 
   * @param swapchain
   * @param wait_semaphore
   * 
   * @return true If swapchain is up to date and DOESN'T need recreating.
   */
  virtual bool Present(SwapchainHandle swapchain, SemaphoreHandle wait_semaphore) = 0;

  /**
   * @brief Recreates the swapchain.
   * 
   * @warning One must not delete the old swapchain! It is handled automatically by the function.
   * @note    After recreating the swapchain, one should retrieve swapchain textures once more.
   * 
   * @param swapchain Old swapchain handle.
   * 
   * @return SwapchainHandle New swapchain handle, can be the same as the old swapchain handle.
   */
  virtual SwapchainHandle RecreateSwapchain(SwapchainHandle swapchain) = 0;

  /************************************************************************************************
   * TEXTURE AND SAMPLER
   ************************************************************************************************/
  virtual TextureHandle CreateTexture(const TextureSpecification& specification) = 0;
  virtual void DeleteTexture(TextureHandle texture) = 0;

  virtual const TextureSpecification& GetTextureSpecification(TextureHandle texture) = 0;

  virtual SamplerHandle CreateSampler(const SamplerSpecification& specification) = 0;
  virtual void DeleteSampler(SamplerHandle sampler) = 0;

  virtual const SamplerSpecification& GetSamplerSpecification(SamplerHandle sampler) = 0;

  /************************************************************************************************
   * BUFFER
   ************************************************************************************************/
  /**
   * @brief Create a buffer with the specified size and usage.
   * 
   * @param size           Buffer's size in bytes.
   * @param usage          Bit mask specifying how the buffer can be used.
   * @param dynamic_memory Whether buffer's memory is visible from CPU. Affects performance! Use it with caution!
   * @param map_data       If memory is dynamic, then returns the mapped data pointer.
   *
   * @return BufferHandle
   */
  virtual BufferHandle CreateBuffer(uint32_t size, BufferUsageFlags usage, bool dynamic_memory = false,
                                    void** map_data = nullptr) = 0;

  template <typename T>
  BufferHandle CreateDynamicVertexBuffer(uint32_t count, T** map_data = nullptr) {
    assert(count > 0);
    return CreateBuffer(count * sizeof(T), kBufferUsageBitVertexBuffer, true, reinterpret_cast<void**>(map_data));
  }

  template <typename T>
  BufferHandle CreateStaticVertexBuffer(uint32_t count) {
    assert(count > 0);
    return CreateBuffer(count * sizeof(T), kBufferUsageBitVertexBuffer);
  }

  BufferHandle CreateDynamicIndexBuffer(uint32_t indices_count, uint32_t** map_data = nullptr) {
    assert(indices_count > 0);
    return CreateBuffer(indices_count * sizeof(uint32_t), kBufferUsageBitIndexBuffer, true,
                        reinterpret_cast<void**>(map_data));
  }

  BufferHandle CreateStaticIndexBuffer(uint32_t indices_count) {
    assert(indices_count > 0);
    return CreateBuffer(indices_count * sizeof(uint32_t), kBufferUsageBitIndexBuffer);
  }

  BufferHandle CreateDynamicUniformBuffer(uint32_t size, void** map_data = nullptr) {
    assert(size > 0);
    return CreateBuffer(size, kBufferUsageBitUniformBuffer, true, map_data);
  }

  BufferHandle CreateStaticUniformBuffer(uint32_t size) {
    assert(size > 0);
    return CreateBuffer(size, kBufferUsageBitUniformBuffer);
  }

  template <typename T>
  BufferHandle CreateDynamicUniformBuffer(uint32_t count, T** map_data = nullptr) {
    return CreateDynamicUniformBuffer(count * sizeof(T), reinterpret_cast<void**>(map_data));
  }

  template <typename T>
  BufferHandle CreateStaticUniformBuffer(uint32_t count) {
    return CreateStaticUniformBuffer(count * sizeof(T));
  }

  BufferHandle CreateDynamicStorageBuffer(uint32_t size, void** map_data = nullptr) {
    assert(size > 0);
    return CreateBuffer(size, kBufferUsageBitStorageBuffer, true, map_data);
  }

  BufferHandle CreateStaticStorageBuffer(uint32_t size) {
    assert(size > 0);
    return CreateBuffer(size, kBufferUsageBitStorageBuffer);
  }

  template <typename T>
  BufferHandle CreateDynamicStorageBuffer(uint32_t count, T** map_data = nullptr) {
    return CreateDynamicStorageBuffer(count * sizeof(T), reinterpret_cast<void**>(map_data));
  }

  template <typename T>
  BufferHandle CreateStaticStorageBuffer(uint32_t count) {
    return CreateStaticStorageBuffer(count * sizeof(T));
  }

  virtual void DeleteBuffer(BufferHandle buffer) = 0;

  /**
   * @brief Update region of the buffer's memory.
   * 
   * @param buffer 
   * @param offset 
   * @param size 
   * @param data 
   */
  virtual void LoadBufferData(BufferHandle buffer, uint32_t offset, uint32_t size, const void* data) = 0;

  template <typename T>
  void LoadBufferData(BufferHandle buffer, uint32_t offset_idx, uint32_t count, const T* data) {
    LoadBufferData(buffer, offset_idx * sizeof(T), count * sizeof(T), reinterpret_cast<const void*>(data));
  }

  /**
   * @brief Invalidate dynamic buffer's memory.
   * @note General usage of dynamic buffers is
   *       1. InvalidateBufferMemory()
   *       2. Fill/Update the map_data
   *       3. FlushBufferMemory()
   * 
   * @param buffer
   * @param offset
   * @param size
   */
  virtual void InvalidateBufferMemory(BufferHandle buffer, uint32_t offset, uint32_t size) = 0;

  template <typename T>
  void InvalidateBufferMemory(BufferHandle buffer, uint32_t offset_idx = 0, uint32_t count = 1) {
    InvalidateBufferMemory(buffer, offset_idx * sizeof(T), count * sizeof(T));
  }

  /**
   * @brief Flush dynamic buffer's memory.
   * @note General usage of dynamic buffers is
   *       1. InvalidateBufferMemory()
   *       2. Fill/Update the map_data
   *       3. FlushBufferMemory()
   * 
   * @param buffer
   * @param offset
   * @param size
   */
  virtual void FlushBufferMemory(BufferHandle buffer, uint32_t offset, uint32_t size) = 0;

  template <typename T>
  void FlushBufferMemory(BufferHandle buffer, uint32_t offset_idx = 0, uint32_t count = 1) {
    FlushBufferMemory(buffer, offset_idx * sizeof(T), count * sizeof(T));
  }

  /************************************************************************************************
   * DESCRIPTOR SET
   ************************************************************************************************/
  virtual DescriptorSetLayoutHandle CreateDescriptorSetLayout(const DescriptorSetLayoutInfo& layout_info) = 0;
  virtual void DeleteDescriptorSetLayout(DescriptorSetLayoutHandle layout) = 0;

  virtual DescriptorSetHandle CreateDescriptorSet(DescriptorSetLayoutHandle layout) = 0;
  virtual void DeleteDescriptorSet(DescriptorSetHandle descriptor_set) = 0;

  virtual void WriteDescriptorUniformBuffer(DescriptorSetHandle descriptor_set, uint32_t binding_idx,
                                            BufferHandle uniform_buffer, uint32_t offset, uint32_t size) = 0;
  virtual void WriteDescriptorStorageBuffer(DescriptorSetHandle descriptor_set, uint32_t binding_idx,
                                            BufferHandle storage_buffer, uint32_t offset, uint32_t size) = 0;

  virtual void WriteDescriptorInputAttachment(DescriptorSetHandle descriptor_set, uint32_t binding_idx,
                                              TextureHandle texture) = 0;
  virtual void WriteDescriptorSampler(DescriptorSetHandle descriptor_set, uint32_t binding_idx, TextureHandle texture,
                                      SamplerHandle sampler) = 0;

  /************************************************************************************************
   * RENDER PASS
   ************************************************************************************************/
  virtual RenderPassHandle CreateRenderPass(const RenderPassDescription& render_pass_description) = 0;
  virtual void DeleteRenderPass(RenderPassHandle render_pass) = 0;

  virtual FramebufferHandle CreateFramebuffer(const std::vector<TextureHandle>& attachments,
                                              RenderPassHandle compatible_render_pass) = 0;
  virtual void DeleteFramebuffer(FramebufferHandle framebuffer) = 0;

  /************************************************************************************************
   * PIPELINE
   ************************************************************************************************/
  virtual ShaderModuleHandle CreateShaderModule(ShaderModuleType type, uint32_t binary_size,
                                                const uint32_t* binary) = 0;
  virtual void DeleteShaderModule(ShaderModuleHandle shader_module) = 0;

  virtual PipelineHandle CreatePipeline(const PipelineDescription& description,
                                        RenderPassHandle compatible_render_pass,
                                        uint32_t subpass_idx) = 0;
  virtual void DeletePipeline(PipelineHandle pipeline) = 0;

  /************************************************************************************************
   * COMMAND BUFFER
   ************************************************************************************************/
  virtual CommandBuffer* CreateCommandBuffer(CommandBufferType type, bool temporary = false) = 0;
  virtual void DeleteCommandBuffer(CommandBuffer* command_buffer) = 0;
 
 protected:
  RenderDevice() = default;
};

}  // namespace vulture