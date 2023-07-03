/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file render_queue_pass.cpp
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

#include <vulture/renderer/features/render_queue_pass.hpp>

using namespace vulture;

void IRenderQueuePass::Render(CommandBuffer& command_buffer, rg::Blackboard& blackboard, const RenderQueue& queue,
                              DescriptorSetHandle view_set, DescriptorSetHandle custom_set, RenderPassId id,
                              RenderPassHandle handle) {
  RendererBlackboardData& renderer_data = blackboard.Get<RendererBlackboardData>();
  
  PipelineHandle pipeline      = kInvalidRenderResourceHandle;
  Material*      prev_material = nullptr;

  for (auto& render_object : queue.renderables) {
    Mesh& mesh = *render_object.mesh.get(); 
    const glm::mat4& model_matrix = render_object.model_matrix;

    for (auto& submesh : mesh.GetSubmeshes()) {
      Material& material = submesh.GetMaterial();
      if (!material.Has(id)) {
        continue;
      }

      MaterialPass& material_pass = material.GetMaterialPass(id);
      Shader&       shader        = material_pass.GetShader();

      if (!shader.IsBuilt()) {
        shader.Build(handle);
      }

      if (!ValidRenderHandle(pipeline) || pipeline != shader.GetPipeline()) {
        pipeline = shader.GetPipeline();
      }

      // TODO: (tralf-strues) not bind per submesh!
      command_buffer.CmdBindGraphicsPipeline(pipeline);
      shader.BindDescriptorSetIfUsed(command_buffer, Shader::kFrameSetBit, renderer_data.descriptor_set_frame);
      shader.BindDescriptorSetIfUsed(command_buffer, Shader::kViewSetBit,  view_set);
      shader.BindDescriptorSetIfUsed(command_buffer, Shader::kSceneSetBit, renderer_data.descriptor_set_scene);

      if ((prev_material == nullptr || prev_material != &material) && material_pass.IsMaterialUsed()) {
        shader.BindDescriptorSetIfUsed(command_buffer, Shader::kMaterialSetBit, material_pass.GetDescriptorSet());
        prev_material = &material;

        if (ValidRenderHandle(custom_set)) {
          shader.BindDescriptorSetIfUsed(command_buffer, Shader::kCustomSetBit, custom_set);
        }
      }

      command_buffer.CmdPushConstants(pipeline, &model_matrix, 0, sizeof(model_matrix), kShaderStageBitVertex);

      command_buffer.CmdBindVertexBuffer(0, submesh.GetVertexBuffer());
      command_buffer.CmdBindIndexBuffer(submesh.GetIndexBuffer());
      command_buffer.CmdDrawIndexed(submesh.GetGeometry().GetIndices().size());
    }
  }
}