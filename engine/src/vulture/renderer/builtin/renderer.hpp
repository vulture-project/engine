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

class Renderer {
 public:
  static constexpr uint32_t kMaxDirectionalLights = 16;
  static constexpr uint32_t kMaxPointLights       = 128;
  static constexpr uint32_t kMaxSpotLights        = 128;

 public:
  Renderer(RenderDevice& device, UniquePtr<rg::RenderGraph> render_graph);
  ~Renderer();

  void Render(CommandBuffer& command_buffer, uint32_t current_frame);

  RenderDevice& GetDevice();

  rg::RenderGraph& GetRenderGraph();
  rg::Blackboard& GetBlackboard();

  void UpdateFrameData(float time);
  void UpdateViewData(const glm::mat4& view, const glm::mat4& proj, glm::vec3 position);

  LightEnvironment& GetLightEnvironment();

 private:
  // void CreateCommandBuffers();
  void CreateDescriptorSets();
  void CreateBuffers();
  void InitLightBuffers();

  void WriteBlackboard(uint32_t current_frame);
  void WriteDescriptors(uint32_t current_frame);

 private:
  /* Device */
  RenderDevice& device_;

  /* Render Graph */
  UniquePtr<rg::RenderGraph> render_graph_;

  /* Frame data */
  Array<DescriptorSet, kFramesInFlight> frame_set_;

  Array<BufferHandle, kFramesInFlight> ub_frame_{kInvalidRenderResourceHandle};
  struct UBFrameData {
    float time{0};
  } ub_frame_data_;

  /* View data */
  Array<DescriptorSet, kFramesInFlight> view_set_;

  Array<BufferHandle, kFramesInFlight> ub_view_{kInvalidRenderResourceHandle};
  struct UBViewData {
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec3 position;
  } ub_view_data_;

  /* Light */
  Array<DescriptorSet, kFramesInFlight> scene_set_;

  LightEnvironment light_environment_;

  Array<BufferHandle, kFramesInFlight> sb_directional_lights_{kInvalidRenderResourceHandle};
  Array<BufferHandle, kFramesInFlight> sb_point_lights_{kInvalidRenderResourceHandle};
  Array<BufferHandle, kFramesInFlight> sb_spot_lights_{kInvalidRenderResourceHandle};

  Array<BufferHandle, kFramesInFlight> ub_light_{kInvalidRenderResourceHandle};
  struct UBLightEnvironmentData {
    int directional_lights_count{0};
    int point_lights_count{0};
    int spot_lights_count{0};
  } ub_light_data_;
};

}  // namespace vulture