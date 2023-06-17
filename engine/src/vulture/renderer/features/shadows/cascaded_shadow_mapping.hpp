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

constexpr uint32_t kCascadedShadowMapCascadesCount = 3;

/************************************************************************************************
 * Cascaded Shadow Map Pass
 ************************************************************************************************/
struct UBCSMData {
  glm::mat4 cascade_matrices[kCascadedShadowMapCascadesCount];

  struct aligned_float {
    alignas(16) float value;
    aligned_float() = default;
    aligned_float(float value) : value(value) {}

    operator float() const { return value; }
  };

  aligned_float cascade_splits[kCascadedShadowMapCascadesCount + 1];

  alignas(16) glm::vec3 shadow_color;
  alignas(4)  uint32_t  soft_shadows;
  alignas(4)  float     bias;
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
  CascadedShadowMapPass(uint32_t cascade_num);

  static const StringView GetName() { return "Cascaded Shadow Map Pass"; }

  void Setup(rg::RenderGraphBuilder& builder, rg::Blackboard& blackboard, RenderPassId pass_id) override;

  void Execute(CommandBuffer& command_buffer, rg::Blackboard& blackboard, RenderPassId pass_id,
               RenderPassHandle handle) override;

 private:
  uint32_t cascade_num_{0};
};

/************************************************************************************************
 * Cascaded Shadow Map Render Feature
 ************************************************************************************************/
class CascadedShadowMapRenderFeature : public IRenderFeature {
 public:
  CascadedShadowMapRenderFeature(RenderDevice& device, uint32_t shadow_map_size = 4096);

  ~CascadedShadowMapRenderFeature();

  StringView Name() const override { return "Cascaded Shadow Mapping"; }

  Texture& GetShadowMap() { return *shadow_map_; }
  float& GetLogSplitContribution() { return log_split_contribution_; }
  float& GetCascadeNearOffset() { return cascade_near_offset_; }
  float& GetCascadeFarOffset() { return cascade_far_offset_; }
  glm::vec3& GetShadowColor() { return shadow_color_; }
  bool& GetUseSoftShadows() { return soft_shadows_; }
  float& GetBias() { return bias_; }
  uint32_t& GetResolution() { return shadow_map_size_; }

  void SetupRenderPasses(rg::RenderGraph& render_graph) override;
  void Execute(RenderContext& context) override;

 private:
  void CreateShadowMap();
  void OnResize(rg::RenderGraph& render_graph);

 private:
  RenderDevice&               device_;

  uint32_t                    shadow_map_size_        {0};
  float                       log_split_contribution_ {0.65f};
  float                       cascade_near_offset_    {-150.0f};
  float                       cascade_far_offset_     {0.0f};
  glm::vec3                   shadow_color_           {0.1f};
  bool                        soft_shadows_           {true};
  float                       bias_                   {0.000f};

  SharedPtr<Sampler>          shadow_map_sampler_     {nullptr};
  SharedPtr<Texture>          shadow_map_             {nullptr};

  PerFrameData<DescriptorSet> shadow_map_set_;
  PerFrameData<BufferHandle>  ub_csm_;

  PerFrameData<DescriptorSet> view_set_per_cascade_ [kCascadedShadowMapCascadesCount];
  PerFrameData<BufferHandle>  ub_view_per_cascade_  [kCascadedShadowMapCascadesCount];
};

}  // namespace vulture