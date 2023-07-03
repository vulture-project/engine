/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file render_context.cpp
 * @date 2023-04-14
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

#include <vulture/renderer/render_context.hpp>

using namespace vulture;

RenderContext::RenderContext(RenderDevice& device, CommandBuffer& command_buffer, uint32_t frame_idx,
                             rg::RenderGraph& render_graph, const Camera& camera, const RenderQueue& render_queue,
                             const LightEnvironment& lights)
    : device_(device),
      command_buffer_(command_buffer),
      frame_(frame_idx),
      render_graph_(render_graph),
      camera_(camera),
      render_queue_(render_queue),
      light_environment_(lights) {}

RenderDevice&           RenderContext::GetRenderDevice()      { return device_; }
CommandBuffer&          RenderContext::GetCommandBuffer()     { return command_buffer_; }
uint32_t                RenderContext::GetFrameIdx() const    { return frame_; }

rg::RenderGraph&        RenderContext::GetRenderGraph()       { return render_graph_; }
rg::Blackboard&         RenderContext::GetBlackboard()        { return render_graph_.GetBlackboard(); }
const Camera&           RenderContext::GetCamera() const      { return camera_; }
const RenderQueue&      RenderContext::GetRenderQueue() const { return render_queue_; }
const LightEnvironment& RenderContext::GetLights() const      { return light_environment_; }