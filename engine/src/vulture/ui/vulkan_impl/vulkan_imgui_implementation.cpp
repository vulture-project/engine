/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file vulkan_imgui_implementation.cpp
 * @date 2023-03-30
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

#include <vulture/ui/vulkan_impl/imgui_impl_glfw.h>
#include <vulture/ui/vulkan_impl/imgui_impl_vulkan.h>
#include <ImGuizmo.h>
#include <vulture/ui/vulkan_impl/vulkan_imgui_implementation.hpp>

using namespace vulture;

VulkanImGuiImplementation::VulkanImGuiImplementation(Window& window, VulkanRenderDevice& device,
                                                     SwapchainHandle swapchain)
    : window_(window), device_(device), swapchain_(&device_.GetVulkanSwapchain(swapchain)) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable Docking

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  ImGui::StyleColorsLight();
  //ImGui::StyleColorsClassic();

  InitImplementation();
}

VulkanImGuiImplementation::~VulkanImGuiImplementation() {
  VkDevice vk_device = device_.device_;

  vkDestroySampler(vk_device, texture_ui_sampler_, /*pAllocator=*/nullptr);

  vkDestroyRenderPass(vk_device, render_pass_, /*pAllocator=*/nullptr);

  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  vkDestroyDescriptorPool(vk_device, descriptor_pool_, /*pAllocator=*/nullptr);
}

void VulkanImGuiImplementation::OnResize(SwapchainHandle new_swapchain) {
  swapchain_ = &device_.GetVulkanSwapchain(new_swapchain);
  ImGui_ImplVulkan_SetMinImageCount(swapchain_->textures.size()); // FIXME: (tralf-strues)

  RecreateFramebuffers();
}

void VulkanImGuiImplementation::UpdateFonts(const char* filename, float size) {
  ImGuiIO& io = ImGui::GetIO();
  io.Fonts->Clear();

  float scale = io.DisplayFramebufferScale.y;

  ImFontConfig cfg;
  cfg.SizePixels = size * scale;
  io.Fonts->AddFontFromFileTTF(filename, size * scale, &cfg);
  io.FontGlobalScale = 1.0f / scale;

  VkCommandBuffer command_buffer = device_.BeginSingleTimeCommands();
  ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
  device_.EndSingleTimeCommands(command_buffer);
}

void VulkanImGuiImplementation::FrameStart() {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  ImGuizmo::BeginFrame();

  ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
  if (opt_fullscreen_) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
  } else {
    dockspace_flags_ &= ~ImGuiDockNodeFlags_PassthruCentralNode;
  }

  if (dockspace_flags_ & ImGuiDockNodeFlags_PassthruCentralNode) {
    window_flags |= ImGuiWindowFlags_NoBackground;
  }

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("DockSpace Demo", &dockspace_open_, window_flags);
  ImGui::PopStyleVar();

  if (opt_fullscreen_) ImGui::PopStyleVar(2);

  // Submit the DockSpace
  ImGuiIO& io = ImGui::GetIO();
  if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags_);
  }

  ImGui::End();
}

void VulkanImGuiImplementation::Render(CommandBuffer& command_buffer, uint32_t swapchain_texture_idx) {
  ImGui::Render();

  VulkanCommandBuffer& vulkan_command_buffer = dynamic_cast<VulkanCommandBuffer&>(command_buffer);

  VkClearValue vk_clear_value;
  vk_clear_value.color.float32[0] = 0.0f;
  vk_clear_value.color.float32[1] = 0.0f;
  vk_clear_value.color.float32[2] = 0.0f;
  vk_clear_value.color.float32[3] = 0.0f;

  VkRenderPassBeginInfo info = {};
  info.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  info.renderPass        = render_pass_;
  info.framebuffer       = framebuffers_[swapchain_texture_idx];
  info.renderArea.extent = swapchain_->vk_extent;
  info.clearValueCount   = 1;
  info.pClearValues      = &vk_clear_value;
  vkCmdBeginRenderPass(vulkan_command_buffer.vk_command_buffer_, &info, VK_SUBPASS_CONTENTS_INLINE);

  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), vulkan_command_buffer.vk_command_buffer_);

  vkCmdEndRenderPass(vulkan_command_buffer.vk_command_buffer_);
}

void VulkanImGuiImplementation::FrameEnd() {}

