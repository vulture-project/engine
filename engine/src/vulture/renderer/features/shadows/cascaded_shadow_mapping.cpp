/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file cascaded_shadow_mapping.cpp
 * @date 2023-05-25
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

#include <vulture/renderer/features/shadows/cascaded_shadow_mapping.hpp>

using namespace vulture;

/************************************************************************************************
 * Cascaded Shadow Map Pass
 ************************************************************************************************/
CascadedShadowMapPass::CascadedShadowMapPass(uint32_t cascade_num) : cascade_num_(cascade_num) {}

void CascadedShadowMapPass::Setup(rg::RenderGraphBuilder& builder, rg::Blackboard& blackboard, RenderPassId pass_id) {
  Data& data = blackboard.Get<Data>();

  data.input_depth[cascade_num_] = builder.LastVersion("cascaded_shadow_map");

  data.output_depth[cascade_num_] = builder.SetDepthStencil(data.input_depth[cascade_num_],
                                                            AttachmentLoad::kClear,
                                                            AttachmentStore::kStore,
                                                            ClearValue{1.0f, 0},
                                                            cascade_num_);
}

void CascadedShadowMapPass::Execute(CommandBuffer& command_buffer, rg::Blackboard& blackboard, RenderPassId pass_id,
                                    RenderPassHandle handle) {
  Data& data = blackboard.Get<Data>();
  Render(command_buffer, blackboard, *data.render_queue, data.view_set[cascade_num_], kInvalidRenderResourceHandle,
         pass_id, handle);
}

/************************************************************************************************
 * Cascaded Shadow Map Render Feature
 ************************************************************************************************/
CascadedShadowMapRenderFeature::CascadedShadowMapRenderFeature(RenderDevice& device, uint32_t shadow_map_size)
    : device_(device), shadow_map_size_(shadow_map_size) {
  VULTURE_ASSERT(shadow_map_size_ > 0, "Shadow map size cannot be zero, but shadow_map_size = {0}!", shadow_map_size_);

  CreateShadowMap();

  SamplerSpecification sampler_specification{};
  sampler_specification.min_filter     = SamplerFilter::kNearest;
  sampler_specification.mag_filter     = SamplerFilter::kNearest;
  sampler_specification.address_mode_u = SamplerAddressMode::kClampToBorder;
  sampler_specification.address_mode_v = SamplerAddressMode::kClampToBorder;
  sampler_specification.address_mode_w = SamplerAddressMode::kClampToBorder;
  sampler_specification.border_color   = SamplerBorderColor::kFloatOpaqueWhite;
  shadow_map_sampler_ = CreateShared<Sampler>(device_, sampler_specification);

  const ShaderStageFlags stage_flags = kShaderStageBitVertex | kShaderStageBitFragment;

  for (uint32_t frame = 0; frame < kFramesInFlight; ++frame) {
    ub_csm_[frame] = device_.CreateDynamicUniformBuffer<UBCSMData>(1);

    shadow_map_set_[frame].AddBinding(DescriptorType::kTextureSampler, stage_flags)
                          .AddBinding(DescriptorType::kUniformBuffer, stage_flags)
                          .Build(device_);

    device_.WriteDescriptorSampler(shadow_map_set_[frame].GetHandle(), 0, shadow_map_->GetHandle(),
                                   shadow_map_sampler_->GetHandle());
    device_.WriteDescriptorUniformBuffer(shadow_map_set_[frame].GetHandle(), 1, ub_csm_[frame], 0, sizeof(UBCSMData));
  }

  for (uint32_t cascade_num = 0; cascade_num < kCascadedShadowMapCascadesCount; ++cascade_num) {
    for (uint32_t frame = 0; frame < view_set_per_cascade_[cascade_num].size(); ++frame) {
      ub_view_per_cascade_[cascade_num][frame] = device_.CreateDynamicUniformBuffer<UBViewData>(1);

      view_set_per_cascade_[cascade_num][frame].AddBinding(DescriptorType::kUniformBuffer, stage_flags).Build(device_);

      device_.WriteDescriptorUniformBuffer(view_set_per_cascade_[cascade_num][frame].GetHandle(), 0,
                                           ub_view_per_cascade_[cascade_num][frame], 0, sizeof(UBViewData));
    }
  }
}

CascadedShadowMapRenderFeature::~CascadedShadowMapRenderFeature() {
  // TODO:
}

