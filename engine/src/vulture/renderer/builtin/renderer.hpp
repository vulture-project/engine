/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file renderer.hpp
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
#include <vulture/renderer/descriptor_set.hpp>
#include <vulture/renderer/light.hpp>

namespace vulture {

struct UBFrameData {
  float time{0};
};

struct UBViewData {
  glm::mat4 view;
  glm::mat4 proj;
  alignas(16) glm::vec3 position;
  alignas(4)  float      near_plane; 
  alignas(4)  float      far_plane;
};

struct UBLightEnvironmentData {
  int directional_lights_count{0};
  int point_lights_count{0};
  int spot_lights_count{0};
};

class Renderer {
 public:
  static constexpr uint32_t kMaxDirectionalLights = 8;
  static constexpr uint32_t kMaxPointLights       = 128;
  static constexpr uint32_t kMaxSpotLights        = 128;

 public:
  Renderer(RenderDevice& device, SharedPtr<Texture> color_output);
  ~Renderer();

  void Render(CommandBuffer& command_buffer, uint32_t current_frame);

  RenderDevice& GetDevice();

  rg::Blackboard& GetBlackboard();
  rg::RenderGraph& GetRenderGraph();

  void UpdateFrameData(float time);
  void UpdateViewData(const glm::mat4& view, const glm::mat4& proj, glm::vec3 position, float near, float far);

  LightEnvironment& GetLightEnvironment();

 private:
  void CreateRenderGraph(SharedPtr<Texture> color_output);
  void CreateDescriptorSets();
  void CreateBuffers();
  void InitLightBuffers();

  void WriteBlackboard(uint32_t current_frame);
  void WriteDescriptors(uint32_t current_frame);

 private:
  /* Device */
  RenderDevice& device_;

  /* Render Graph */
  rg::Blackboard blackboard_;
  UniquePtr<rg::RenderGraph> render_graph_;

  /* Frame data */
  PerFrameData<DescriptorSet> frame_set_;

  PerFrameData<BufferHandle> ub_frame_{kInvalidRenderResourceHandle};
  UBFrameData ub_frame_data_;

  /* View data */
  PerFrameData<DescriptorSet> view_set_;

  PerFrameData<BufferHandle> ub_view_{kInvalidRenderResourceHandle};
  UBViewData ub_view_data_;

  /* Light */
  PerFrameData<DescriptorSet> scene_set_;

  LightEnvironment light_environment_;

  PerFrameData<BufferHandle> sb_directional_lights_{kInvalidRenderResourceHandle};
  PerFrameData<BufferHandle> sb_point_lights_{kInvalidRenderResourceHandle};
  PerFrameData<BufferHandle> sb_spot_lights_{kInvalidRenderResourceHandle};

  PerFrameData<BufferHandle> ub_light_{kInvalidRenderResourceHandle};
  UBLightEnvironmentData ub_light_data_;
};

}  // namespace vulture