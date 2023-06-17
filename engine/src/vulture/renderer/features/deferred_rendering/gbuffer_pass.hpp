/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file gbuffer_feature.hpp
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

#pragma once

#include <vulture/renderer/features/shadows/cascaded_shadow_mapping.hpp>
#include <vulture/renderer/features/render_queue_pass.hpp>
#include <vulture/renderer/render_feature.hpp>

namespace vulture {

/************************************************************************************************
 * GBuffer Pass
 ************************************************************************************************/
class GBufferPass final : public IRenderQueuePass {
 public:
  struct Data {
    rg::TextureVersionId input_color           {rg::kInvalidTextureVersionId};

    rg::TextureVersionId output_depth          {rg::kInvalidTextureVersionId};
    rg::TextureVersionId output_position       {rg::kInvalidTextureVersionId};
    rg::TextureVersionId output_normal         {rg::kInvalidTextureVersionId};
    rg::TextureVersionId output_albedo         {rg::kInvalidTextureVersionId};
    rg::TextureVersionId output_ao_metal_rough {rg::kInvalidTextureVersionId};

    const RenderQueue*   render_queue          {nullptr};
    DescriptorSetHandle  view_set              {kInvalidRenderResourceHandle};
  };

  static const StringView GetName() { return "GBuffer Pass"; }

  void Setup(rg::RenderGraphBuilder& builder, rg::Blackboard& blackboard, RenderPassId pass_id) override;

  void Execute(CommandBuffer& command_buffer, rg::Blackboard& blackboard, RenderPassId pass_id,
               RenderPassHandle handle) override;
};

}  // namespace vulture
