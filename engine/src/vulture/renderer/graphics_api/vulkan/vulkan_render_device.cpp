/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file vulkan_render_device.cpp
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

#define VMA_IMPLEMENTATION
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnullability-extension"
#pragma GCC diagnostic ignored "-Wnullability-completeness"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#include <vk_mem_alloc.h>
#pragma GCC diagnostic pop
#undef VMA_IMPLEMENTATION

#include <array>
#include <iostream>  // FIXME: Get rid of
#include <set>

using namespace vulture;

static constexpr const char* kValidationLayers[] = {"VK_LAYER_KHRONOS_validation"};

static const std::vector<const char*> kRequiredDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                                                                   VK_KHR_MAINTENANCE1_EXTENSION_NAME};

uint32_t VulkanRenderDevice::GenNextHandle() {
  return next_handle_++;
}

VulkanBuffer VulkanRenderDevice::CreateStagingBuffer(VkDeviceSize size) {
  VulkanBuffer staging_buffer{};

  VkBufferCreateInfo staging_buffer_info = {};
  staging_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  staging_buffer_info.size  = size;
  staging_buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  
  VmaAllocationCreateInfo staging_alloc_info = {};
  staging_alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
  staging_alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

  VkResult result = vmaCreateBuffer(allocator_, &staging_buffer_info, &staging_alloc_info, &staging_buffer.vk_buffer,
                                    &staging_buffer.vma_allocation, nullptr);
  assert(result == VK_SUCCESS);

  return staging_buffer;
}

uint32_t VulkanRenderDevice::FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties mem_properties{};
  vkGetPhysicalDeviceMemoryProperties(physical_device_, &mem_properties);

  for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
    if ((type_filter & (1 << i)) &&
        (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }

  assert(!"Couldn't find suitable memory type");
  return 0;
}

void VulkanRenderDevice::CopyBuffer(VkCommandBuffer command_buffer, VkBuffer src_buffer, VkBuffer dst_buffer,
                                    VkDeviceSize size, VkDeviceSize src_offset, VkDeviceSize dst_offset) {
  VkBufferCopy copy_region{};
  copy_region.srcOffset = src_offset;
  copy_region.dstOffset = dst_offset;
  copy_region.size      = size;
  vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, /*regionCount=*/1, &copy_region);
}

VkCommandPool VulkanRenderDevice::CreateCommandPool(VkCommandPoolCreateFlags flags) {
  VkCommandPoolCreateInfo pool_create_info{};
  pool_create_info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_create_info.flags            = flags;
  pool_create_info.queueFamilyIndex = queue_family_indices_.graphics_family.value();

  VkCommandPool command_pool{VK_NULL_HANDLE};
  VkResult result = vkCreateCommandPool(device_, &pool_create_info, /*allocator=*/nullptr, &command_pool);
  assert(result == VK_SUCCESS);

  return command_pool;
}

VkCommandBuffer VulkanRenderDevice::CreateCommandBuffer(VkCommandPool command_pool) {
  VkCommandBuffer command_buffer{VK_NULL_HANDLE};

  VkCommandBufferAllocateInfo command_buffer_alloc_info{};
  command_buffer_alloc_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  command_buffer_alloc_info.commandPool        = command_pool;
  command_buffer_alloc_info.commandBufferCount = 1;
  // Can be submitted to a queue for execution, can't be called from other command buffers
  command_buffer_alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  VkResult result = vkAllocateCommandBuffers(device_, &command_buffer_alloc_info, &command_buffer);
  assert(result == VK_SUCCESS);

  return command_buffer;
}

VkCommandBuffer VulkanRenderDevice::BeginSingleTimeCommands() {
  // Better to create a separate command pool for short-lived buffers and use VK_COMMAND_POOL_CREATE_TRANSIENT_BIT
  VkCommandBufferAllocateInfo alloc_info{};
  alloc_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandPool        = transient_command_pool_;
  alloc_info.commandBufferCount = 1;

  VkCommandBuffer command_buffer{VK_NULL_HANDLE};
  VkResult result = vkAllocateCommandBuffers(device_, &alloc_info, &command_buffer);
  assert(result == VK_SUCCESS);

  VkCommandBufferBeginInfo begin_info{};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(command_buffer, &begin_info);

  return command_buffer;
}

void VulkanRenderDevice::EndSingleTimeCommands(VkCommandBuffer command_buffer) {
  vkEndCommandBuffer(command_buffer);

  VkSubmitInfo submit_info{};
  submit_info.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers    = &command_buffer;

  // Could use a fence and wait with vkWaitForFences to allow multiple transfers simultaneously
  vkQueueSubmit(graphics_queue_, /*submitCount=*/1, &submit_info, /*fence=*/VK_NULL_HANDLE);
  vkQueueWaitIdle(graphics_queue_);

  vkFreeCommandBuffers(device_, transient_command_pool_, /*commandBufferCount*/1, &command_buffer);
}

void VulkanRenderDevice::WaitIdle() {
  vkDeviceWaitIdle(device_);
}

/************************************************************************************************
 * INIT
 ************************************************************************************************/
VulkanRenderDevice::VulkanRenderDevice() {
    
}

VulkanRenderDevice::~VulkanRenderDevice() {
  vkDestroyFence(device_, fence_swapchain_image_available_, /*allocator=*/nullptr);

  vkDestroyCommandPool(device_, transient_command_pool_, /*allocator=*/nullptr);
  vkDestroyCommandPool(device_, main_command_pool_, /*allocator=*/nullptr);

  vmaDestroyAllocator(allocator_);

  vkDestroyDevice(device_, /*allocator=*/nullptr);

  vkDestroySurfaceKHR(instance_, window_surface_, /*allocator=*/nullptr);
  vkDestroyInstance(instance_, /*allocator=*/nullptr);
}

