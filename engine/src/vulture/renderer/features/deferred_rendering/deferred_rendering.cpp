/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file deferred_rendering.cpp
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

#include <vulture/renderer/features/deferred_rendering/deferred_rendering.hpp>
#include <vulture/renderer/features/deferred_rendering/gbuffer_pass.hpp>
#include <vulture/asset/asset_registry.hpp>

using namespace vulture;

/************************************************************************************************
 * Deferred Pass
 ************************************************************************************************/
void DeferredPass::Setup(rg::RenderGraphBuilder& builder, rg::Blackboard& blackboard, RenderPassId pass_id) {
  Data& data = blackboard.Get<Data>();
  GBufferPass::Data& gbuffer = blackboard.Get<GBufferPass::Data>();

  data.input_color = builder.LastVersion("backbuffer");

  data.output_color = builder.AddColorAttachment(data.input_color, AttachmentLoad::kLoad, AttachmentStore::kStore);
  builder.AddSampledTexture(gbuffer.output_position);
  builder.AddSampledTexture(gbuffer.output_normal);
  builder.AddSampledTexture(gbuffer.output_albedo);
  builder.AddSampledTexture(gbuffer.output_ao_metal_rough);
}

void DeferredPass::Execute(CommandBuffer& command_buffer, rg::Blackboard& blackboard, RenderPassId pass_id,
                           RenderPassHandle handle) {
  const auto& data        = blackboard.Get<Data>();
  const auto& shadow_data = blackboard.Get<CascadedShadowMapPass::Data>();

  auto& renderer_data = blackboard.Get<RendererBlackboardData>();

  auto& shader = data.material_pass->GetShader();

  if (!shader.IsBuilt()) {
    shader.Build(handle);
  }

  auto pipeline = shader.GetPipeline();

  command_buffer.CmdBindGraphicsPipeline(pipeline);
  shader.BindDescriptorSetIfUsed(command_buffer, Shader::kFrameSetBit, renderer_data.descriptor_set_frame);
  shader.BindDescriptorSetIfUsed(command_buffer, Shader::kViewSetBit,  data.view_set);
  shader.BindDescriptorSetIfUsed(command_buffer, Shader::kSceneSetBit, renderer_data.descriptor_set_scene);

  if (data.material_pass->IsMaterialUsed()) {
    shader.BindDescriptorSetIfUsed(command_buffer, Shader::kMaterialSetBit, data.material_pass->GetDescriptorSet());

    if (ValidRenderHandle(shadow_data.shadow_map_set)) {
      shader.BindDescriptorSetIfUsed(command_buffer, Shader::kCustomSetBit, shadow_data.shadow_map_set);
    }
  }

  command_buffer.CmdDraw(6);
}

/************************************************************************************************
 * Deferred Render Feature
 ************************************************************************************************/
void DeferredRenderFeature::SetupRenderPasses(rg::RenderGraph& render_graph) {
  render_graph.AddPass<GBufferPass::Data, GBufferPass>(GBufferPass::GetName());
  render_graph.AddPass<DeferredPass::Data, DeferredPass>(DeferredPass::GetName());
}

void DeferredRenderFeature::Execute(RenderContext& context) {
  RendererBlackboardData& renderer_data = context.GetBlackboard().Get<RendererBlackboardData>();

  /* GBuffer Pass */
  GBufferPass::Data& gbuffer_pass_data = context.GetBlackboard().Get<GBufferPass::Data>();

  gbuffer_pass_data.render_queue = &context.GetRenderQueue();
  gbuffer_pass_data.view_set     = renderer_data.descriptor_set_main_view;

  context.GetRenderGraph().ReimportTexture(gbuffer_pass_data.input_color, context.GetCamera().render_texture);

  /* Deferred Pass */
  if (!material_pass_) {
    AssetRegistry* asset_registry = AssetRegistry::Instance();
    SharedPtr<Shader> deferred_shader = asset_registry->Load<Shader>(".vulture/shaders/BuiltIn.Deferred.shader");

    material_pass_ = CreateUnique<MaterialPass>(context.GetRenderDevice(), deferred_shader);
  }

  material_pass_->SetTextureSampler("uGBuffer_Position", context.GetRenderGraph().GetTexture(gbuffer_pass_data.output_position));
  material_pass_->SetTextureSampler("uGBuffer_Normal", context.GetRenderGraph().GetTexture(gbuffer_pass_data.output_normal));
  material_pass_->SetTextureSampler("uGBuffer_Albedo", context.GetRenderGraph().GetTexture(gbuffer_pass_data.output_albedo));
  material_pass_->SetTextureSampler("uGBuffer_AO_Metal_Rough", context.GetRenderGraph().GetTexture(gbuffer_pass_data.output_ao_metal_rough));
  material_pass_->WriteDescriptorSet();

  DeferredPass::Data& deferred_pass_data = context.GetBlackboard().Get<DeferredPass::Data>();
  deferred_pass_data.material_pass = material_pass_.get();
  deferred_pass_data.view_set = renderer_data.descriptor_set_main_view;

  context.GetRenderGraph().ReimportTexture(deferred_pass_data.input_color, context.GetCamera().render_texture);
}