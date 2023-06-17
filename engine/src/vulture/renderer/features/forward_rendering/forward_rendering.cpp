/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file forward_rendering.cpp
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

#include <vulture/renderer/features/forward_rendering/forward_rendering.hpp>

using namespace vulture;

/************************************************************************************************
 * Forward Pass
 ************************************************************************************************/
void ForwardPass::Setup(rg::RenderGraphBuilder& builder, rg::Blackboard& blackboard, RenderPassId pass_id) {
  Data& data = blackboard.Get<Data>();

  data.input_color = builder.LastVersion("backbuffer");

  rg::DynamicTextureSpecification depth_specification{};
  depth_specification.format = DataFormat::kD32_SFLOAT;
  depth_specification.usage = kTextureUsageBitDepthAttachment;
  depth_specification.width.SetDependency(data.input_color);
  depth_specification.height.SetDependency(data.input_color);
  depth_specification.samples = 4;
  data.input_depth = builder.CreateTexture("forward_depth", depth_specification);

  rg::DynamicTextureSpecification color_specification{};
  color_specification.format.SetDependency(data.input_color);
  color_specification.usage = kTextureUsageBitColorAttachment;
  color_specification.width.SetDependency(data.input_color);
  color_specification.height.SetDependency(data.input_color);
  color_specification.samples = 4;
  rg::TextureVersionId color = builder.CreateTexture("forward_color", color_specification);

  builder.AddColorAttachment(color, AttachmentLoad::kClear, AttachmentStore::kDontCare);
  data.output_color = builder.AddResolveAttachment(data.input_color, AttachmentLoad::kClear, AttachmentStore::kStore);
  data.output_depth = builder.SetDepthStencil(data.input_depth, AttachmentLoad::kClear, AttachmentStore::kStore);
}

void ForwardPass::Execute(CommandBuffer& command_buffer, rg::Blackboard& blackboard, RenderPassId pass_id,
                          RenderPassHandle handle) {
  const auto& data        = blackboard.Get<Data>();
  const auto& shadow_data = blackboard.Get<CascadedShadowMapPass::Data>();

  Render(command_buffer, blackboard, *data.render_queue, data.view_set, shadow_data.shadow_map_set, pass_id, handle);
}

/************************************************************************************************
 * Forward Render Feature
 ************************************************************************************************/
void ForwardRenderFeature::SetupRenderPasses(rg::RenderGraph& render_graph) {
  render_graph.AddPass<ForwardPass::Data, ForwardPass>(ForwardPass::GetName());
}

void ForwardRenderFeature::Execute(RenderContext& context) {
  RendererBlackboardData& renderer_data     = context.GetBlackboard().Get<RendererBlackboardData>();
  ForwardPass::Data&      forward_pass_data = context.GetBlackboard().Get<ForwardPass::Data>();

  forward_pass_data.render_queue = &context.GetRenderQueue();
  forward_pass_data.view_set     = renderer_data.descriptor_set_main_view;

  context.GetRenderGraph().ReimportTexture(forward_pass_data.input_color, context.GetCamera().render_texture);
}