void VulkanRenderDevice::Init(Window* window, const DeviceFeatures* required_features,
                              const DeviceProperties* required_properties, bool enable_validation) {
  window_ = window;

  CreateInstance(enable_validation);
  CreateWindowSurface();
  PickPhysicalDevice(required_features, required_properties);
  CreateLogicalDevice();

  VmaAllocatorCreateInfo allocator_info = {};
  allocator_info.physicalDevice = physical_device_;
  allocator_info.device         = device_;
  allocator_info.instance       = instance_;
  VkResult result = vmaCreateAllocator(&allocator_info, &allocator_);
  assert(result == VK_SUCCESS);

  transient_command_pool_ = CreateCommandPool(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
  main_command_pool_      = CreateCommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
  
  VkFenceCreateInfo fence_info = {};
  fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  result = vkCreateFence(device_, &fence_info, /*allocator=*/nullptr, &fence_swapchain_image_available_);
  assert(result == VK_SUCCESS);
}

DeviceFeatures VulkanRenderDevice::GetDeviceFeatures() {
  return GetDeviceFeatures(physical_device_);
}

DeviceProperties VulkanRenderDevice::GetDeviceProperties() {
  return GetDeviceProperties(physical_device_);
}

void VulkanRenderDevice::CreateInstance(bool enable_validation) {
  bool validation_layers_supported = true;
  if (enable_validation) {
    validation_layers_supported = CheckValidationLayersSupport();
  }

  VkApplicationInfo app_info{};
  app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName   = "Vulkan app";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName        = "No Engine";
  app_info.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion         = VK_API_VERSION_1_0;

  uint32_t glfw_extension_count = 0;
  const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

  std::vector<const char*> extensions{};
  for (uint32_t i = 0; i < glfw_extension_count; ++i) {
    extensions.push_back(glfw_extensions[i]);
  }

#ifdef __APPLE__
  extensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
  extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
  extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#endif

  VkInstanceCreateInfo create_info{};
  create_info.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo        = &app_info;
  create_info.enabledExtensionCount   = extensions.size();
  create_info.ppEnabledExtensionNames = extensions.data();
  create_info.enabledLayerCount       = 0;

#ifdef __APPLE__
  create_info.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

  if (enable_validation && validation_layers_supported) {
    create_info.enabledLayerCount = static_cast<uint32_t>(sizeof(kValidationLayers) / sizeof(kValidationLayers[0]));
    create_info.ppEnabledLayerNames = kValidationLayers;
  } else {
    create_info.enabledLayerCount = 0;
  }

  VkResult result = vkCreateInstance(&create_info, /*allocator=*/nullptr, &instance_);
  assert(result == VK_SUCCESS);  // TODO: Handle errors
}

bool VulkanRenderDevice::CheckValidationLayersSupport() {
  uint32_t layer_count = 0;
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

  std::vector<VkLayerProperties> available_layers(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

  for (const char* layer_name : kValidationLayers) {
    bool layer_found = false;

    for (const auto& layer_properties : available_layers) {
      if (strcmp(layer_name, layer_properties.layerName) == 0) {
        layer_found = true;
        break;
      }
    }

    if (!layer_found) {
      std::cerr << "Validation layer \"" << layer_name << "\" is not supported!\n";
      return false;
    }
  }

  return true;
}

void VulkanRenderDevice::CreateWindowSurface() {
  VkResult result =
      glfwCreateWindowSurface(instance_, window_->GetNativeWindow(), /*allocator=*/nullptr, &window_surface_);
  assert(result == VK_SUCCESS);
}

void VulkanRenderDevice::PickPhysicalDevice(const DeviceFeatures* required_features,
                                            const DeviceProperties* required_properties) {
  uint32_t device_count = 0;
  vkEnumeratePhysicalDevices(instance_, &device_count, nullptr);
  assert(device_count != 0);

  std::vector<VkPhysicalDevice> physical_devices(device_count);
  vkEnumeratePhysicalDevices(instance_, &device_count, physical_devices.data());

  for (const auto& physical_device : physical_devices) {
    if (IsPhysicalDeviceSuitable(physical_device, required_features, required_properties)) {
      physical_device_ = physical_device;
      break;
    }
  }

  assert(physical_device_ != VK_NULL_HANDLE);
}

bool VulkanRenderDevice::IsPhysicalDeviceSuitable(VkPhysicalDevice physical_device,
                                                  const DeviceFeatures* required_features,
                                                  const DeviceProperties* required_properties) {
  /* Check required features */
  if (required_features != nullptr) {
    DeviceFeatures features = GetDeviceFeatures(physical_device);

    if (required_features->sampler_anisotropy && !features.sampler_anisotropy) {
      return false;
    }
  }

  /* Check required properties */
  if (required_properties != nullptr) {
    DeviceProperties properties = GetDeviceProperties(physical_device);

    if (properties.max_msaa_samples < required_properties->max_msaa_samples) {
      return false;
    }

    if (properties.max_sampler_anisotropy < required_properties->max_sampler_anisotropy) {
      return false;
    }
  }

  uint32_t extensions_count = 0;
  vkEnumerateDeviceExtensionProperties(physical_device, /*pLayerName=*/nullptr, &extensions_count, nullptr);

  std::vector<VkExtensionProperties> available_extensions{extensions_count};
  vkEnumerateDeviceExtensionProperties(physical_device, /*pLayerName=*/nullptr, &extensions_count,
                                        available_extensions.data());

  for (const auto& required_extension : kRequiredDeviceExtensions) {
    bool found_extension = false;

    for (const auto& available_extension : available_extensions) {
      if (std::strcmp(required_extension, available_extension.extensionName) == 0) {
        found_extension = true;
        break;
      }
    }

    if (!found_extension) {
      return false;
    }
  }

  VulkanSwapChainSupportDetails swap_chain_support = QuerySwapChainSupport(physical_device);
  bool swap_chain_ok = !swap_chain_support.formats.empty() && !swap_chain_support.present_modes.empty();

  QueueFamilyIndices indices = FindQueueFamilies(physical_device);
  bool queue_families_ok = indices.graphics_family.has_value() && indices.present_family.has_value();

  return swap_chain_ok && queue_families_ok;
}

DeviceFeatures VulkanRenderDevice::GetDeviceFeatures(VkPhysicalDevice physical_device) {
  VkPhysicalDeviceFeatures device_features;
  vkGetPhysicalDeviceFeatures(physical_device, &device_features);

  DeviceFeatures features{};
  features.sampler_anisotropy = device_features.samplerAnisotropy;
  // TODO:

  return features;
}

DeviceProperties VulkanRenderDevice::GetDeviceProperties(VkPhysicalDevice physical_device) {
  VkPhysicalDeviceProperties device_properties{};
  vkGetPhysicalDeviceProperties(physical_device, &device_properties);

  DeviceProperties properties{};
  properties.max_msaa_samples       = GetMaxMSAASamples(device_properties);
  properties.max_sampler_anisotropy = device_properties.limits.maxSamplerAnisotropy;
  // TODO:

  return properties;
}

VulkanRenderDevice::QueueFamilyIndices VulkanRenderDevice::FindQueueFamilies(VkPhysicalDevice physical_device) {
  QueueFamilyIndices indices;

  uint32_t queue_family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

  std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

  int i = 0;
  for (const auto& queue_family : queue_families) {
    if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphics_family = i;
    }
    
    VkBool32 present_support = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, window_surface_, &present_support);
    if (present_support) {
      indices.present_family = i;
    }

    if (indices.graphics_family.has_value() && indices.present_family.has_value()) {
      break;
    }

    i++;
  }

  return indices;
}

VulkanSwapChainSupportDetails VulkanRenderDevice::QuerySwapChainSupport(VkPhysicalDevice physical_device) {
  VulkanSwapChainSupportDetails details;

  /* Surface capabilities */
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, window_surface_, &details.capabilities);

  /* Surface formats */
  uint32_t format_count = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, window_surface_, &format_count, nullptr);

  if (format_count != 0) {
    details.formats.resize(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, window_surface_, &format_count, details.formats.data());
  }

  /* Present modes */
  uint32_t present_mode_count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, window_surface_, &present_mode_count, nullptr);

  if (present_mode_count != 0) {
      details.present_modes.resize(present_mode_count);
      vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, window_surface_, &present_mode_count,
                                                details.present_modes.data());
  }

  return details;
}

void VulkanRenderDevice::CreateLogicalDevice() {
  QueueFamilyIndices indices = FindQueueFamilies(physical_device_);

  std::vector<VkDeviceQueueCreateInfo> queue_create_infos{};

  std::set<uint32_t> unique_queue_families; // Set is used in case some queue families support several operations
  unique_queue_families.insert(indices.graphics_family.value());
  unique_queue_families.insert(indices.present_family.value());

  // Even though we are creating only one device queue, we still need to specify the priority
  float queue_priority = 1.0f;

  for (uint32_t queue_family : unique_queue_families) {
    VkDeviceQueueCreateInfo queue_create_info{};
    queue_create_info.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = queue_family;
    queue_create_info.queueCount       = 1;
    queue_create_info.pQueuePriorities = &queue_priority; 
    queue_create_infos.push_back(queue_create_info);
  }

  VkPhysicalDeviceFeatures device_features{};
  device_features.samplerAnisotropy = VK_TRUE;

  std::vector<const char*> extensions = kRequiredDeviceExtensions;

#ifdef __APPLE__
  extensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

  std::cout << "Device extensions:\n";
  for (uint32_t i = 0; i < extensions.size(); ++i) {
    std::cout << extensions[i] << std::endl;
  }

  VkDeviceCreateInfo create_info{};
  create_info.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  create_info.pQueueCreateInfos       = queue_create_infos.data();
  create_info.queueCreateInfoCount    = static_cast<uint32_t>(queue_create_infos.size());
  create_info.pEnabledFeatures        = &device_features;
  create_info.enabledExtensionCount   = extensions.size();
  create_info.ppEnabledExtensionNames = extensions.data();

  VkResult result = vkCreateDevice(physical_device_, &create_info, nullptr, &device_);
  assert(result == VK_SUCCESS);

  queue_family_indices_ = indices;
  vkGetDeviceQueue(device_, indices.graphics_family.value(), /*queueIndex=*/0, &graphics_queue_);
  vkGetDeviceQueue(device_, indices.present_family.value(), /*queueIndex=*/0, &present_queue_);
}

/************************************************************************************************
 * SWAPCHAIN
 ************************************************************************************************/
