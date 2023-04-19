/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file render_context.hpp
 * @date 2023-04-12
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

#include <vulture/renderer/camera.hpp>
#include <vulture/renderer/light.hpp>
#include <vulture/renderer/render_graph/render_graph.hpp>
#include <vulture/renderer/render_queue.hpp>

namespace vulture {

class RenderContext {
 public:
  RenderContext(RenderDevice& device, CommandBuffer& command_buffer, uint32_t frame_idx, rg::RenderGraph& render_graph,
                const Camera& camera, const RenderQueue& render_queue, const LightEnvironment& lights);

  RenderDevice&           GetRenderDevice();
  CommandBuffer&          GetCommandBuffer();
  uint32_t                GetFrameIdx() const;

  rg::RenderGraph&        GetRenderGraph();
  rg::Blackboard&         GetBlackboard();
  const Camera&           GetCamera() const;
  const RenderQueue&      GetRenderQueue() const;
  const LightEnvironment& GetLights() const;

 private:
  RenderDevice&           device_;
  CommandBuffer&          command_buffer_;
  uint32_t                frame_{0};

  rg::RenderGraph&        render_graph_;
  const Camera&           camera_;
  const RenderQueue&      render_queue_;
  const LightEnvironment& light_environment_;
};

}  // namespace vulture