ImGuiTextureHandle VulkanImGuiImplementation::AddTextureUI(TextureHandle texture) {
  const VulkanTexture& vulkan_texture = device_.GetVulkanTexture(texture);

  return ImGui_ImplVulkan_AddTexture(texture_ui_sampler_, vulkan_texture.vk_image_view,
                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void VulkanImGuiImplementation::RemoveTextureUI(ImGuiTextureHandle imgui_texture) {
  ImGui_ImplVulkan_RemoveTexture(reinterpret_cast<VkDescriptorSet>(imgui_texture));
}

void VulkanImGuiImplementation::InitImplementation() {
  ImGui_ImplGlfw_InitForVulkan(window_.GetNativeWindow(), true);

  CreateDescriptorPool();
  CreateRenderPass();
  CreateTextureUISampler();
  RecreateFramebuffers();

  /* Init implementation */
  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.Instance        = device_.instance_;
  init_info.PhysicalDevice  = device_.physical_device_;
  init_info.Device          = device_.device_;
  init_info.QueueFamily     = device_.queue_family_indices_.graphics_family.value();
  init_info.Queue           = device_.graphics_queue_;
  init_info.PipelineCache   = VK_NULL_HANDLE;
  init_info.DescriptorPool  = descriptor_pool_;
  init_info.Subpass         = 0;
  init_info.MinImageCount   = swapchain_->textures.size();  // FIXME: (tralf-strues)
  init_info.ImageCount      = swapchain_->textures.size();
  init_info.MSAASamples     = VK_SAMPLE_COUNT_1_BIT;
  init_info.Allocator       = nullptr;
  init_info.CheckVkResultFn = nullptr;
  ImGui_ImplVulkan_Init(&init_info, render_pass_);
}

void VulkanImGuiImplementation::CreateDescriptorPool() {
  VkDevice device = device_.device_;

  VkDescriptorPoolSize pool_sizes[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
                                       {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                                       {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
                                       {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
                                       {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
                                       {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
                                       {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
                                       {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
                                       {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                                       {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                                       {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

  VkResult result{VK_SUCCESS};

  VkDescriptorPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
  pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
  pool_info.pPoolSizes = pool_sizes;
  result = vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptor_pool_);
  assert(result == VK_SUCCESS);
}

void VulkanImGuiImplementation::CreateRenderPass() {
  VkDevice device = device_.device_;

  VkAttachmentDescription attachment = {};
  attachment.format                  = swapchain_->vk_format;
  attachment.samples                 = VK_SAMPLE_COUNT_1_BIT;
  attachment.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachment.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
  attachment.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachment.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachment.initialLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
  attachment.finalLayout              = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference color_attachment = {};
  color_attachment.attachment            = 0;
  color_attachment.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments    = &color_attachment;

  VkRenderPassCreateInfo info = {};
  info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  info.attachmentCount = 1;
  info.pAttachments    = &attachment;
  info.subpassCount    = 1;
  info.pSubpasses      = &subpass;
  info.dependencyCount = 0;
  info.pDependencies   = nullptr;

  VkResult result = vkCreateRenderPass(device, &info, nullptr, &render_pass_);
  assert(result == VK_SUCCESS);
}

void VulkanImGuiImplementation::CreateTextureUISampler() {
  VkSamplerCreateInfo sampler_info{};
  sampler_info.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  sampler_info.minFilter               = VK_FILTER_LINEAR;
  sampler_info.magFilter               = VK_FILTER_LINEAR;
  sampler_info.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.anisotropyEnable        = false;
  sampler_info.maxAnisotropy           = 0;
  sampler_info.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  sampler_info.unnormalizedCoordinates = VK_FALSE;  // If true, then uvs are in [0, width) and [0, height)
  sampler_info.compareEnable           = VK_FALSE;  // Mainly used for shadow maps
  sampler_info.compareOp               = VK_COMPARE_OP_ALWAYS;
  sampler_info.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  sampler_info.mipLodBias              = 0;
  sampler_info.minLod                  = 0;
  sampler_info.maxLod                  = 0;

  VkResult result = vkCreateSampler(device_.device_, &sampler_info, /*allocator=*/nullptr, &texture_ui_sampler_);
  assert(result == VK_SUCCESS);
}

void VulkanImGuiImplementation::RecreateFramebuffers() {
  uint32_t swapchain_size = swapchain_->textures.size();

  if (!framebuffers_.empty()) {
    for (auto& framebuffer : framebuffers_) {
      vkDestroyFramebuffer(device_.device_, framebuffer, /*pAllocator=*/nullptr);
    }
  }

  framebuffers_.resize(swapchain_size);
  for (uint32_t i = 0; i < swapchain_size; ++i) {
    auto& swapchain_texture = device_.GetVulkanTexture(swapchain_->textures[i]);

    VkFramebufferCreateInfo framebuffer_info{};
    framebuffer_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_info.renderPass      = render_pass_;
    framebuffer_info.attachmentCount = 1;
    framebuffer_info.pAttachments    = &swapchain_texture.vk_image_view;
    framebuffer_info.width           = swapchain_->vk_extent.width;
    framebuffer_info.height          = swapchain_->vk_extent.height;
    framebuffer_info.layers          = 1;

    VkResult result = vkCreateFramebuffer(device_.device_, &framebuffer_info, /*allocator=*/nullptr, &framebuffers_[i]);
    assert(result == VK_SUCCESS);
  }
}