SwapchainHandle VulkanRenderDevice::CreateSwapchain(TextureUsageFlags usage) {
  VulkanSwapChainSupportDetails swap_chain_support = QuerySwapChainSupport(physical_device_);

  VkSurfaceFormatKHR surface_format  = ChooseSwapSurfaceFormat(swap_chain_support);
  VkPresentModeKHR   present_mode    = ChooseSwapPresentMode(swap_chain_support);
  VkExtent2D         extent          = VkExtent2D{window_->GetFramebufferWidth(), window_->GetFramebufferHeight()};
  uint32_t           min_image_count = swap_chain_support.capabilities.minImageCount + 1;  // FIXME: clamp!

  VkSwapchainCreateInfoKHR create_info{};
  create_info.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  create_info.surface          = window_surface_;
  create_info.minImageCount    = min_image_count;
  create_info.imageFormat      = surface_format.format;
  create_info.imageColorSpace  = surface_format.colorSpace;
  create_info.imageExtent      = extent;
  create_info.imageArrayLayers = 1;
  create_info.imageUsage       = usage | kTextureUsageBitColorAttachment;  // FIXME: Color attachment usage not needed
  create_info.preTransform     = swap_chain_support.capabilities.currentTransform;
  create_info.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  create_info.presentMode      = present_mode;
  create_info.clipped          = VK_TRUE;         // Pixels that are obscured by another window are clipped
  create_info.oldSwapchain     = VK_NULL_HANDLE;  // Used when recreating the swap chain (e.g. on window resize)

  if (queue_family_indices_.graphics_family.value() == queue_family_indices_.present_family.value()) {
    create_info.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
    create_info.queueFamilyIndexCount = 0;        // Optional
    create_info.pQueueFamilyIndices   = nullptr;  // Optional
  } else {
    uint32_t indices_array[] = {queue_family_indices_.graphics_family.value(),
                                queue_family_indices_.present_family.value()};

    create_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
    create_info.queueFamilyIndexCount = 2;
    create_info.pQueueFamilyIndices   = indices_array;
  }

  VkSwapchainKHR vk_swapchain{VK_NULL_HANDLE};

  VkResult result = vkCreateSwapchainKHR(device_, &create_info, /*allocator=*/nullptr, &vk_swapchain);
  assert(result == VK_SUCCESS);

  VulkanSwapchain swapchain{};
  swapchain.vk_swapchain = vk_swapchain;
  swapchain.vk_format    = surface_format.format;
  swapchain.vk_extent    = extent;
  swapchain.usage        = usage;

  /* Retrieving the images */
  uint32_t image_count = 0;
  static VkImage vk_images[32];

  vkGetSwapchainImagesKHR(device_, vk_swapchain, &image_count, nullptr);
  assert(image_count > 0);
  
  vkGetSwapchainImagesKHR(device_, vk_swapchain, &image_count, vk_images);

  /* Creating image views */
  static VkImageView vk_image_views[32];

  for (uint32_t i = 0; i < image_count; ++i) {
    vk_image_views[i] = CreateImageView(vk_images[i], surface_format.format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
  }

  /* Creating textures */
  swapchain.textures.resize(image_count);
  for (uint32_t i = 0; i < image_count; ++i) {
    VulkanTexture vulkan_texture{};
    vulkan_texture.specification.format       = GetDataFormatFromVk(surface_format.format);
    vulkan_texture.specification.type         = TextureType::kTexture2D;
    vulkan_texture.specification.usage        = usage;
    vulkan_texture.specification.cpu_readable = false;
    vulkan_texture.specification.width        = extent.width;
    vulkan_texture.specification.height       = extent.height;
    vulkan_texture.specification.mip_levels   = 1;
    vulkan_texture.specification.samples      = 1;

    vulkan_texture.vk_image       = vk_images[i];
    vulkan_texture.vk_image_view  = vk_image_views[i];
    vulkan_texture.vma_allocation = VK_NULL_HANDLE;  // Not accessible!

    TextureHandle texture_handle = GenNextHandle();
    textures_.emplace(texture_handle, std::move(vulkan_texture));

    swapchain.textures[i] = texture_handle;
  }

  /* Adding VulkanSwapchain */
  SwapchainHandle handle = GenNextHandle();
  swapchains_.emplace(handle, std::move(swapchain));
  
  return handle;
}

void VulkanRenderDevice::DeleteSwapchain(SwapchainHandle handle) {
  auto it = swapchains_.find(handle);
  assert(it != swapchains_.end());

  VulkanSwapchain& swapchain = it->second;

  /* Deleting textures */
  for (TextureHandle texture_handle : swapchain.textures) {
    auto texture_it = textures_.find(texture_handle);
    assert(texture_it != textures_.end());

    // NOTE: only need to delete the image view, because the image is owned by the vulkan swapchain itself
    VulkanTexture& texture = texture_it->second;
    vkDestroyImageView(device_, texture.vk_image_view, /*allocator=*/nullptr);

    textures_.erase(texture_it);
  }

  /* Deleting swapchain */
  vkDestroySwapchainKHR(device_, swapchain.vk_swapchain, /*allocator=*/nullptr);
  swapchains_.erase(it);
}

void VulkanRenderDevice::GetSwapchainTextures(SwapchainHandle swapchain_handle, uint32_t* textures_count,
                                              TextureHandle* texture_handles) {
  VulkanSwapchain& swapchain = GetVulkanSwapchain(swapchain_handle);

  if (textures_count != nullptr) {
    *textures_count = static_cast<uint32_t>(swapchain.textures.size());
  }  

  if (texture_handles != nullptr) {
    std::memcpy(texture_handles, swapchain.textures.data(), swapchain.textures.size() * sizeof(*texture_handles));
  }
}

bool VulkanRenderDevice::FrameBegin(SwapchainHandle swapchain_handle, uint32_t* texture_idx) {
  assert(!frame_began_);

  VulkanSwapchain& swapchain = GetVulkanSwapchain(swapchain_handle);

  uint32_t tmp_texture_idx = 0;
  VkResult result = vkAcquireNextImageKHR(device_, swapchain.vk_swapchain, /*timeout=*/UINT64_MAX,
                                          /*semaphore=*/VK_NULL_HANDLE,
                                          /*fence=*/fence_swapchain_image_available_, &tmp_texture_idx);
  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    return false;
  }

  assert(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);

  *texture_idx = tmp_texture_idx;
  current_swapchain_texture_idx_ = tmp_texture_idx;

  // FIXME: Don't wait on image acquiring
  vkWaitForFences(device_, 1, &fence_swapchain_image_available_, true, /*timeout=*/UINT64_MAX);
  vkResetFences(device_, 1, &fence_swapchain_image_available_);

  frame_began_ = true;
  return true;
}

void VulkanRenderDevice::FrameEnd(SwapchainHandle) {
  assert(frame_began_);

  vkQueueWaitIdle(graphics_queue_);
  frame_began_ = false;
}

bool VulkanRenderDevice::Present(SwapchainHandle swapchain_handle) {
  assert(!frame_began_);

  VulkanSwapchain& swapchain = GetVulkanSwapchain(swapchain_handle);

  VkPresentInfoKHR present_info{};
  present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present_info.waitSemaphoreCount = 0;
  present_info.pWaitSemaphores    = nullptr;
  present_info.swapchainCount     = 1;
  present_info.pSwapchains        = &swapchain.vk_swapchain;
  present_info.pImageIndices      = &current_swapchain_texture_idx_;
  // Optional array of results, to check per swap chain if presenting was successful 
  present_info.pResults           = nullptr;

  VkResult result = vkQueuePresentKHR(present_queue_, &present_info);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    return false;
  } else {
    assert(result == VK_SUCCESS);
  }

  current_swapchain_texture_idx_ = 0;

  return true;
}

SwapchainHandle VulkanRenderDevice::RecreateSwapchain(SwapchainHandle handle) {
  vkDeviceWaitIdle(device_);

  TextureUsageFlags usage = GetVulkanSwapchain(handle).usage;

  // TODO: Instead use VkSwapchainCreateInfoKHR.oldSwapchain field
  DeleteSwapchain(handle);
  return CreateSwapchain(usage);
}

VkSurfaceFormatKHR VulkanRenderDevice::ChooseSwapSurfaceFormat(const VulkanSwapChainSupportDetails& support_details) {
  for (const auto& format : support_details.formats) {
    if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return format;
    }
  }

  return support_details.formats[0];
}

VkPresentModeKHR VulkanRenderDevice::ChooseSwapPresentMode(const VulkanSwapChainSupportDetails& support_details) {
  for (const auto& mode : support_details.present_modes) {
    if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return mode;
    }
  }

  return VK_PRESENT_MODE_FIFO_KHR;  // Guaranteed to be always available
}

/************************************************************************************************
 * TEXTURE AND SAMPLER
 ************************************************************************************************/
