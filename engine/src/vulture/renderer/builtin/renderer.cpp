/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file renderer.cpp
 * @date 2023-03-21
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

#include <vulture/renderer/builtin/renderer.hpp>

using namespace vulture;

Renderer::Renderer(RenderDevice& device, UniquePtr<rg::RenderGraph> render_graph)
    : device_(device), render_graph_(std::move(render_graph)) {
  // CreateCommandBuffers();
  CreateDescriptorSets();
  CreateBuffers();

  auto& blackboard = GetBlackboard();
  blackboard.Add<ColorOutput>();
  blackboard.Add<FrameData>();
  blackboard.Add<ViewData>();
  blackboard.Add<LightEnvironmentData>();
  blackboard.Add<RenderQueue<MainQueueTag>>();
}

Renderer::~Renderer() {
  for (uint32_t i = 0; i < kFramesInFlight; ++i) {
    if (ValidRenderHandle(ub_frame_[i])) {
      device_.DeleteBuffer(ub_frame_[i]);
    }

    if (ValidRenderHandle(ub_view_[i])) {
      device_.DeleteBuffer(ub_view_[i]);
    }

    if (ValidRenderHandle(ub_light_[i])) {
      device_.DeleteBuffer(ub_light_[i]);
    }

    if (ValidRenderHandle(sb_directional_lights_[i])) {
      device_.DeleteBuffer(sb_directional_lights_[i]);
    }

    if (ValidRenderHandle(sb_point_lights_[i])) {
      device_.DeleteBuffer(sb_point_lights_[i]);
    }

    if (ValidRenderHandle(sb_spot_lights_[i])) {
      device_.DeleteBuffer(sb_spot_lights_[i]);
    }
  }
}

void Renderer::Render(CommandBuffer& command_buffer, uint32_t current_frame) {
  WriteBlackboard(current_frame);
  WriteDescriptors(current_frame);

  {
    ScopedTimer trace_timer{"render_graph.Execute()"};

    // command_buffer.Reset();
    // command_buffer.Begin();

    render_graph_->Execute(device_, command_buffer);

    // command_buffer.End();
    // command_buffer.Submit();
  }
}

RenderDevice& Renderer::GetDevice() { return device_; }
rg::RenderGraph& Renderer::GetRenderGraph() { return *render_graph_; }
rg::Blackboard& Renderer::GetBlackboard() { return render_graph_->GetBlackboard(); }

void Renderer::UpdateFrameData(float time) {
  ub_frame_data_.time = time;
}

void Renderer::UpdateViewData(const glm::mat4& view, const glm::mat4& proj, glm::vec3 position) {
  ub_view_data_.view     = view;
  ub_view_data_.proj     = proj;
  ub_view_data_.position = position;
}

LightEnvironment& Renderer::GetLightEnvironment() { return light_environment_; }

// void Renderer::CreateCommandBuffers() {
//   for (auto& command_buffer : command_buffer_) {
//     command_buffer = device_.CreateCommandBuffer(CommandBufferType::kGraphics, false);
//   }
// }

void Renderer::CreateDescriptorSets() {
  for (uint32_t i = 0; i < kFramesInFlight; ++i) {
    /* Frame Set */
    frame_set_[i].SetRenderDevice(&device_);
    frame_set_[i].AddBinding(DescriptorType::kUniformBuffer, kShaderStageBitVertex | kShaderStageBitFragment).Build();

    /* View Set */
    view_set_[i].SetRenderDevice(&device_);
    view_set_[i].AddBinding(DescriptorType::kUniformBuffer, kShaderStageBitVertex | kShaderStageBitFragment).Build();

    /* Scene Set */
    scene_set_[i].SetRenderDevice(&device_);
    scene_set_[i].AddBinding(DescriptorType::kUniformBuffer, kShaderStageBitFragment)
                 .AddBinding(DescriptorType::kStorageBuffer, kShaderStageBitFragment)
                 .AddBinding(DescriptorType::kStorageBuffer, kShaderStageBitFragment)
                 .AddBinding(DescriptorType::kStorageBuffer, kShaderStageBitFragment)
                 .Build();
  }
}

