/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file cascaded_shadow_mapping.hpp
 * @date 2023-04-17
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

#include <vulture/renderer/features/render_queue_pass.hpp>
#include <vulture/renderer/render_feature.hpp>

namespace vulture {

constexpr uint32_t kCascadedShadowMapCascadesCount = 4;
constexpr float     kCascadeThresholds[kCascadedShadowMapCascadesCount] = {0.25f, 0.5f, 0.75f, 1.0f};

struct UBCSMData {
  glm::mat4 cascade_matrices[kCascadedShadowMapCascadesCount];
  float      cascade_splits[kCascadedShadowMapCascadesCount + 1];
};

class CascadedShadowMapPass final : public IRenderQueuePass {
 public:
  struct Data {
    rg::TextureVersionId input_depth [kCascadedShadowMapCascadesCount] {rg::kInvalidTextureVersionId};
    rg::TextureVersionId output_depth[kCascadedShadowMapCascadesCount] {rg::kInvalidTextureVersionId};
    DescriptorSetHandle  view_set    [kCascadedShadowMapCascadesCount] {kInvalidRenderResourceHandle};

    SharedPtr<Texture>   shadow_map                                    {nullptr};
    DescriptorSetHandle  shadow_map_set                                {kInvalidRenderResourceHandle};
    const RenderQueue*   render_queue                                  {nullptr};
  };

 public:
  CascadedShadowMapPass(uint32_t cascade_num) : cascade_num_(cascade_num) {}

  static const StringView GetName() { return "Cascaded Shadow Map Pass"; }

  void Setup(rg::RenderGraphBuilder& builder, rg::Blackboard& blackboard, RenderPassId pass_id) override {
    Data& data = blackboard.Get<Data>();

    data.input_depth[cascade_num_] = builder.LastVersion("cascaded_shadow_map");

    data.output_depth[cascade_num_] = builder.SetDepthStencil(data.input_depth[cascade_num_],
                                                              AttachmentLoad::kClear,
                                                              AttachmentStore::kStore,
                                                              ClearValue{1.0f, 0},
                                                              cascade_num_);
  }

  void Execute(CommandBuffer& command_buffer, rg::Blackboard& blackboard, RenderPassId pass_id,
               RenderPassHandle handle) override {
    Data& data = blackboard.Get<Data>();
    Render(command_buffer, blackboard, *data.render_queue, data.view_set[cascade_num_], kInvalidRenderResourceHandle,
           pass_id, handle);
  }