TextureHandle VulkanRenderDevice::CreateTexture(const TextureSpecification& specification) {
  uint32_t width  = specification.width;
  uint32_t height = specification.height;

  /* Create image */
  VkImage           vk_image        = VK_NULL_HANDLE;
  VmaAllocation     vma_allocation  = VK_NULL_HANDLE;
  VkFormat          vk_format       = GetVKFormat(specification.format);
  VkImageTiling     vk_tiling       = specification.cpu_readable ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL;
  
  // We use VK_IMAGE_USAGE_TRANSFER_SRC_BIT for vkCmdBlitImage which generates mip levels for the image
  VkImageUsageFlags vk_usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  if (specification.usage & kTextureUsageBitSampled) {
    vk_usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
  }
  if (specification.usage & kTextureUsageBitColorAttachment) {
    vk_usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  }
  if (specification.usage & kTextureUsageBitDepthAttachment) {
    vk_usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  }

  /* Create image */
  VkImageCreateInfo image_info{};
  image_info.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image_info.imageType     = VK_IMAGE_TYPE_2D;
  image_info.extent.width  = width;
  image_info.extent.height = height;
  image_info.extent.depth  = 1;
  image_info.mipLevels     = specification.mip_levels;
  image_info.arrayLayers   = 1;
  image_info.format        = vk_format;
  // If want to directly access texels in memory, then should use VK_IMAGE_TILING_LINEAR
  // VK_IMAGE_TILING_OPTIMAL - Implementation-based tiling (not necessarily row-major)
  image_info.tiling        = vk_tiling;
  image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_info.usage         = vk_usage;
  image_info.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
  image_info.samples       = static_cast<VkSampleCountFlagBits>(specification.samples);
  image_info.flags         = 0;

  VmaAllocationCreateInfo vma_alloc_info = {};
  vma_alloc_info.usage  = (specification.cpu_readable ? VMA_MEMORY_USAGE_AUTO : VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
  vma_alloc_info.flags |= (specification.cpu_readable ? VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT : 0);
  
  VkResult result = vmaCreateImage(allocator_, &image_info, &vma_alloc_info, &vk_image, &vma_allocation, nullptr);
  assert(result == VK_SUCCESS);

  /* Create image view */
  VkImageAspectFlags vk_aspect_flags = VK_IMAGE_ASPECT_NONE;
  if (IsDepthStencilDataFormat(specification.format)) {
    vk_aspect_flags |= VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
  } else if (IsDepthContainingDataFormat(specification.format)) {
    vk_aspect_flags |= VK_IMAGE_ASPECT_DEPTH_BIT;
  } else {
    vk_aspect_flags |= VK_IMAGE_ASPECT_COLOR_BIT;
  }

  VkImageView vk_image_view = CreateImageView(vk_image, vk_format, vk_aspect_flags, specification.mip_levels);

  /* Create texture object */
  VulkanTexture texture{specification};
  texture.vk_image       = vk_image;
  texture.vk_image_view  = vk_image_view;
  texture.vma_allocation = vma_allocation;

  TextureHandle handle = GenNextHandle();
  textures_.emplace(handle, std::move(texture));
  return handle;
}

void VulkanRenderDevice::DeleteTexture(TextureHandle handle) {
  auto it = textures_.find(handle);
  if (it != textures_.end()) {
    vkDestroyImageView(device_, it->second.vk_image_view, /*allocator=*/nullptr);

    // If not swapchain image
    if (it->second.vma_allocation != nullptr) {
      vmaDestroyImage(allocator_, it->second.vk_image, it->second.vma_allocation);
    }

    textures_.erase(it);
  }
}

const TextureSpecification& VulkanRenderDevice::GetTextureSpecification(TextureHandle handle) {
  VulkanTexture& texture = GetVulkanTexture(handle);
  return texture.specification;
}

SamplerHandle VulkanRenderDevice::CreateSampler(const SamplerSpecification& specification) {
  VkSampler vk_sampler{VK_NULL_HANDLE};

  // Samplers don't reference image objects!
  VkSamplerCreateInfo sampler_create_info{};
  sampler_create_info.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  sampler_create_info.minFilter               = GetVKFilter(specification.min_filter);
  sampler_create_info.magFilter               = GetVKFilter(specification.mag_filter);
  sampler_create_info.addressModeU            = GetVKSamplerAddressMode(specification.address_mode_u);
  sampler_create_info.addressModeV            = GetVKSamplerAddressMode(specification.address_mode_v);
  sampler_create_info.addressModeW            = GetVKSamplerAddressMode(specification.address_mode_w);
  sampler_create_info.anisotropyEnable        = specification.anisotropy_enabled;
  sampler_create_info.maxAnisotropy           = specification.max_anisotropy;
  sampler_create_info.borderColor             = GetVKBorderColor(specification.border_color);
  sampler_create_info.unnormalizedCoordinates = VK_FALSE;  // If true, then uvs are in [0, width) and [0, height)
  sampler_create_info.compareEnable           = VK_FALSE;  // Mainly used for shadow maps
  sampler_create_info.compareOp               = VK_COMPARE_OP_ALWAYS;
  sampler_create_info.mipmapMode              = GetVKSamplerMipmapMode(specification.mipmap_mode);
  sampler_create_info.mipLodBias              = specification.lod_bias;
  sampler_create_info.minLod                  = specification.min_lod;
  sampler_create_info.maxLod                  = specification.max_lod;

  VkResult result = vkCreateSampler(device_, &sampler_create_info, /*allocator=*/nullptr, &vk_sampler);
  assert(result == VK_SUCCESS);

  /* Create sampler object */
  VulkanSampler sampler{specification};
  sampler.vk_sampler = vk_sampler;

  SamplerHandle handle = GenNextHandle();
  samplers_.emplace(handle, std::move(sampler));
  return handle;
}

void VulkanRenderDevice::DeleteSampler(SamplerHandle handle) {
  auto it = samplers_.find(handle);
  if (it != samplers_.end()) {
    vkDestroySampler(device_, it->second.vk_sampler, /*allocator=*/nullptr);

    samplers_.erase(it);
  }
}

const SamplerSpecification& VulkanRenderDevice::GetSamplerSpecification(SamplerHandle handle) {
  VulkanSampler& sampler = GetVulkanSampler(handle);
  return sampler.specification;
}

void VulkanRenderDevice::CreateImage(uint32_t width, uint32_t height, uint32_t mip_levels,
                                     VkSampleCountFlagBits samples, VkFormat format, VkImageTiling tiling,
                                     VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image,
                                     VkDeviceMemory* image_memory) {
  /* Create image */
  VkImageCreateInfo image_create_info{};
  image_create_info.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image_create_info.imageType     = VK_IMAGE_TYPE_2D;
  image_create_info.extent.width  = width;
  image_create_info.extent.height = height;
  image_create_info.extent.depth  = 1;
  image_create_info.mipLevels     = mip_levels;
  image_create_info.arrayLayers   = 1;
  image_create_info.format        = format;
  // If want to directly access texels in memory, then should use VK_IMAGE_TILING_LINEAR
  // VK_IMAGE_TILING_OPTIMAL - Implementation-based tiling (not necessarily row-major)
  image_create_info.tiling        = tiling;
  image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_create_info.usage         = usage;
  image_create_info.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
  image_create_info.samples       = samples;
  image_create_info.flags         = 0;
  
  VkResult result = vkCreateImage(device_, &image_create_info, /*allocator=*/nullptr, image);
  assert(result == VK_SUCCESS);

  /* Image memory */
  VkMemoryRequirements mem_requirements;
  vkGetImageMemoryRequirements(device_, *image, &mem_requirements);

  VkMemoryAllocateInfo alloc_info{};
  alloc_info.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.allocationSize  = mem_requirements.size;
  alloc_info.memoryTypeIndex = FindMemoryType(mem_requirements.memoryTypeBits, properties);

  result = vkAllocateMemory(device_, &alloc_info, /*allocator=*/nullptr, image_memory);
  assert(result == VK_SUCCESS);

  vkBindImageMemory(device_, *image, *image_memory, /*memoryOffset=*/0);
}

VkImageView VulkanRenderDevice::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags,
                                                uint32_t mip_levels) {
  VkImageViewCreateInfo view_create_info{};
  view_create_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  view_create_info.image                           = image;
  view_create_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
  view_create_info.format                          = format;
  view_create_info.subresourceRange.aspectMask     = aspect_flags;
  view_create_info.subresourceRange.baseMipLevel   = 0;
  view_create_info.subresourceRange.levelCount     = mip_levels;
  view_create_info.subresourceRange.baseArrayLayer = 0;
  view_create_info.subresourceRange.layerCount     = 1;
  view_create_info.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
  view_create_info.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
  view_create_info.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
  view_create_info.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;

  VkImageView image_view{VK_NULL_HANDLE};
  VkResult result = vkCreateImageView(device_, &view_create_info, /*allocator=*/nullptr, &image_view);
  assert(result == VK_SUCCESS);

  return image_view;
}

/************************************************************************************************
 * BUFFER
 ************************************************************************************************/
