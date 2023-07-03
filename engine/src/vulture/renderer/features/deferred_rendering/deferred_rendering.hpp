/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file deferred_rendering.hpp
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

#pragma once

#include <vulture/renderer/features/shadows/cascaded_shadow_mapping.hpp>
#include <vulture/renderer/features/render_queue_pass.hpp>
#include <vulture/renderer/render_feature.hpp>

namespace vulture {

/************************************************************************************************
 * Deferred Pass
 ************************************************************************************************/
class DeferredPass final : public rg::IRenderPass {
 public:
  struct Data {
    rg::TextureVersionId input_color   {rg::kInvalidTextureVersionId};
    rg::TextureVersionId output_color  {rg::kInvalidTextureVersionId};

    DescriptorSetHandle  view_set      {kInvalidRenderResourceHandle};

    MaterialPass*        material_pass {nullptr};
  };

  static const StringView GetName() { return "Deferred Pass"; }

  void Setup(rg::RenderGraphBuilder& builder, rg::Blackboard& blackboard, RenderPassId pass_id) override;

  void Execute(CommandBuffer& command_buffer, rg::Blackboard& blackboard, RenderPassId pass_id,
               RenderPassHandle handle) override;
};

/************************************************************************************************
 * Deferred Render Feature
 ************************************************************************************************/
class DeferredRenderFeature : public IRenderFeature {
 public:
  StringView Name() const override { return "Deferred Rendering"; }

  void SetupRenderPasses(rg::RenderGraph& render_graph) override;

  void Execute(RenderContext& context) override;

 private:
  UniquePtr<MaterialPass> material_pass_{nullptr};
};

}  // namespace vulture