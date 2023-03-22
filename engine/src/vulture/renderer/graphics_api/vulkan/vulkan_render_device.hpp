/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file vulkan_render_device.hpp
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
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnullability-extension"
#include <vk_mem_alloc.h>
#pragma GCC diagnostic pop

#include <map>

#include <vulture/platform/window.hpp>
#include <vulture/renderer/graphics_api/render_device.hpp>

namespace vulture {

struct VulkanTexture {
  VulkanTexture() = default;
  VulkanTexture(const TextureSpecification& specification) : specification(specification) {}

  VkImage              vk_image{VK_NULL_HANDLE};
  VkImageView          vk_image_view{VK_NULL_HANDLE};
  VmaAllocation        vma_allocation{nullptr};
  TextureSpecification specification{};
};

struct VulkanSampler {
  VulkanSampler() = default;
  VulkanSampler(const SamplerSpecification& specification) : specification(specification) {}

  VkSampler            vk_sampler{VK_NULL_HANDLE};
  SamplerSpecification specification{};
};

struct VulkanBuffer {
  VkBuffer      vk_buffer{VK_NULL_HANDLE};
  VmaAllocation vma_allocation{VK_NULL_HANDLE};
  bool          dynamic_memory{false};
  void*         map_data{nullptr};
};

struct VulkanDescriptorSetLayout {
  VulkanDescriptorSetLayout() = default;
  VulkanDescriptorSetLayout(const DescriptorSetLayoutInfo& layout_info) : layout_info(layout_info) {}

  DescriptorSetLayoutInfo layout_info{};
  VkDescriptorSetLayout   vk_layout{VK_NULL_HANDLE};
};

struct VulkanDescriptorSet {
  VulkanDescriptorSet() = default;

  DescriptorSetLayoutHandle layout_handle{kInvalidRenderResourceHandle};  // Not owned by the set
  VkDescriptorPool          vk_pool{VK_NULL_HANDLE};  // FIXME: make a pool of VkDescriptorPools to reuse
  VkDescriptorSet           vk_set{VK_NULL_HANDLE};  // Automaticaly freed when destroying descriptor pool
};

struct VulkanRenderPass {
  VulkanRenderPass() = default;
  VulkanRenderPass(const RenderPassDescription& description) : description(description) {}

  RenderPassDescription description{};
  VkRenderPass          vk_render_pass{VK_NULL_HANDLE};
};

struct VulkanFramebuffer {
  VulkanFramebuffer() = default;
  VkFramebuffer vk_framebuffer{VK_NULL_HANDLE};
};

struct VulkanShaderModule {
  VulkanShaderModule() = default;
  VulkanShaderModule(ShaderModuleType type) : type(type) {}

  ShaderModuleType type{ShaderModuleType::kInvalid};
  VkShaderModule   vk_module{VK_NULL_HANDLE};
};

struct VulkanPipeline {
  VulkanPipeline() = default;
  VulkanPipeline(const PipelineDescription& description) : description(description) {}

  PipelineDescription description{};
  VkPipeline          vk_pipeline{VK_NULL_HANDLE};
  VkPipelineLayout    vk_pipeline_layout{VK_NULL_HANDLE};
};

struct VulkanSwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR        capabilities;  // TODO: unused at the moment
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR>   present_modes;
};

struct VulkanSwapchain {
  VulkanSwapchain() = default;

  VkSwapchainKHR             vk_swapchain{VK_NULL_HANDLE};
  VkFormat                   vk_format{};
  VkExtent2D                 vk_extent{};
  TextureUsageFlags          usage{0};
  std::vector<TextureHandle> textures;
};

class VulkanRenderDevice final : public RenderDevice {
 public:
  struct QueueFamilyIndices {
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;
  };

 public:
  void WaitIdle() override;

  /************************************************************************************************
   * INIT
   ************************************************************************************************/
  VulkanRenderDevice();
  ~VulkanRenderDevice() override;

  void Init(Window* window, const DeviceFeatures* required_features, const DeviceProperties* required_properties,
            bool enable_validation) override;

  DeviceFeatures GetDeviceFeatures() override;
  DeviceProperties GetDeviceProperties() override;

  /************************************************************************************************
   * SWAPCHAIN
   ************************************************************************************************/
  SwapchainHandle CreateSwapchain(TextureUsageFlags usage) override;
  void DeleteSwapchain(SwapchainHandle swapchain) override;