BufferHandle VulkanRenderDevice::CreateBuffer(uint32_t size, BufferUsageFlags usage, bool dynamic_memory,
                                              void** map_data) {
  assert(dynamic_memory || (!dynamic_memory && (map_data == nullptr)));

  VulkanBuffer buffer{};
  buffer.dynamic_memory = dynamic_memory;

	VkBufferCreateInfo buffer_info = {};
	buffer_info.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.size        = size;
	buffer_info.usage       = usage | (dynamic_memory ? 0 : VK_BUFFER_USAGE_TRANSFER_DST_BIT);
  // buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;  // Owned by one queue - graphics_queue_

  VmaAllocationCreateInfo vma_alloc_info = {};
  vma_alloc_info.usage = (dynamic_memory ? VMA_MEMORY_USAGE_AUTO : VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
  vma_alloc_info.flags |= (dynamic_memory ? VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT : 0);

  VkResult result =
      vmaCreateBuffer(allocator_, &buffer_info, &vma_alloc_info, &buffer.vk_buffer, &buffer.vma_allocation, nullptr);
  assert(result == VK_SUCCESS);

  if (dynamic_memory) {
    vmaMapMemory(allocator_, buffer.vma_allocation, &buffer.map_data);
    if (map_data != nullptr) {
      *map_data = buffer.map_data;
    }
  }

  BufferHandle handle = GenNextHandle();
  buffers_.emplace(handle, std::move(buffer));
  return handle;
}

void VulkanRenderDevice::DeleteBuffer(BufferHandle handle) {
  auto it = buffers_.find(handle);
  if (it != buffers_.end()) {
    if (it->second.dynamic_memory) {
      vmaUnmapMemory(allocator_, it->second.vma_allocation);
    }

    vmaDestroyBuffer(allocator_, it->second.vk_buffer, it->second.vma_allocation);
    buffers_.erase(it);
  }
}

void VulkanRenderDevice::LoadBufferData(BufferHandle handle, uint32_t offset, uint32_t size, const void* data) {
  VulkanBuffer& buffer = GetVulkanBuffer(handle);

  if (buffer.dynamic_memory) {
    InvalidateBufferMemory(handle, offset, size);
    std::memcpy(reinterpret_cast<uint8_t*>(buffer.map_data) + offset, data, size);
    FlushBufferMemory(handle, offset, size);
  } else {
    /* Create staging buffer */
    VulkanBuffer staging_buffer = CreateStagingBuffer(size);

    /* Fill the staging buffer */
    void* map_data = nullptr;
    vmaMapMemory(allocator_, staging_buffer.vma_allocation, &map_data);
    std::memcpy(map_data, data, size);
    vmaUnmapMemory(allocator_, staging_buffer.vma_allocation);

    /* Transfer data from the staging buffer to the buffer */
    VkCommandBuffer command_buffer = BeginSingleTimeCommands();
    CopyBuffer(command_buffer, staging_buffer.vk_buffer, buffer.vk_buffer, size, 0, offset);
    EndSingleTimeCommands(command_buffer);

    /* Free the staging buffer */
    vmaDestroyBuffer(allocator_, staging_buffer.vk_buffer, staging_buffer.vma_allocation);
  }
}

void VulkanRenderDevice::InvalidateBufferMemory(BufferHandle handle, uint32_t offset, uint32_t size) {
  VulkanBuffer& buffer = GetVulkanBuffer(handle);

  assert(buffer.dynamic_memory);

  VkResult result = vmaInvalidateAllocation(allocator_, buffer.vma_allocation, offset, size);
  assert(result == VK_SUCCESS);
}

void VulkanRenderDevice::FlushBufferMemory(BufferHandle handle, uint32_t offset, uint32_t size) {
  VulkanBuffer& buffer = GetVulkanBuffer(handle);
 
  assert(buffer.dynamic_memory);

  VkResult result = vmaFlushAllocation(allocator_, buffer.vma_allocation, offset, size);
  assert(result == VK_SUCCESS);
}

/************************************************************************************************
 * DESCRIPTOR SET
 ************************************************************************************************/
DescriptorSetLayoutHandle VulkanRenderDevice::CreateDescriptorSetLayout(const DescriptorSetLayoutInfo& layout_info) {
  VkDescriptorSetLayout vk_layout{VK_NULL_HANDLE};

  std::vector<VkDescriptorSetLayoutBinding> vk_bindings;
  for (const auto& binding : layout_info.bindings_layout_info) {
    VkDescriptorSetLayoutBinding vk_binding{};
    vk_binding.binding            = binding.binding_idx;
    vk_binding.descriptorType     = GetVKDescriptorType(binding.descriptor_type);
    vk_binding.descriptorCount    = 1;
    vk_binding.stageFlags         = static_cast<VkShaderStageFlags>(binding.shader_stages);
    vk_binding.pImmutableSamplers = nullptr;

    vk_bindings.emplace_back(vk_binding);
  }

  VkDescriptorSetLayoutCreateInfo layout_create_info{};
  layout_create_info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layout_create_info.bindingCount = static_cast<uint32_t>(vk_bindings.size());
  layout_create_info.pBindings    = vk_bindings.data();

  VkResult result = vkCreateDescriptorSetLayout(device_, &layout_create_info, /*allocator=*/nullptr, &vk_layout);
  assert(result == VK_SUCCESS);

  DescriptorSetLayoutHandle handle = GenNextHandle();
  VulkanDescriptorSetLayout layout{layout_info};
  layout.vk_layout = vk_layout;
  descriptor_set_layouts_.emplace(handle, std::move(layout));

  return handle;
}

void VulkanRenderDevice::DeleteDescriptorSetLayout(DescriptorSetLayoutHandle layout_handle) {
  auto it = descriptor_set_layouts_.find(layout_handle);
  if (it != descriptor_set_layouts_.end()) {
    vkDestroyDescriptorSetLayout(device_, it->second.vk_layout, /*allocator=*/nullptr);
    descriptor_set_layouts_.erase(it);
  }
}

DescriptorSetHandle VulkanRenderDevice::CreateDescriptorSet(DescriptorSetLayoutHandle layout_handle) {
  /* Get Descriptor Set Layout */
  VulkanDescriptorSetLayout& layout = GetVulkanDescriptorSetLayout(layout_handle);

  /* Descriptor Pool Sizes */
  std::array<uint32_t, static_cast<size_t>(DescriptorType::kTotalTypes)> descriptor_counts;
  std::memset(descriptor_counts.data(), 0, descriptor_counts.size() * sizeof(descriptor_counts[0]));

  for (const auto& binding : layout.layout_info.bindings_layout_info) {
    ++descriptor_counts[static_cast<size_t>(binding.descriptor_type)];
  }

  std::vector<VkDescriptorPoolSize> vk_pool_sizes{};
  for (uint32_t i = 0; i < descriptor_counts.size(); ++i) {
    DescriptorType type = static_cast<DescriptorType>(i);

    if (type == DescriptorType::kInvalid || descriptor_counts[i] == 0) { continue; }

    VkDescriptorPoolSize vk_pool_size{};
    vk_pool_size.descriptorCount = descriptor_counts[i];
    vk_pool_size.type            = GetVKDescriptorType(type);

    vk_pool_sizes.emplace_back(vk_pool_size);
  }

  /* Create Descriptor Pool */
  VkDescriptorPool vk_pool{VK_NULL_HANDLE};

  VkDescriptorPoolCreateInfo pool_create_info{};
  pool_create_info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_create_info.poolSizeCount = static_cast<uint32_t>(vk_pool_sizes.size());
  pool_create_info.pPoolSizes    = vk_pool_sizes.data();
  pool_create_info.maxSets       = 1;

  VkResult result = vkCreateDescriptorPool(device_, &pool_create_info, /*allocator=*/nullptr, &vk_pool);
  assert(result == VK_SUCCESS);

  /* Allocate Descriptor Set */
  VkDescriptorSet vk_set{VK_NULL_HANDLE};

  VkDescriptorSetAllocateInfo alloc_info{};
  alloc_info.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc_info.descriptorPool     = vk_pool;
  alloc_info.descriptorSetCount = 1;
  alloc_info.pSetLayouts        = &layout.vk_layout;

  result = vkAllocateDescriptorSets(device_, &alloc_info, &vk_set);
  assert(result == VK_SUCCESS);

  /* Create Descriptor Set */
  VulkanDescriptorSet descriptor_set{};
  descriptor_set.layout_handle = layout_handle;
  descriptor_set.vk_pool       = vk_pool;
  descriptor_set.vk_set        = vk_set;

  DescriptorSetHandle handle = GenNextHandle();
  descriptor_sets_.emplace(handle, std::move(descriptor_set));
  return handle;
}

void VulkanRenderDevice::DeleteDescriptorSet(DescriptorSetHandle handle) {
  auto it = descriptor_sets_.find(handle);
  if (it != descriptor_sets_.end()) {
    vkDestroyDescriptorPool(device_, it->second.vk_pool, /*allocator=*/nullptr);
    descriptor_sets_.erase(it);
  }
}

void VulkanRenderDevice::WriteDescriptorUniformBuffer(DescriptorSetHandle ds_handle, uint32_t binding,
                                                      BufferHandle uniform_buffer_handle, uint32_t offset,
                                                      uint32_t size) {
  if (size == 0) {
    return;
  }
  
  VulkanDescriptorSet& descriptor_set = GetVulkanDescriptorSet(ds_handle);
  VulkanBuffer&        uniform_buffer = GetVulkanBuffer(uniform_buffer_handle);

  VkDescriptorBufferInfo buffer_info{};
  buffer_info.buffer = uniform_buffer.vk_buffer;
  buffer_info.offset = offset;
  buffer_info.range  = size;

  VkWriteDescriptorSet descriptor_write{};
  descriptor_write.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptor_write.dstSet           = descriptor_set.vk_set;
  descriptor_write.dstBinding       = binding;
  descriptor_write.dstArrayElement  = 0;
  descriptor_write.descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  descriptor_write.descriptorCount  = 1;
  descriptor_write.pBufferInfo      = &buffer_info;
  descriptor_write.pImageInfo       = nullptr;
  descriptor_write.pTexelBufferView = nullptr;

  vkUpdateDescriptorSets(device_, 1, &descriptor_write, /*descriptorCopyCount=*/0, /*pDescriptorCopies=*/nullptr);
}

void VulkanRenderDevice::WriteDescriptorStorageBuffer(DescriptorSetHandle ds_handle, uint32_t binding,
                                                      BufferHandle storage_buffer_handle, uint32_t offset,
                                                      uint32_t size) {
  if (size == 0) {
    return;
  }

  VulkanDescriptorSet& descriptor_set = GetVulkanDescriptorSet(ds_handle);
  VulkanBuffer&        storage_buffer = GetVulkanBuffer(storage_buffer_handle);

  VkDescriptorBufferInfo buffer_info{};
  buffer_info.buffer = storage_buffer.vk_buffer;
  buffer_info.offset = offset;
  buffer_info.range  = size;

  VkWriteDescriptorSet descriptor_write{};
  descriptor_write.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptor_write.dstSet           = descriptor_set.vk_set;
  descriptor_write.dstBinding       = binding;
  descriptor_write.dstArrayElement  = 0;
  descriptor_write.descriptorType   = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  descriptor_write.descriptorCount  = 1;
  descriptor_write.pBufferInfo      = &buffer_info;
  descriptor_write.pImageInfo       = nullptr;
  descriptor_write.pTexelBufferView = nullptr;

  vkUpdateDescriptorSets(device_, 1, &descriptor_write, /*descriptorCopyCount=*/0, /*pDescriptorCopies=*/nullptr);
}

void VulkanRenderDevice::WriteDescriptorInputAttachment(DescriptorSetHandle ds_handle, uint32_t binding_idx,
                                                        TextureHandle texture_handle) {
  VulkanDescriptorSet& descriptor_set = GetVulkanDescriptorSet(ds_handle);
  VulkanTexture&       texture        = GetVulkanTexture(texture_handle);

  VkDescriptorImageInfo image_info{};
  image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  image_info.imageView   = texture.vk_image_view;
  image_info.sampler     = VK_NULL_HANDLE;

  VkWriteDescriptorSet descriptor_write{};
  descriptor_write.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptor_write.dstSet           = descriptor_set.vk_set;
  descriptor_write.dstBinding       = binding_idx;
  descriptor_write.dstArrayElement  = 0;
  descriptor_write.descriptorType   = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
  descriptor_write.descriptorCount  = 1;
  descriptor_write.pBufferInfo      = nullptr;
  descriptor_write.pImageInfo       = &image_info;
  descriptor_write.pTexelBufferView = nullptr;

  vkUpdateDescriptorSets(device_, 1, &descriptor_write, /*descriptorCopyCount=*/0, /*pDescriptorCopies=*/nullptr);
}

void VulkanRenderDevice::WriteDescriptorSampler(DescriptorSetHandle ds_handle, uint32_t binding_idx,
                                            TextureHandle texture_handle, SamplerHandle sampler_handle) {
  VulkanDescriptorSet& descriptor_set = GetVulkanDescriptorSet(ds_handle);
  VulkanTexture&       texture        = GetVulkanTexture(texture_handle);
  VulkanSampler&       sampler        = GetVulkanSampler(sampler_handle);

  VkDescriptorImageInfo image_info{};
  image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  image_info.imageView   = texture.vk_image_view;
  image_info.sampler     = sampler.vk_sampler;

  VkWriteDescriptorSet descriptor_write{};
  descriptor_write.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptor_write.dstSet           = descriptor_set.vk_set;
  descriptor_write.dstBinding       = binding_idx;
  descriptor_write.dstArrayElement  = 0;
  descriptor_write.descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  descriptor_write.descriptorCount  = 1;
  descriptor_write.pBufferInfo      = nullptr;
  descriptor_write.pImageInfo       = &image_info;
  descriptor_write.pTexelBufferView = nullptr;

  vkUpdateDescriptorSets(device_, 1, &descriptor_write, /*descriptorCopyCount=*/0, /*pDescriptorCopies=*/nullptr);
}

/************************************************************************************************
 * RENDER PASS
 ************************************************************************************************/
RenderPassHandle VulkanRenderDevice::CreateRenderPass(const RenderPassDescription& render_pass_description) {
  /* Attachments */
  std::vector<VkAttachmentDescription> vk_attachments;
  for (const auto& attachment : render_pass_description.attachments) {
    VkAttachmentDescription vk_attachment{};
    vk_attachment.format         = GetVKFormat(attachment.format);
    vk_attachment.samples        = static_cast<VkSampleCountFlagBits>(attachment.samples);
    vk_attachment.loadOp         = GetVKAttachmentLoadOperation(attachment.load_op);
    vk_attachment.storeOp        = GetVKAttachmentStoreOperation(attachment.store_op);
    vk_attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    vk_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    vk_attachment.initialLayout  = GetVKImageLayout(attachment.initial_layout);
    vk_attachment.finalLayout    = GetVKImageLayout(attachment.final_layout);

    vk_attachments.emplace_back(std::move(vk_attachment));
  }

  /* Subpasses */
  struct TmpVkSubpassRefs {
    std::vector<VkAttachmentReference> vk_color_attachment_refs;
    std::vector<VkAttachmentReference> vk_resolve_attachment_refs;
    std::vector<VkAttachmentReference> vk_depth_stencil_attachment_refs;
    std::vector<VkAttachmentReference> vk_input_attachment_refs;
  };

  std::vector<TmpVkSubpassRefs> tmp_vk_subpasses_refs;
  std::vector<VkSubpassDescription> vk_subpasses;

  for (const auto& subpass : render_pass_description.subpasses) {
    TmpVkSubpassRefs vk_subpass_refs;

    for (const auto& ref : subpass.color_attachments) {
      VkAttachmentReference vk_ref{};
      vk_ref.attachment = ref.attachment_idx;
      vk_ref.layout     = GetVKImageLayout(ref.layout);
      vk_subpass_refs.vk_color_attachment_refs.emplace_back(std::move(vk_ref));
    }

    for (const auto& ref : subpass.resolve_attachments) {
      VkAttachmentReference vk_ref{};
      vk_ref.attachment = ref.attachment_idx;
      vk_ref.layout     = GetVKImageLayout(ref.layout);
      vk_subpass_refs.vk_resolve_attachment_refs.emplace_back(std::move(vk_ref));
    }

    if (subpass.depth_stencil_attachment.has_value()) {
      VkAttachmentReference vk_ref{};
      vk_ref.attachment = subpass.depth_stencil_attachment->attachment_idx;
      vk_ref.layout     = GetVKImageLayout(subpass.depth_stencil_attachment->layout);
      vk_subpass_refs.vk_depth_stencil_attachment_refs.emplace_back(std::move(vk_ref));
    }

    for (const auto& ref : subpass.input_attachments) {
      VkAttachmentReference vk_ref{};
      vk_ref.attachment = ref.attachment_idx;
      vk_ref.layout     = GetVKImageLayout(ref.layout);
      vk_subpass_refs.vk_input_attachment_refs.emplace_back(std::move(vk_ref));
    }

    VkSubpassDescription vk_subpass{};
    vk_subpass.pipelineBindPoint       = GetVKPipelineBindPoint(subpass.bind_point);
    vk_subpass.colorAttachmentCount    = vk_subpass_refs.vk_color_attachment_refs.size();
    vk_subpass.pColorAttachments       = vk_subpass_refs.vk_color_attachment_refs.data();
    vk_subpass.inputAttachmentCount    = vk_subpass_refs.vk_input_attachment_refs.size();
    vk_subpass.pInputAttachments       = vk_subpass_refs.vk_input_attachment_refs.data();
    // For resolving multisampling, can be either nullptr or an array of colorAttachmentCount resolve attachments,
    // NOTE: in vulkan all color attachments must have the same number of samples!
    vk_subpass.pResolveAttachments     = vk_subpass_refs.vk_resolve_attachment_refs.data();
    vk_subpass.pDepthStencilAttachment = vk_subpass_refs.vk_depth_stencil_attachment_refs.data();
    vk_subpass.preserveAttachmentCount = subpass.preserve_attachments.size();
    vk_subpass.pPreserveAttachments    = subpass.preserve_attachments.data();

    tmp_vk_subpasses_refs.emplace_back(std::move(vk_subpass_refs));
    vk_subpasses.emplace_back(std::move(vk_subpass));
  }

  /* Subpass Dependencies */
  std::vector<VkSubpassDependency> vk_dependencies;
  for (const auto& dependency : render_pass_description.subpass_dependencies) {
    VkSubpassDependency vk_dependency{};
    // What subpass we're dependent on
    vk_dependency.srcSubpass    = dependency.dependency_subpass_idx;
    // For which subpass this dependency is declared
    vk_dependency.dstSubpass    = dependency.dependent_subpass_idx;
    // Stages we require to be finished within srcSubpass before executing dstSubpass
    vk_dependency.srcStageMask  = dependency.dependency_stage_mask;
    // Stages NOT to execute in dstSubpass before the stages in srcStageMask have been finished within srcSubpass
    vk_dependency.dstStageMask  = dependency.dependent_stage_mask;
    // Bitmask of all memory access types used by srcSubpass
    vk_dependency.srcAccessMask = dependency.dependency_access_mask;
    // Bitmask of all memory access types used by dstSubpass
    vk_dependency.dstAccessMask = dependency.dependent_access_mask;

    vk_dependencies.emplace_back(std::move(vk_dependency));
  }

  /* RenderPass */
  VkRenderPass vk_render_pass{VK_NULL_HANDLE};

  VkRenderPassCreateInfo render_pass_create_info{};
  render_pass_create_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_create_info.attachmentCount = static_cast<uint32_t>(vk_attachments.size());
  render_pass_create_info.pAttachments    = vk_attachments.data();
  render_pass_create_info.subpassCount    = static_cast<uint32_t>(vk_subpasses.size());
  render_pass_create_info.pSubpasses      = vk_subpasses.data();
  render_pass_create_info.dependencyCount = static_cast<uint32_t>(vk_dependencies.size());
  render_pass_create_info.pDependencies   = vk_dependencies.data();

  VkResult result = vkCreateRenderPass(device_, &render_pass_create_info, /*allocator=*/nullptr, &vk_render_pass);
  assert(result == VK_SUCCESS);

  VulkanRenderPass render_pass{render_pass_description};
  render_pass.vk_render_pass = vk_render_pass;

  RenderPassHandle handle = GenNextHandle();
  render_passes_.emplace(handle, std::move(render_pass));
  return handle;
}

void VulkanRenderDevice::DeleteRenderPass(RenderPassHandle handle) {
  auto it = render_passes_.find(handle);
  if (it != render_passes_.end()) {
    vkDestroyRenderPass(device_, it->second.vk_render_pass, /*allocator=*/nullptr);
    render_passes_.erase(it);
  }
}

FramebufferHandle VulkanRenderDevice::CreateFramebuffer(const std::vector<TextureHandle>& attachments,
                                                    RenderPassHandle compatible_render_pass_handle) {
  VulkanRenderPass& render_pass = GetVulkanRenderPass(compatible_render_pass_handle);

  std::vector<VkImageView> vk_image_views;
  uint32_t width{0};
  uint32_t height{0};

  for (const auto& texture_handle : attachments) {
    VulkanTexture& texture = GetVulkanTexture(texture_handle);

    vk_image_views.push_back(texture.vk_image_view);
    width  = texture.specification.width;
    height = texture.specification.height;
  }

  VkFramebuffer vk_framebuffer{VK_NULL_HANDLE};

  VkFramebufferCreateInfo framebuffer_create_info{};
  framebuffer_create_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  framebuffer_create_info.renderPass      = render_pass.vk_render_pass;
  framebuffer_create_info.attachmentCount = vk_image_views.size();
  framebuffer_create_info.pAttachments    = vk_image_views.data();
  framebuffer_create_info.width           = width;
  framebuffer_create_info.height          = height;
  framebuffer_create_info.layers          = 1;  // TODO:

  VkResult result = vkCreateFramebuffer(device_, &framebuffer_create_info, /*allocator=*/nullptr, &vk_framebuffer);
  assert(result == VK_SUCCESS);

  VulkanFramebuffer framebuffer{};
  framebuffer.vk_framebuffer = vk_framebuffer;

  FramebufferHandle handle = GenNextHandle();
  framebuffers_.emplace(handle, std::move(framebuffer));
  return handle;
}

void VulkanRenderDevice::DeleteFramebuffer(FramebufferHandle handle) {
  auto it = framebuffers_.find(handle);
  if (it != framebuffers_.end()) {
    vkDestroyFramebuffer(device_, it->second.vk_framebuffer, /*allocator=*/nullptr);
    framebuffers_.erase(it);
  }
}

/************************************************************************************************
 * PIPELINE
 ************************************************************************************************/
ShaderModuleHandle VulkanRenderDevice::CreateShaderModule(ShaderModuleType type, uint32_t binary_size,
                                                          const uint32_t* binary) {
  VkShaderModuleCreateInfo create_info{};
  create_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.codeSize = binary_size;
  create_info.pCode    = binary;
  
  VkShaderModule vk_module{};
  VkResult result = vkCreateShaderModule(device_, &create_info, /*allocator=*/nullptr, &vk_module);
  assert(result == VK_SUCCESS);

  VulkanShaderModule shader_module{type};
  shader_module.vk_module = vk_module;

  ShaderModuleHandle handle = GenNextHandle();
  shader_modules_.emplace(handle, std::move(shader_module));
  return handle;
}

void VulkanRenderDevice::DeleteShaderModule(ShaderModuleHandle handle) {
  auto it = shader_modules_.find(handle);
  if (it != shader_modules_.end()) {
    vkDestroyShaderModule(device_, it->second.vk_module, /*allocator=*/nullptr);
    shader_modules_.erase(it);
  }
}

PipelineHandle VulkanRenderDevice::CreatePipeline(const PipelineDescription& description,
                                                  RenderPassHandle compatible_render_pass_handle,
                                                  uint32_t subpass_idx) {
  /* Get Render Pass */
  const VulkanRenderPass& render_pass = GetVulkanRenderPass(compatible_render_pass_handle);

  assert(subpass_idx < render_pass.description.subpasses.size());
  const SubpassDescription& subpass_description = render_pass.description.subpasses[subpass_idx];

  uint32_t msaa_samples = 1;
  if (subpass_description.color_attachments.size() > 0) {
    msaa_samples = render_pass.description.attachments[subpass_description.color_attachments[0].attachment_idx].samples;
  }

  /* Shader stages */
  std::vector<VkPipelineShaderStageCreateInfo> vk_stages_info{description.shader_modules_count};
  for (uint32_t i = 0; i < description.shader_modules_count; ++i) {
    const VulkanShaderModule& shader_module = GetVulkanShaderModule(description.shader_modules[i]);

    VkPipelineShaderStageCreateInfo vk_stage_create_info{};
    vk_stage_create_info.sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vk_stage_create_info.stage               = static_cast<VkShaderStageFlagBits>(GetShaderStageBitFromShaderModuleType(shader_module.type));
    vk_stage_create_info.module              = shader_module.vk_module;
    vk_stage_create_info.pName               = "main";
    vk_stage_create_info.pSpecializationInfo = nullptr;  // Used for compile-time constants

    vk_stages_info[i] = std::move(vk_stage_create_info);
  }

  /* Vertex input state */
  assert(description.input_vertex_data_info);

  std::vector<VkVertexInputBindingDescription> vk_binding_descriptions;
  std::vector<VkVertexInputAttributeDescription> vk_attribute_descriptions;
  for (const auto& binding_info : description.input_vertex_data_info->bindings) {
    VkVertexInputBindingDescription vk_binding_description{};
    vk_binding_description.binding   = binding_info.binding_slot;
    vk_binding_description.stride    = binding_info.stride;
    vk_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    
    vk_binding_descriptions.emplace_back(std::move(vk_binding_description));

    for (const auto& attribute_info : binding_info.attributes_info) {
      VkVertexInputAttributeDescription vk_attribute_description{};
      vk_attribute_description.binding  = binding_info.binding_slot;
      vk_attribute_description.location = attribute_info.location;
      vk_attribute_description.format   = GetVKFormat(attribute_info.format);
      vk_attribute_description.offset   = attribute_info.offset;

      vk_attribute_descriptions.emplace_back(std::move(vk_attribute_description));
    }
  }

  VkPipelineVertexInputStateCreateInfo vk_vertex_input_info{};
  vk_vertex_input_info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vk_vertex_input_info.vertexBindingDescriptionCount   = static_cast<uint32_t>(vk_binding_descriptions.size());
  vk_vertex_input_info.pVertexBindingDescriptions      = vk_binding_descriptions.data();
  vk_vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(vk_attribute_descriptions.size());
  vk_vertex_input_info.pVertexAttributeDescriptions    = vk_attribute_descriptions.data();

  /* Input assembly state */
  VkPipelineInputAssemblyStateCreateInfo vk_input_assembly_info{};
  vk_input_assembly_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  vk_input_assembly_info.topology               = GetVKPrimitiveTopology(description.input_assembly_info.topology);
  vk_input_assembly_info.primitiveRestartEnable = VK_FALSE;

  /* Dynamic states */
  const std::vector<VkDynamicState> vk_dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT};

  VkPipelineDynamicStateCreateInfo vk_dynamic_state_info{};
  vk_dynamic_state_info.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  vk_dynamic_state_info.dynamicStateCount = static_cast<uint32_t>(vk_dynamic_states.size());
  vk_dynamic_state_info.pDynamicStates    = vk_dynamic_states.data();

  /* Viewport and scissors */
  VkViewport vk_viewport{};
  vk_viewport.x        = 0.0f;
  vk_viewport.y        = 0.0f;
  vk_viewport.width    = 0.0f;
  vk_viewport.height   = 0.0f;
  vk_viewport.minDepth = 0.0f;
  vk_viewport.maxDepth = 1.0f;

  VkRect2D vk_scissor{};
  vk_scissor.offset = {0, 0};
  vk_scissor.extent = {0, 0};

  VkPipelineViewportStateCreateInfo vk_viewport_state_info{};
  vk_viewport_state_info.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  vk_viewport_state_info.viewportCount = 1;  // > 1 requires VkPhysicalDeviceFeatures::multiViewport to be true
  vk_viewport_state_info.scissorCount  = 1;  // > 1 requires VkPhysicalDeviceFeatures::multiViewport to be true
  vk_viewport_state_info.pViewports    = &vk_viewport;
  vk_viewport_state_info.pScissors     = &vk_scissor;

  /* Rasterization state */
  VkPipelineRasterizationStateCreateInfo vk_rasterizer_info{};
  vk_rasterizer_info.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  // Disable any output to the framebuffer
  vk_rasterizer_info.rasterizerDiscardEnable = VK_FALSE;
  // LINE and POINT require ?some feature?
  vk_rasterizer_info.polygonMode             = GetVKPolygonMode(description.rasterization_info.polygon_mode);
  // > 1 requires VkPhysicalDeviceFeatures::wideLines
  vk_rasterizer_info.lineWidth               = 1.0f;
  vk_rasterizer_info.cullMode                = GetVKCullMode(description.rasterization_info.cull_mode);
  vk_rasterizer_info.frontFace               = GetVKFrontFace(description.rasterization_info.front_face);
  // Used for shadow mapping
  vk_rasterizer_info.depthBiasEnable         = VK_FALSE;
  vk_rasterizer_info.depthBiasConstantFactor = 0.0f;
  vk_rasterizer_info.depthBiasClamp          = 0.0f;
  vk_rasterizer_info.depthBiasSlopeFactor    = 0.0f;

  // Clamping fragments outside near/far planes instead of discarding (used for shadow mapping),
  // requires VkPhysicalDeviceFeatures::depthClamp to be true
  vk_rasterizer_info.depthClampEnable        = VK_FALSE;

  /* Multisampling (requires ?some feature?) */
  VkPipelineMultisampleStateCreateInfo vk_multisampling_info{};
  vk_multisampling_info.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  vk_multisampling_info.sampleShadingEnable   = VK_FALSE;
  vk_multisampling_info.rasterizationSamples  = static_cast<VkSampleCountFlagBits>(msaa_samples);
  vk_multisampling_info.minSampleShading      = 1.0f;
  vk_multisampling_info.pSampleMask           = nullptr;
  vk_multisampling_info.alphaToCoverageEnable = VK_FALSE;
  vk_multisampling_info.alphaToOneEnable      = VK_FALSE;

  /* Depth and stencil testing */
  VkPipelineDepthStencilStateCreateInfo vk_depth_stencil_info{};
  vk_depth_stencil_info.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  vk_depth_stencil_info.depthTestEnable       = description.depth_test_description.test_enable;
  vk_depth_stencil_info.depthWriteEnable      = description.depth_test_description.write_enable;
  vk_depth_stencil_info.depthCompareOp        = GetVKCompareOp(description.depth_test_description.compare_op);
  vk_depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
  vk_depth_stencil_info.minDepthBounds        = 0.0f;
  vk_depth_stencil_info.maxDepthBounds        = 1.0f;
  vk_depth_stencil_info.stencilTestEnable     = VK_FALSE; // TODO:
  vk_depth_stencil_info.front                 = {};
  vk_depth_stencil_info.back                  = {};

  /* Color blending */
  VkPipelineColorBlendAttachmentState vk_blend_attachment_state{};  // Created for each color attachment
  vk_blend_attachment_state.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  vk_blend_attachment_state.blendEnable         = description.blend_description.enable;
  vk_blend_attachment_state.srcColorBlendFactor = GetVKBlendFactor(description.blend_description.src_color_blend_factor);
  vk_blend_attachment_state.dstColorBlendFactor = GetVKBlendFactor(description.blend_description.dst_color_blend_factor);
  vk_blend_attachment_state.colorBlendOp        = GetVKBlendOp(description.blend_description.color_blend_op);
  vk_blend_attachment_state.srcAlphaBlendFactor = GetVKBlendFactor(description.blend_description.src_alpha_blend_factor);
  vk_blend_attachment_state.dstAlphaBlendFactor = GetVKBlendFactor(description.blend_description.dst_alpha_blend_factor);
  vk_blend_attachment_state.alphaBlendOp        = GetVKBlendOp(description.blend_description.alpha_blend_op);

  std::vector<VkPipelineColorBlendAttachmentState> vk_blend_attachment_states{
      subpass_description.color_attachments.size(), vk_blend_attachment_state};

  VkPipelineColorBlendStateCreateInfo vk_color_blend_info{};
  vk_color_blend_info.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  vk_color_blend_info.logicOpEnable     = VK_FALSE;
  vk_color_blend_info.logicOp           = VK_LOGIC_OP_COPY;
  vk_color_blend_info.attachmentCount   = static_cast<uint32_t>(vk_blend_attachment_states.size());
  vk_color_blend_info.pAttachments      = vk_blend_attachment_states.data();
  vk_color_blend_info.blendConstants[0] = 0.0f;
  vk_color_blend_info.blendConstants[1] = 0.0f;
  vk_color_blend_info.blendConstants[2] = 0.0f;
  vk_color_blend_info.blendConstants[3] = 0.0f;

  /* Pipeline layout */
  VkPipelineLayout vk_pipeline_layout{VK_NULL_HANDLE};

  std::vector<VkDescriptorSetLayout> vk_descriptor_set_layouts{description.descriptor_sets_count};
  for (uint32_t i = 0; i < description.descriptor_sets_count; ++i) {
    vk_descriptor_set_layouts[i] = GetVulkanDescriptorSetLayout(description.descriptor_set_layouts[i]).vk_layout;
  }

  std::vector<VkPushConstantRange> vk_push_constant_ranges{description.push_constant_ranges_count};
  for (uint32_t i = 0; i < description.push_constant_ranges_count; ++i) {
    vk_push_constant_ranges[i].offset     = description.push_constant_ranges[i].offset;
    vk_push_constant_ranges[i].size       = description.push_constant_ranges[i].size;
    vk_push_constant_ranges[i].stageFlags =
        static_cast<VkShaderStageFlagBits>(description.push_constant_ranges[i].shader_stages);
  }

  VkPipelineLayoutCreateInfo vk_pipeline_layout_info{};
  vk_pipeline_layout_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  vk_pipeline_layout_info.setLayoutCount         = static_cast<uint32_t>(vk_descriptor_set_layouts.size());
  vk_pipeline_layout_info.pSetLayouts            = vk_descriptor_set_layouts.data();
  vk_pipeline_layout_info.pushConstantRangeCount = static_cast<uint32_t>(vk_push_constant_ranges.size());
  vk_pipeline_layout_info.pPushConstantRanges    = vk_push_constant_ranges.data();

  VkResult result =
      vkCreatePipelineLayout(device_, &vk_pipeline_layout_info, /*allocator=*/nullptr, &vk_pipeline_layout);
  assert(result == VK_SUCCESS);

  /* Graphics pipeline */
  VkPipeline vk_pipeline{VK_NULL_HANDLE};

  VkGraphicsPipelineCreateInfo vk_pipeline_info{};
  vk_pipeline_info.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

  vk_pipeline_info.stageCount          = static_cast<uint32_t>(vk_stages_info.size());
  vk_pipeline_info.pStages             = vk_stages_info.data();

  vk_pipeline_info.pVertexInputState   = &vk_vertex_input_info;
  vk_pipeline_info.pInputAssemblyState = &vk_input_assembly_info;
  vk_pipeline_info.pViewportState      = &vk_viewport_state_info;
  vk_pipeline_info.pRasterizationState = &vk_rasterizer_info;
  vk_pipeline_info.pMultisampleState   = &vk_multisampling_info;
  vk_pipeline_info.pDepthStencilState  = &vk_depth_stencil_info;
  vk_pipeline_info.pColorBlendState    = &vk_color_blend_info;
  vk_pipeline_info.pDynamicState       = &vk_dynamic_state_info;

  vk_pipeline_info.layout              = vk_pipeline_layout;

  vk_pipeline_info.renderPass          = render_pass.vk_render_pass;
  vk_pipeline_info.subpass             = subpass_idx;

  // These are used only if VK_PIPELINE_CREATE_DERIVATIVE_BIT flag is set
  vk_pipeline_info.basePipelineHandle  = VK_NULL_HANDLE;
  vk_pipeline_info.basePipelineIndex   = -1;

  result = vkCreateGraphicsPipelines(device_, /*pipelineCache=*/VK_NULL_HANDLE, 1, &vk_pipeline_info,
                                     /*allocator=*/nullptr, &vk_pipeline);
  assert(result == VK_SUCCESS);

  PipelineHandle handle = GenNextHandle();
  VulkanPipeline pipeline{description};
  pipeline.vk_pipeline        = vk_pipeline;
  pipeline.vk_pipeline_layout = vk_pipeline_layout;
  pipelines_.emplace(handle, std::move(pipeline));

  return handle;
}