void CascadedShadowMapRenderFeature::SetupRenderPasses(rg::RenderGraph& render_graph) {
  render_graph.GetBlackboard().Add<CascadedShadowMapPass::Data>();

  render_graph.ImportTexture("cascaded_shadow_map", shadow_map_, TextureLayout::kDepthStencilReadOnly);

  for (uint32_t cascade_num = 0; cascade_num < kCascadedShadowMapCascadesCount; ++cascade_num) {
    render_graph.AddPass<CascadedShadowMapPass>(CascadedShadowMapPass::GetName(), cascade_num);
  }
}

void CascadedShadowMapRenderFeature::Execute(RenderContext& context) {
  if (shadow_map_size_ != shadow_map_->GetSpecification().width) {
    OnResize(context.GetRenderGraph());
  }

  RendererBlackboardData&      renderer_data = context.GetBlackboard().Get<RendererBlackboardData>();
  CascadedShadowMapPass::Data& pass_data     = context.GetBlackboard().Get<CascadedShadowMapPass::Data>();

  // FIXME: disable passes when there are no directional lights
  const DirectionalLight& light = context.GetLights().directional_lights[0];
  
  UBCSMData ub_csm_data{};
  UBViewData view_data_per_cascade[kCascadedShadowMapCascadesCount];

  const Camera& camera = context.GetCamera();
  float camera_near          = camera.NearPlane();
  float camera_far           = camera.FarPlane();
  float camera_clip_range    = camera_far - camera_near;
  auto  camera_proj_view     = camera.ProjMatrix() * camera.ViewMatrix();
  auto  camera_inv_proj_view = glm::inverse(camera_proj_view);

  /* Step 1. Calculate cascade splits */
  VULTURE_ASSERT(log_split_contribution_ >= 0.0f && log_split_contribution_ <= 1.0f,
                  "Log split contribution must be in range [0, 1]");

  // Based on https://developer.nvidia.com/gpugems/gpugems3/part-ii-light-and-shadows/chapter-10-parallel-split-shadow-maps-programmable-gpus
  for (uint32_t split_idx = 0; split_idx <= kCascadedShadowMapCascadesCount; ++split_idx) {
    float cur_ratio = static_cast<float>(split_idx) / static_cast<float>(kCascadedShadowMapCascadesCount);

    float split_log = camera_near * std::powf(camera_far / camera_near, cur_ratio);
    split_log *= log_split_contribution_;

    float split_uniform = camera_near + (camera_far - camera_near) * cur_ratio;
    split_uniform *= (1 - log_split_contribution_);

    ub_csm_data.cascade_splits[split_idx] = (split_log + split_uniform) / camera_clip_range;
  }

  ub_csm_data.cascade_splits[0]                               = 0.0f;
  ub_csm_data.cascade_splits[kCascadedShadowMapCascadesCount] = 1.0f;

  /* Step 2. Calculate transforms per cascade */
  for (uint32_t cascade = 0; cascade < kCascadedShadowMapCascadesCount; ++cascade) {
    glm::vec3 frustum_corners[8] = {
      glm::vec3(-1.0f,  1.0f, 0.0f),
      glm::vec3( 1.0f,  1.0f, 0.0f),
      glm::vec3( 1.0f, -1.0f, 0.0f),
      glm::vec3(-1.0f, -1.0f, 0.0f),
      
      glm::vec3(-1.0f,  1.0f, 1.0f),
      glm::vec3( 1.0f,  1.0f, 1.0f),
      glm::vec3( 1.0f, -1.0f, 1.0f),
      glm::vec3(-1.0f, -1.0f, 1.0f),
    };

    // Frustum corners from NDC to world space
    for (uint32_t i = 0; i < 8; ++i) {
      glm::vec4 corner = camera_inv_proj_view * glm::vec4(frustum_corners[i], 1.0f);
      frustum_corners[i] = corner / corner.w;
    }

    // Entire camera frustum to the current cascade frustum
    for (uint32_t i = 0; i < 4; ++i) {
      glm::vec3 dist = frustum_corners[i + 4] - frustum_corners[i];
      frustum_corners[i]     = frustum_corners[i] + (dist * ub_csm_data.cascade_splits[cascade].value);
      frustum_corners[i + 4] = frustum_corners[i] + (dist * ub_csm_data.cascade_splits[cascade + 1].value);
    }

    // Frustum center
    glm::vec3 frustum_center{0.0f};
    for (uint32_t i = 0; i < 8; ++i) {
      frustum_center += frustum_corners[i];
    }
    frustum_center /= 8.0f;

    // Frustum radius
    float radius = 0.0f;
    for (uint32_t i = 0; i < 8; ++i) {
      float distance = glm::length(frustum_corners[i] - frustum_center);
      radius = glm::max(radius, distance);
    }
    radius = std::ceil(radius * 16.0f) / 16.0f;

    // Transforms
    float     cascade_near     = 0.0f + cascade_near_offset_;
    float     cascade_far      = 2.0f * radius + cascade_far_offset_;
    glm::vec3 cascade_position = frustum_center - light.direction * radius;

    glm::mat4 view = glm::lookAt(cascade_position, frustum_center, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 proj = glm::ortho(-radius, radius, -radius, radius, cascade_near, cascade_far);

    // Get rid of shimmering (based on https://stackoverflow.com/questions/33499053/cascaded-shadow-map-shimmering)
    glm::mat4 shadow_matrix = proj * view;
    glm::vec4 shadow_origin = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    shadow_origin = shadow_matrix * shadow_origin;
    shadow_origin = shadow_origin * static_cast<float>(shadow_map_size_) / 2.0f;

    glm::vec4 rounded_origin = glm::round(shadow_origin);
    glm::vec4 round_offset = rounded_origin - shadow_origin;
    round_offset = round_offset *  2.0f / static_cast<float>(shadow_map_size_);
    round_offset.z = 0.0f;
    round_offset.w = 0.0f;

    proj[3] += round_offset;

    view_data_per_cascade[cascade].near_plane = cascade_near;
    view_data_per_cascade[cascade].far_plane  = cascade_far;
    view_data_per_cascade[cascade].position   = cascade_position;
    view_data_per_cascade[cascade].view       = view;
    view_data_per_cascade[cascade].proj       = proj;

    ub_csm_data.cascade_matrices[cascade] = proj * view;
  }

  for (uint32_t cascade = 0; cascade < kCascadedShadowMapCascadesCount; ++cascade) {
    device_.LoadBufferData<UBViewData>(ub_view_per_cascade_[cascade][context.GetFrameIdx()], 0, 1,
                                        &view_data_per_cascade[cascade]);

    pass_data.view_set[cascade] = view_set_per_cascade_[cascade][context.GetFrameIdx()].GetHandle();
    ub_csm_data.cascade_matrices[cascade] = view_data_per_cascade[cascade].proj * view_data_per_cascade[cascade].view;
  }

  ub_csm_data.shadow_color = shadow_color_;
  ub_csm_data.soft_shadows = soft_shadows_;
  ub_csm_data.bias         = bias_;

  device_.LoadBufferData<UBCSMData>(ub_csm_[context.GetFrameIdx()], 0, 1, &ub_csm_data);

  pass_data.shadow_map     = shadow_map_;
  pass_data.shadow_map_set = shadow_map_set_[context.GetFrameIdx()].GetHandle();
  pass_data.render_queue   = &context.GetRenderQueue();
}

void CascadedShadowMapRenderFeature::CreateShadowMap() {
  TextureSpecification specification{};
  specification.format                       = DataFormat::kD32_SFLOAT;
  specification.type                         = TextureType::kTexture2DArray;
  specification.usage                        = kTextureUsageBitDepthAttachment | kTextureUsageBitSampled;
  specification.width                        = shadow_map_size_;
  specification.height                       = shadow_map_size_;
  specification.array_layers                 = kCascadedShadowMapCascadesCount;
  specification.individual_layers_accessible = true;
  shadow_map_ = CreateShared<Texture>(device_, specification);
}

void CascadedShadowMapRenderFeature::OnResize(rg::RenderGraph& render_graph) {
  device_.WaitIdle();  // FIXME: make sure descriptor sets are not in use by the GPU
  CreateShadowMap();
  for (uint32_t frame = 0; frame < kFramesInFlight; ++frame) {
    device_.WriteDescriptorSampler(shadow_map_set_[frame].GetHandle(), 0, shadow_map_->GetHandle(),
                                    shadow_map_sampler_->GetHandle());
  }

  render_graph.ReimportTexture(render_graph.FirstVersion("cascaded_shadow_map"), shadow_map_);
}