  void GetSwapchainTextures(SwapchainHandle swapchain, uint32_t* textures_count, TextureHandle* textures) override;

  bool FrameBegin(SwapchainHandle swapchain, uint32_t* texture_idx) override;
  void FrameEnd(SwapchainHandle swapchain) override;

  bool Present(SwapchainHandle swapchain) override;

  SwapchainHandle RecreateSwapchain(SwapchainHandle swapchain) override;

  /************************************************************************************************
   * TEXTURE AND SAMPLER
   ************************************************************************************************/
  TextureHandle CreateTexture(const TextureSpecification& specification) override;
  void DeleteTexture(TextureHandle texture) override;

  const TextureSpecification& GetTextureSpecification(TextureHandle texture) override;

  SamplerHandle CreateSampler(const SamplerSpecification& specification) override;
  void DeleteSampler(SamplerHandle sampler) override;

  const SamplerSpecification& GetSamplerSpecification(SamplerHandle sampler) override;

  /************************************************************************************************
   * BUFFER
   ************************************************************************************************/
  BufferHandle CreateBuffer(uint32_t size, BufferUsageFlags usage, bool dynamic_memory, void** map_data) override;
  void DeleteBuffer(BufferHandle buffer) override;

  void LoadBufferData(BufferHandle buffer, uint32_t offset, uint32_t size, const void* data) override;

  void InvalidateBufferMemory(BufferHandle buffer, uint32_t offset, uint32_t size) override;
  void FlushBufferMemory(BufferHandle buffer, uint32_t offset, uint32_t size) override;

  /************************************************************************************************
   * DESCRIPTOR SET
   ************************************************************************************************/
  DescriptorSetLayoutHandle CreateDescriptorSetLayout(const DescriptorSetLayoutInfo& layout_info) override;
  void DeleteDescriptorSetLayout(DescriptorSetLayoutHandle layout) override;

  DescriptorSetHandle CreateDescriptorSet(DescriptorSetLayoutHandle layout) override;
  void DeleteDescriptorSet(DescriptorSetHandle descriptor_set) override;

  void WriteDescriptorUniformBuffer(DescriptorSetHandle descriptor_set, uint32_t binding, BufferHandle uniform_buffer,
                                    uint32_t offset, uint32_t size) override;
  void WriteDescriptorStorageBuffer(DescriptorSetHandle descriptor_set, uint32_t binding, BufferHandle storage_buffer,
                                    uint32_t offset, uint32_t size) override;

  void WriteDescriptorInputAttachment(DescriptorSetHandle descriptor_set, uint32_t binding_idx,
                                      TextureHandle texture) override;
  void WriteDescriptorSampler(DescriptorSetHandle descriptor_set, uint32_t binding_idx, TextureHandle texture,
                              SamplerHandle sampler) override;

  /************************************************************************************************
   * RENDER PASS
   ************************************************************************************************/
  RenderPassHandle CreateRenderPass(const RenderPassDescription& render_pass_description) override;
  void DeleteRenderPass(RenderPassHandle render_pass) override;

  FramebufferHandle CreateFramebuffer(const std::vector<TextureHandle>& attachments,
                                      RenderPassHandle compatible_render_pass) override;
  void DeleteFramebuffer(FramebufferHandle framebuffer) override;

  /************************************************************************************************
   * PIPELINE
   ************************************************************************************************/
  ShaderModuleHandle CreateShaderModule(ShaderModuleType type, uint32_t binary_size, const uint32_t* binary) override;
  void DeleteShaderModule(ShaderModuleHandle shader_module) override;

  PipelineHandle CreatePipeline(const PipelineDescription& description, RenderPassHandle compatible_render_pass,
                                uint32_t subpass_idx) override;
  void DeletePipeline(PipelineHandle pipeline) override;

  /************************************************************************************************
   * COMMAND BUFFER
   ************************************************************************************************/
  CommandBuffer* CreateCommandBuffer(CommandBufferType type, bool temporary) override;
  void DeleteCommandBuffer(CommandBuffer* command_buffer) override;

 private:
  uint32_t GenNextHandle();