void VulkanRenderDevice::DeletePipeline(PipelineHandle handle) {
  auto it = pipelines_.find(handle);
  if (it != pipelines_.end()) {
    vkDestroyPipeline(device_, it->second.vk_pipeline, /*allocator=*/nullptr);
    vkDestroyPipelineLayout(device_, it->second.vk_pipeline_layout, /*allocator=*/nullptr);
    pipelines_.erase(it);
  }
}

/************************************************************************************************
 * COMMAND BUFFER
 ************************************************************************************************/
CommandBuffer* VulkanRenderDevice::CreateCommandBuffer(CommandBufferType type, bool temporary) {
  assert(type != CommandBufferType::kInvalid);
  return new VulkanCommandBuffer(type, *this, temporary);
}

void VulkanRenderDevice::DeleteCommandBuffer(CommandBuffer* command_buffer) {
  assert(command_buffer);
  delete command_buffer;
}

VulkanTexture& VulkanRenderDevice::GetVulkanTexture(TextureHandle handle) {
  auto it = textures_.find(handle);
  assert(it != textures_.end());
  return it->second;
}

VulkanSampler& VulkanRenderDevice::GetVulkanSampler(SamplerHandle handle) {
  auto it = samplers_.find(handle);
  assert(it != samplers_.end());
  return it->second;
}

