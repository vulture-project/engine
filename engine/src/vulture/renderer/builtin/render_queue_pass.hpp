/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file render_queue_pass.hpp
 * @date 2023-03-20
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

#include <vulture/renderer/builtin/render_pass_data.hpp>
#include <vulture/renderer/geometry/mesh.hpp>
#include <vulture/renderer/render_graph/render_graph.hpp>

namespace vulture {

struct RenderQueueObject {
  SharedPtr<Mesh> mesh;
  glm::mat4 model_matrix;
};

template<typename QueueTagT>
struct RenderQueue {
  Vector<RenderQueueObject> render_objects;
};

template<typename QueueTagT>
class IRenderQueuePass : public rg::IRenderPass {
 public:
  void Execute(CommandBuffer& command_buffer, rg::Blackboard& blackboard, RenderPassId id,
               RenderPassHandle handle) override final {
    FrameData&              frame_data   = blackboard.Get<FrameData>();
    ViewData&               view_data    = blackboard.Get<ViewData>();
    LightEnvironmentData&   light_data   = blackboard.Get<LightEnvironmentData>();
    RenderQueue<QueueTagT>& render_queue = blackboard.Get<RenderQueue<QueueTagT>>();
    
    PipelineHandle pipeline      = kInvalidRenderResourceHandle;
    Material*      prev_material = nullptr;

    for (auto& render_object : render_queue.render_objects) {
      Mesh&      mesh         = *render_object.mesh.get(); 
      glm::mat4& model_matrix = render_object.model_matrix;

      for (auto& submesh : mesh.GetSubmeshes()) {
        Material&     material      = submesh.GetMaterial();
        MaterialPass& material_pass = material.GetMaterialPass(id);
        Shader&       shader        = material_pass.GetShader();

        if (!shader.IsBuilt()) {
          shader.Build(handle);
        }

        if (!ValidRenderHandle(pipeline) || pipeline != shader.GetPipeline()) {
          pipeline = shader.GetPipeline();

          command_buffer.CmdBindGraphicsPipeline(pipeline);
          command_buffer.CmdBindDescriptorSet(pipeline, kFrameDescriptorSetIdx, frame_data.set_handle);
          command_buffer.CmdBindDescriptorSet(pipeline, kViewDescriptorSetIdx,  view_data.set_handle);
          command_buffer.CmdBindDescriptorSet(pipeline, kSceneDescriptorSetIdx, light_data.set_handle);
        }

        if (prev_material == nullptr || prev_material != &material) {
          // TODO: (tralf-strues) only if changed
          DescriptorSetHandle material_set = material_pass.WriteDescriptorSet();
          command_buffer.CmdBindDescriptorSet(pipeline, kMaterialDescriptorSetIdx, material_set);

          prev_material = &material;
        }

        command_buffer.CmdPushConstants(pipeline, &model_matrix, 0, sizeof(model_matrix), kShaderStageBitVertex);

        command_buffer.CmdBindVertexBuffer(0, submesh.GetVertexBuffer());
        command_buffer.CmdBindIndexBuffer(submesh.GetIndexBuffer());
        command_buffer.CmdDrawIndexed(submesh.GetGeometry().GetIndices().size());
      }
    }
  }
};


}  // namespace vulture