void Renderer::CreateBuffers() {
  for (uint32_t i = 0; i < kFramesInFlight; ++i) {
    /* Frame */
    ub_frame_[i] = device_.CreateDynamicUniformBuffer<UBFrameData>(1);

    /* View */
    ub_view_[i] = device_.CreateDynamicUniformBuffer<UBViewData>(1);

    /* Scene */
    ub_light_[i] = device_.CreateDynamicUniformBuffer<UBLightEnvironmentData>(1);

    sb_directional_lights_[i] = device_.CreateDynamicStorageBuffer<DirectionalLight>(kMaxDirectionalLights);
    sb_point_lights_[i]       = device_.CreateDynamicStorageBuffer<PointLight>(kMaxPointLights);
    sb_spot_lights_[i]        = device_.CreateDynamicStorageBuffer<SpotLight>(kMaxSpotLights);
  }
}

void Renderer::WriteBlackboard(uint32_t current_frame) {
  auto& blackboard = GetBlackboard();

  blackboard.Get<FrameData>().set_handle            = frame_set_[current_frame].GetHandle();
  blackboard.Get<ViewData>().set_handle             = view_set_[current_frame].GetHandle();
  blackboard.Get<LightEnvironmentData>().set_handle = scene_set_[current_frame].GetHandle();
}

void Renderer::WriteDescriptors(uint32_t current_frame) {
  /* Frame */
  device_.LoadBufferData<UBFrameData>(ub_frame_[current_frame], 0, 1, &ub_frame_data_);
  device_.WriteDescriptorUniformBuffer(frame_set_[current_frame].GetHandle(), 0, ub_frame_[current_frame], 0,
                                       sizeof(ub_frame_data_));

  /* View */
  device_.LoadBufferData<UBViewData>(ub_view_[current_frame], 0, 1, &ub_view_data_);
  device_.WriteDescriptorUniformBuffer(view_set_[current_frame].GetHandle(), 0, ub_view_[current_frame], 0,
                                       sizeof(ub_view_data_));

  /* Scene */
  ub_light_data_.directional_lights_count = light_environment_.directional_lights.size();
  ub_light_data_.point_lights_count       = light_environment_.point_lights.size();
  ub_light_data_.spot_lights_count        = light_environment_.spot_lights.size();

  device_.LoadBufferData<UBLightEnvironmentData>(ub_light_[current_frame], 0, 1, &ub_light_data_);
  device_.WriteDescriptorUniformBuffer(scene_set_[current_frame].GetHandle(), 0, ub_light_[current_frame], 0,
                                       sizeof(ub_light_data_));

  device_.LoadBufferData<DirectionalLight>(sb_directional_lights_[current_frame], 0,
                                           light_environment_.directional_lights.size(),
                                           light_environment_.directional_lights.data());
  device_.WriteDescriptorStorageBuffer(scene_set_[current_frame].GetHandle(), 1, sb_directional_lights_[current_frame],
                                       0, light_environment_.directional_lights.size() * sizeof(DirectionalLight));

  device_.LoadBufferData<PointLight>(sb_point_lights_[current_frame], 0, light_environment_.point_lights.size(),
                                     light_environment_.point_lights.data());
  device_.WriteDescriptorStorageBuffer(scene_set_[current_frame].GetHandle(), 2, sb_point_lights_[current_frame], 0,
                                       light_environment_.point_lights.size() * sizeof(PointLight));

  device_.LoadBufferData<SpotLight>(sb_spot_lights_[current_frame], 0, light_environment_.spot_lights.size(),
                                    light_environment_.spot_lights.data());
  device_.WriteDescriptorStorageBuffer(scene_set_[current_frame].GetHandle(), 3, sb_spot_lights_[current_frame], 0,
                                       light_environment_.spot_lights.size() * sizeof(SpotLight));
}