VulkanBuffer& VulkanRenderDevice::GetVulkanBuffer(BufferHandle handle) {
  auto it = buffers_.find(handle);
  assert(it != buffers_.end());
  return it->second;
}

VulkanDescriptorSetLayout& VulkanRenderDevice::GetVulkanDescriptorSetLayout(DescriptorSetLayoutHandle handle) {
  auto it = descriptor_set_layouts_.find(handle);
  assert(it != descriptor_set_layouts_.end());
  return it->second;
}

VulkanDescriptorSet& VulkanRenderDevice::GetVulkanDescriptorSet(DescriptorSetHandle handle) {
  auto it = descriptor_sets_.find(handle);
  assert(it != descriptor_sets_.end());
  return it->second;
}

VulkanRenderPass& VulkanRenderDevice::GetVulkanRenderPass(RenderPassHandle handle) {
  auto it = render_passes_.find(handle);
  assert(it != render_passes_.end());
  return it->second;
}

VulkanFramebuffer& VulkanRenderDevice::GetVulkanFramebuffer(FramebufferHandle handle) {
  auto it = framebuffers_.find(handle);
  assert(it != framebuffers_.end());
  return it->second;
}

VulkanShaderModule& VulkanRenderDevice::GetVulkanShaderModule(ShaderModuleHandle handle) {
  auto it = shader_modules_.find(handle);
  assert(it != shader_modules_.end());
  return it->second;
}

VulkanPipeline& VulkanRenderDevice::GetVulkanPipeline(PipelineHandle handle) {
  auto it = pipelines_.find(handle);
  assert(it != pipelines_.end());
  return it->second;
}

VulkanSwapchain& VulkanRenderDevice::GetVulkanSwapchain(SwapchainHandle handle) {
  auto it = swapchains_.find(handle);
  assert(it != swapchains_.end());
  return it->second;
}
