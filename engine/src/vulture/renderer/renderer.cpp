/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file renderer.cpp
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

#include <vulture/renderer/renderer.hpp>

using namespace vulture;

struct UBFrameData {
  float time{0};
};

Renderer::Renderer(RenderDevice& device, Vector<UniquePtr<IRenderFeature>> features)
    : device_(device), render_graph_(blackboard_), features_(std::move(features)) {
  CreateDescriptorSets();
  CreateBuffers();
  WriteDescriptors();

  blackboard_.Add<RendererBlackboardData>();

  render_graph_.DeclareTexture("backbuffer", TextureLayout::kShaderReadOnly);

  for (auto& feature : features_) {
    render_graph_.BeginSubgraph(feature->Name());
    feature->SetupRenderPasses(render_graph_);
    render_graph_.EndSubgraph();
  }

  render_graph_.Setup();
}

LightEnvironment& Renderer::GetLightEnvironment() { return light_environment_; }

rg::RenderGraph& Renderer::GetRenderGraph() { return render_graph_; }
Vector<UniquePtr<IRenderFeature>>& Renderer::GetFeatures() { return features_; }

void Renderer::Render(CommandBuffer& command_buffer, const Camera& camera, RenderQueue render_queue, float time,
                      uint32_t frame_in_flight) {
  UpdateBuffers(frame_in_flight, camera, time);
  UpdateBlackboard(frame_in_flight, camera, time);

  RenderContext context{device_, command_buffer, frame_in_flight,   render_graph_,
                        camera,  render_queue,   light_environment_};
  for (auto& feature : features_) {
    feature->Execute(context);
  }

  static bool first_frame = true;
  if (first_frame) {
    render_graph_.Compile(device_);
    first_frame = false;

    // FIXME: (tralf-strues) get rid of
    std::ofstream output_file("log/render_graph.dot", std::ios::trunc);
    assert(output_file.is_open());
    render_graph_.ExportGraphviz(output_file);
    system("dot -Tpng log/render_graph.dot > log/render_graph.png");
    output_file.close();
  }

  render_graph_.Execute(device_, command_buffer);
}

void Renderer::CreateDescriptorSets() {
  const ShaderStageFlags stage_flags = kShaderStageBitVertex | kShaderStageBitFragment;

  for (uint32_t frame = 0; frame < kFramesInFlight; ++frame) {
    /* Frame Set */
    frame_set_[frame].AddBinding(DescriptorType::kUniformBuffer, stage_flags).Build(device_);

    /* View Set */
    main_view_set_[frame].AddBinding(DescriptorType::kUniformBuffer, stage_flags).Build(device_);

    /* Scene Set */
    scene_set_[frame].AddBinding(DescriptorType::kUniformBuffer, stage_flags)
                     .AddBinding(DescriptorType::kStorageBuffer, stage_flags)
                     .AddBinding(DescriptorType::kStorageBuffer, stage_flags)
                     .AddBinding(DescriptorType::kStorageBuffer, stage_flags)
                     .Build(device_);
  }
}

void Renderer::CreateBuffers() {
  for (uint32_t frame = 0; frame < kFramesInFlight; ++frame) {
    /* Frame */
    ub_frame_[frame] = device_.CreateDynamicUniformBuffer<UBFrameData>(1);

    /* View */
    ub_main_view_[frame] = device_.CreateDynamicUniformBuffer<UBViewData>(1);

    /* Scene */
    ub_light_[frame] = device_.CreateDynamicUniformBuffer<UBLightData>(1);

    sb_directional_lights_[frame] = device_.CreateDynamicStorageBuffer<DirectionalLight>(kMaxDirectionalLights);
    sb_point_lights_[frame] = device_.CreateDynamicStorageBuffer<PointLight>(kMaxPointLights);
    sb_spot_lights_[frame] = device_.CreateDynamicStorageBuffer<SpotLight>(kMaxSpotLights);
  }
}

void Renderer::WriteDescriptors() {
  for (uint32_t frame = 0; frame < kFramesInFlight; ++frame) {
    /* Frame set */
    device_.WriteDescriptorUniformBuffer(frame_set_[frame].GetHandle(), 0, ub_frame_[frame], 0, sizeof(UBFrameData));

    /* View set */
    device_.WriteDescriptorUniformBuffer(main_view_set_[frame].GetHandle(), 0, ub_main_view_[frame], 0,
                                         sizeof(UBViewData));

    /* Scene set */
    device_.WriteDescriptorUniformBuffer(scene_set_[frame].GetHandle(), 0, ub_light_[frame], 0, sizeof(UBLightData));

    device_.WriteDescriptorStorageBuffer(scene_set_[frame].GetHandle(), 1, sb_directional_lights_[frame], 0,
                                         kMaxDirectionalLights * sizeof(DirectionalLight));

    device_.WriteDescriptorStorageBuffer(scene_set_[frame].GetHandle(), 2, sb_point_lights_[frame], 0,
                                         kMaxPointLights * sizeof(PointLight));

    device_.WriteDescriptorStorageBuffer(scene_set_[frame].GetHandle(), 3, sb_spot_lights_[frame], 0,
                                         kMaxSpotLights * sizeof(SpotLight));
  }
}

void Renderer::UpdateBuffers(uint32_t frame, const Camera& camera, float time) {
  /* Frame */
  UBFrameData frame_data{};
  frame_data.time = time;

  device_.LoadBufferData<UBFrameData>(ub_frame_[frame], 0, 1, &frame_data);

  /* Main view */
  UBViewData main_view_data{};
  main_view_data.view       = camera.ViewMatrix();
  main_view_data.proj       = camera.ProjMatrix();
  main_view_data.position   = camera.Position();
  main_view_data.near_plane = camera.NearPlane();
  main_view_data.far_plane  = camera.FarPlane();
  main_view_data.exposure   = camera.exposure;

  device_.LoadBufferData<UBViewData>(ub_main_view_[frame], 0, 1, &main_view_data);

  /* Scene */
  UBLightData light_data{};
  light_data.directional_lights_count = light_environment_.directional_lights.size();
  light_data.point_lights_count       = light_environment_.point_lights.size();
  light_data.spot_lights_count        = light_environment_.spot_lights.size();
  device_.LoadBufferData<UBLightData>(ub_light_[frame], 0, 1, &light_data);

  device_.LoadBufferData<DirectionalLight>(sb_directional_lights_[frame], 0,
                                           light_environment_.directional_lights.size(),
                                           light_environment_.directional_lights.data());

  device_.LoadBufferData<PointLight>(sb_point_lights_[frame], 0, light_environment_.point_lights.size(),
                                     light_environment_.point_lights.data());

  device_.LoadBufferData<SpotLight>(sb_spot_lights_[frame], 0, light_environment_.spot_lights.size(),
                                    light_environment_.spot_lights.data());
}

void Renderer::UpdateBlackboard(uint32_t frame, const Camera& camera, float time) {
  RendererBlackboardData& blackboard_data  = blackboard_.Get<RendererBlackboardData>();
  blackboard_data.time                     = time;
  blackboard_data.frame_in_flight           = frame;
  blackboard_data.descriptor_set_frame     = frame_set_[frame].GetHandle();
  
  blackboard_data.light_environment        = &light_environment_;
  blackboard_data.descriptor_set_scene     = scene_set_[frame].GetHandle();

  blackboard_data.main_camera              = &camera;
  blackboard_data.descriptor_set_main_view = main_view_set_[frame].GetHandle();
}