/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file gbuffer_feature.cpp
 * @date 2023-06-12
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

#include <vulture/renderer/features/deferred_rendering/gbuffer_pass.hpp>

using namespace vulture;

/************************************************************************************************
 * GBuffer Pass
 ************************************************************************************************/
void GBufferPass::Setup(rg::RenderGraphBuilder& builder, rg::Blackboard& blackboard, RenderPassId pass_id) {
  Data& data = blackboard.Get<Data>();

  data.input_color = builder.LastVersion("backbuffer");

  auto backbuffer_id = data.input_color;

  rg::DynamicTextureSpecification depth_specification{};
  depth_specification.format = DataFormat::kD32_SFLOAT;
  depth_specification.usage = kTextureUsageBitDepthAttachment;
  depth_specification.width.SetDependency(backbuffer_id);
  depth_specification.height.SetDependency(backbuffer_id);
  auto gbuffer_depth = builder.CreateTexture("gbuffer_depth", depth_specification);

  rg::DynamicTextureSpecification position_specification{};
  position_specification.format = DataFormat::kR16G16B16_SFLOAT;
  position_specification.usage = kTextureUsageBitColorAttachment;
  position_specification.width.SetDependency(backbuffer_id);
  position_specification.height.SetDependency(backbuffer_id);
  auto gbuffer_position = builder.CreateTexture("gbuffer_position", position_specification);

  rg::DynamicTextureSpecification normal_specification{};
  normal_specification.format = DataFormat::kR16G16B16_SFLOAT;
  normal_specification.usage = kTextureUsageBitColorAttachment;
  normal_specification.width.SetDependency(backbuffer_id);
  normal_specification.height.SetDependency(backbuffer_id);
  auto gbuffer_normal = builder.CreateTexture("gbuffer_normal", normal_specification);

  rg::DynamicTextureSpecification albedo_specification{};
  albedo_specification.format = DataFormat::kR8G8B8A8_UNORM;
  albedo_specification.usage = kTextureUsageBitColorAttachment;
  albedo_specification.width.SetDependency(backbuffer_id);
  albedo_specification.height.SetDependency(backbuffer_id);
  auto gbuffer_albedo = builder.CreateTexture("gbuffer_albedo", albedo_specification);

  rg::DynamicTextureSpecification ao_metal_rough_specification{};
  ao_metal_rough_specification.format = DataFormat::kR8G8B8_UNORM;
  ao_metal_rough_specification.usage = kTextureUsageBitColorAttachment;
  ao_metal_rough_specification.width.SetDependency(backbuffer_id);
  ao_metal_rough_specification.height.SetDependency(backbuffer_id);
  auto gbuffer_ao_metal_rough = builder.CreateTexture("gbuffer_ao_metal_rough", ao_metal_rough_specification);

  data.output_depth          = builder.SetDepthStencil(gbuffer_depth, AttachmentLoad::kClear, AttachmentStore::kStore);
  data.output_position       = builder.AddColorAttachment(gbuffer_position);
  data.output_normal         = builder.AddColorAttachment(gbuffer_normal);
  data.output_albedo         = builder.AddColorAttachment(gbuffer_albedo);
  data.output_ao_metal_rough = builder.AddColorAttachment(gbuffer_ao_metal_rough);
}

void GBufferPass::Execute(CommandBuffer& command_buffer, rg::Blackboard& blackboard, RenderPassId pass_id,
                          RenderPassHandle handle) {
  const auto& data = blackboard.Get<Data>();

  Render(command_buffer, blackboard, *data.render_queue, data.view_set, kInvalidRenderResourceHandle, pass_id, handle);
}