 private:
  uint32_t cascade_num_{0};
};

class CascadedShadowMapRenderFeature : public IRenderFeature {
 public:
  CascadedShadowMapRenderFeature(RenderDevice& device, uint32_t shadow_map_size = 1024)
      : device_(device), shadow_map_size_(shadow_map_size) {
    VULTURE_ASSERT(shadow_map_size_ > 0, "Shadow map size cannot be zero, but shadow_map_size = {0}!",
                   shadow_map_size_);

    TextureSpecification specification{};
    specification.format                       = DataFormat::kD32_SFLOAT;
    specification.type                         = TextureType::kTexture2DArray;
    specification.usage                        = kTextureUsageBitDepthAttachment | kTextureUsageBitSampled;
    specification.width                        = shadow_map_size_;
    specification.height                       = shadow_map_size_;
    specification.array_layers                 = kCascadedShadowMapCascadesCount;
    specification.individual_layers_accessible = true;
    shadow_map_ = CreateShared<Texture>(device_, specification);

    SamplerSpecification sampler_specification{};
    sampler_specification.min_filter      = SamplerFilter::kNearest;
    sampler_specification.mag_filter      = SamplerFilter::kNearest;
    sampler_specification.address_mode_u = SamplerAddressMode::kClampToEdge;
    sampler_specification.address_mode_v = SamplerAddressMode::kClampToEdge;
    sampler_specification.address_mode_w = SamplerAddressMode::kClampToEdge;
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

  ~CascadedShadowMapRenderFeature() {
    // TODO:
  }

  StringView Name() const override { return "Cascaded Shadow Mapping"; }

  Texture& GetShadowMap() { return *shadow_map_; }
  float& GetLogSplitContribution() { return log_split_contribution_; }
  float& GetCascadeNearOffset() { return cascade_near_offset_; }
  float& GetCascadeFarOffset() { return cascade_far_offset_; }

  void SetupRenderPasses(rg::RenderGraph& render_graph) override {
    render_graph.GetBlackboard().Add<CascadedShadowMapPass::Data>();

    render_graph.ImportTexture("cascaded_shadow_map", shadow_map_, TextureLayout::kDepthStencilReadOnly);

    for (uint32_t cascade_num = 0; cascade_num < kCascadedShadowMapCascadesCount; ++cascade_num) {
      render_graph.AddPass<CascadedShadowMapPass>(CascadedShadowMapPass::GetName(), cascade_num);
    }
  }

  void Execute(RenderContext& context) override {
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
    auto camera_proj_view     = camera.ProjMatrix() * camera.ViewMatrix();
    auto camera_inv_proj_view = glm::inverse(camera_proj_view);

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
        frustum_corners[i]     = frustum_corners[i] + (dist * ub_csm_data.cascade_splits[cascade]);
        frustum_corners[i + 4] = frustum_corners[i] + (dist * ub_csm_data.cascade_splits[cascade + 1]);
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
      float      cascade_near     = 0.0f + cascade_near_offset_;
      float      cascade_far      = 2.0f * radius + cascade_far_offset_;
      glm::vec3 cascade_position = frustum_center - light.direction * radius;

      glm::mat4 view = glm::lookAt(cascade_position, frustum_center, glm::vec3(0.0f, 1.0f, 0.0f));
      glm::mat4 proj = glm::orthoLH(-radius, radius, -radius, radius, cascade_near, cascade_far);

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

    device_.LoadBufferData<UBCSMData>(ub_csm_[context.GetFrameIdx()], 0, 1, &ub_csm_data);

    pass_data.shadow_map     = shadow_map_;
    pass_data.shadow_map_set = shadow_map_set_[context.GetFrameIdx()].GetHandle();
    pass_data.render_queue   = &context.GetRenderQueue();

    // /* HAZEL */
    // auto viewProjection = camera.ProjMatrix() * camera.ViewMatrix();

    // float cascadeSplits[kCascadedShadowMapCascadesCount];

		// float nearClip = camera.NearPlane();
		// float farClip = camera.FarPlane();
		// float clipRange = farClip - nearClip;

		// float minZ = nearClip;
		// float maxZ = nearClip + clipRange;

		// float range = maxZ - minZ;
		// float ratio = maxZ / minZ;

		// // Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
		// for (uint32_t i = 0; i < kCascadedShadowMapCascadesCount; i++)
		// {
		// 	float p = (i + 1) / static_cast<float>(kCascadedShadowMapCascadesCount);
		// 	float log = minZ * std::pow(ratio, p);
		// 	float uniform = minZ + range * p;
		// 	float d = 0.92f * (log - uniform) + uniform;
		// 	cascadeSplits[i] = (d - nearClip) / clipRange;
		// }

    // cascadeSplits[0] = kCascadeThresholds[0];
		// cascadeSplits[1] = kCascadeThresholds[1];
		// cascadeSplits[2] = kCascadeThresholds[2];
		// cascadeSplits[3] = kCascadeThresholds[3];

    // // Calculate orthographic projection matrix for each cascade
		// float lastSplitDist = 0.0;
		// for (uint32_t i = 0; i < kCascadedShadowMapCascadesCount; i++)
		// {
		// 	float splitDist = cascadeSplits[i];

		// 	glm::vec3 frustumCorners[8] =
		// 	{
		// 		glm::vec3(-1.0f,  1.0f, -1.0f),
		// 		glm::vec3(1.0f,  1.0f, -1.0f),
		// 		glm::vec3(1.0f, -1.0f, -1.0f),
		// 		glm::vec3(-1.0f, -1.0f, -1.0f),
		// 		glm::vec3(-1.0f,  1.0f,  1.0f),
		// 		glm::vec3(1.0f,  1.0f,  1.0f),
		// 		glm::vec3(1.0f, -1.0f,  1.0f),
		// 		glm::vec3(-1.0f, -1.0f,  1.0f),
		// 	};

		// 	// Project frustum corners into world space
		// 	glm::mat4 invCam = glm::inverse(viewProjection);
		// 	for (uint32_t i = 0; i < 8; i++)
		// 	{
		// 		glm::vec4 invCorner = invCam * glm::vec4(frustumCorners[i], 1.0f);
		// 		frustumCorners[i] = invCorner / invCorner.w;
		// 	}

		// 	for (uint32_t i = 0; i < 4; i++)
		// 	{
		// 		glm::vec3 dist = frustumCorners[i + 4] - frustumCorners[i];
		// 		frustumCorners[i + 4] = frustumCorners[i] + (dist * splitDist);
		// 		frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
		// 	}

		// 	// Get frustum center
		// 	glm::vec3 frustumCenter = glm::vec3(0.0f);
		// 	for (uint32_t i = 0; i < 8; i++)
		// 		frustumCenter += frustumCorners[i];

		// 	frustumCenter /= 8.0f;

		// 	//frustumCenter *= 0.01f;

		// 	float radius = 0.0f;
		// 	for (uint32_t i = 0; i < 8; i++)
		// 	{
		// 		float distance = glm::length(frustumCorners[i] - frustumCenter);
		// 		radius = glm::max(radius, distance);
		// 	}
		// 	radius = 5.0f * std::ceil(radius * 16.0f) / 16.0f;

		// 	glm::vec3 maxExtents = glm::vec3(radius);
		// 	glm::vec3 minExtents = -maxExtents;

		// 	glm::vec3 lightDir = -light.direction;
		// 	glm::mat4 lightViewMatrix = glm::lookAt(frustumCenter - lightDir * -minExtents.z, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));
		// 	glm::mat4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f + -150.0f, maxExtents.z - minExtents.z + 300.0f);

    //   // lightOrthoMatrix[3] += vec3(lightDir);

		// 	// Offset to texel space to avoid shimmering (from https://stackoverflow.com/questions/33499053/cascaded-shadow-map-shimmering)
		// 	glm::mat4 shadowMatrix = lightOrthoMatrix * lightViewMatrix;
		// 	float ShadowMapResolution = (float)shadow_map_size_;
		// 	glm::vec4 shadowOrigin = (shadowMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)) * ShadowMapResolution / 2.0f;
		// 	glm::vec4 roundedOrigin = glm::round(shadowOrigin);
		// 	glm::vec4 roundOffset = roundedOrigin - shadowOrigin;
		// 	roundOffset = roundOffset * 2.0f / ShadowMapResolution;
		// 	roundOffset.z = 0.0f;
		// 	roundOffset.w = 0.0f;

		// 	lightOrthoMatrix[3] += roundOffset;

		// 	// Store split distance and matrix in cascade
    //   view_data_per_cascade[i].view = lightViewMatrix;
    //   view_data_per_cascade[i].proj = lightOrthoMatrix;
    //   view_data_per_cascade[i].position = frustumCenter - lightDir * -minExtents.z;
    //   // view_data_per_cascade[i].

		// 	// cascades[i].SplitDepth = (nearClip + splitDist * clipRange) * -1.0f;
		// 	// cascades[i].ViewProj = lightOrthoMatrix * lightViewMatrix;
		// 	// cascades[i].View = lightViewMatrix;

		// 	lastSplitDist = cascadeSplits[i];
		// }

    // /* HAZEL */






    // for (uint32_t cascade = 0; cascade < kCascadedShadowMapCascadesCount; ++cascade) {
    //   float near = (cascade == 0) ? camera.NearPlane() : camera.NearPlane() * kCascadeThresholds[cascade - 1];
    //   float far  = camera.FarPlane() * kCascadeThresholds[cascade];

    //   // UBViewData view_data = CalculateCascadeViewData(camera, light, near, far);
    //   device_.LoadBufferData<UBViewData>(ub_view_per_cascade_[cascade][context.GetFrameIdx()], 0, 1, &view_data_per_cascade[cascade]);

    //   pass_data.view_set[cascade] = view_set_per_cascade_[cascade][context.GetFrameIdx()].GetHandle();
    //   ub_csm_data.cascade_matrices[cascade] = view_data_per_cascade[cascade].proj * view_data_per_cascade[cascade].view;
    // }
  }

 private:
  RenderDevice&               device_;

  uint32_t                    shadow_map_size_        {0};
  float                        log_split_contribution_ {0.5f};
  float                        cascade_near_offset_    {-50.0f};
  float                        cascade_far_offset_     {50.0f};

  SharedPtr<Sampler>          shadow_map_sampler_     {nullptr};
  SharedPtr<Texture>          shadow_map_             {nullptr};

  PerFrameData<DescriptorSet> shadow_map_set_;
  PerFrameData<BufferHandle>  ub_csm_;

  PerFrameData<DescriptorSet> view_set_per_cascade_ [kCascadedShadowMapCascadesCount];
  PerFrameData<BufferHandle>  ub_view_per_cascade_  [kCascadedShadowMapCascadesCount];
};

}  // namespace vulture