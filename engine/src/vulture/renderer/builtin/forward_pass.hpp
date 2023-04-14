/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file forward_pass.hpp
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

#include <vulture/renderer/builtin/render_queue_pass.hpp>

namespace vulture {

class ForwardPass final : public IRenderQueuePass<MainQueueTag> {
 public:
  static const StringView GetName() { return "Forward Pass"; }

  void Setup(rg::RenderGraphBuilder& builder, rg::Blackboard& blackboard, RenderPassId pass_id) override {
    ColorOutput& color_output = blackboard.Get<ColorOutput>();
    DepthBuffer& depth_buffer = blackboard.Add<DepthBuffer>();

    rg::DynamicTextureSpecification depth_specification{};
    depth_specification.format = DataFormat::kD32_SFLOAT;
    depth_specification.usage = kTextureUsageBitDepthAttachment;
    depth_specification.width.SetDependency(color_output.texture_id);
    depth_specification.height.SetDependency(color_output.texture_id);
    depth_buffer.texture_id = builder.CreateTexture("depth_buffer", depth_specification);

    builder.SetDepthStencil(depth_buffer.texture_id, AttachmentLoad::kClear, AttachmentStore::kStore);
    builder.AddColorAttachment(color_output.texture_id, AttachmentLoad::kClear, AttachmentStore::kStore);
  }
};

}  // namespace vulture