  VulkanTexture&             GetVulkanTexture(TextureHandle);
  VulkanSampler&             GetVulkanSampler(SamplerHandle);
  VulkanBuffer&              GetVulkanBuffer(BufferHandle);
  VulkanDescriptorSetLayout& GetVulkanDescriptorSetLayout(DescriptorSetLayoutHandle);
  VulkanDescriptorSet&       GetVulkanDescriptorSet(DescriptorSetHandle);
  VulkanRenderPass&          GetVulkanRenderPass(RenderPassHandle);
  VulkanFramebuffer&         GetVulkanFramebuffer(FramebufferHandle);
  VulkanShaderModule&        GetVulkanShaderModule(ShaderModuleHandle);
  VulkanPipeline&            GetVulkanPipeline(PipelineHandle);
  VulkanSwapchain&           GetVulkanSwapchain(SwapchainHandle);

  VulkanBuffer CreateStagingBuffer(VkDeviceSize size);

  uint32_t FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties);
  void CopyBuffer(VkCommandBuffer command_buffer, VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size,
                  VkDeviceSize src_offset = 0, VkDeviceSize dst_offset = 0);

  VkCommandPool CreateCommandPool(VkCommandPoolCreateFlags flags);
  VkCommandBuffer CreateCommandBuffer(VkCommandPool command_pool);

  VkCommandBuffer BeginSingleTimeCommands();
  void EndSingleTimeCommands(VkCommandBuffer command_buffer);

  /************************************************************************************************
   * INIT
   ************************************************************************************************/
  void CreateInstance(bool enable_validation);
  bool CheckValidationLayersSupport();

  void CreateWindowSurface();

  void PickPhysicalDevice(const DeviceFeatures* required_features, const DeviceProperties* required_properties);
  bool IsPhysicalDeviceSuitable(VkPhysicalDevice physical_device, const DeviceFeatures* required_features,
                                const DeviceProperties* required_properties);
  DeviceFeatures GetDeviceFeatures(VkPhysicalDevice physical_device);
  DeviceProperties GetDeviceProperties(VkPhysicalDevice physical_device);
  QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physical_device);
  VulkanSwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice physical_device);

  void CreateLogicalDevice();

  /************************************************************************************************
   * TEXTURE AND SAMPLER
   ************************************************************************************************/
  void CreateImage(uint32_t width, uint32_t height, uint32_t mip_levels, VkSampleCountFlagBits samples, VkFormat format,
                   VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image,
                   VkDeviceMemory* image_memory);

  VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, uint32_t mip_levels);

  /************************************************************************************************
   * SWAP CHAIN
   ************************************************************************************************/
  VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const VulkanSwapChainSupportDetails& support_details);
  VkPresentModeKHR ChooseSwapPresentMode(const VulkanSwapChainSupportDetails& support_details);

 private:
  VkInstance instance_{VK_NULL_HANDLE};
  VkPhysicalDevice physical_device_{VK_NULL_HANDLE};
  QueueFamilyIndices queue_family_indices_{};

  Window* window_{nullptr};
  VkSurfaceKHR window_surface_{VK_NULL_HANDLE};

  VkDevice device_{VK_NULL_HANDLE};

  VmaAllocator allocator_{};

  /** @note Destroyed implicitly when the device is destroyed */
  VkQueue graphics_queue_{VK_NULL_HANDLE};
  /** @note Destroyed implicitly when the device is destroyed */
  VkQueue present_queue_{VK_NULL_HANDLE};

  VkCommandPool transient_command_pool_{VK_NULL_HANDLE};
  VkCommandPool main_command_pool_{VK_NULL_HANDLE};

  bool frame_began_{false};
  uint32_t current_swapchain_texture_idx_{0};
  VkFence fence_swapchain_image_available_{VK_NULL_HANDLE};  // FIXME: Don't wait on image acquiring

  uint32_t next_handle_{1};
  std::map<SwapchainHandle, VulkanSwapchain>                     swapchains_;
  std::map<TextureHandle, VulkanTexture>                         textures_;
  std::map<SamplerHandle, VulkanSampler>                         samplers_;
  std::map<BufferHandle, VulkanBuffer>                           buffers_;
  std::map<DescriptorSetLayoutHandle, VulkanDescriptorSetLayout> descriptor_set_layouts_;
  std::map<DescriptorSetHandle, VulkanDescriptorSet>             descriptor_sets_;
  std::map<RenderPassHandle, VulkanRenderPass>                   render_passes_;
  std::map<FramebufferHandle, VulkanFramebuffer>                 framebuffers_;
  std::map<ShaderModuleHandle, VulkanShaderModule>               shader_modules_;
  std::map<PipelineHandle, VulkanPipeline>                       pipelines_;

  friend class VulkanCommandBuffer;
};

}  // namespace vulture
