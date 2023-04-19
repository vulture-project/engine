/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file renderer.hpp
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

#include <vulture/renderer/descriptor_set.hpp>
#include <vulture/renderer/light.hpp>
#include <vulture/renderer/render_feature.hpp>

namespace vulture {

struct RendererBlackboardData {
  float                    time                     {0.0f};
  uint32_t                frame_in_flight           {0};
  DescriptorSetHandle     descriptor_set_frame     {kInvalidRenderResourceHandle};

  const LightEnvironment* light_environment        {nullptr};
  DescriptorSetHandle     descriptor_set_scene     {kInvalidRenderResourceHandle};

  const Camera*           main_camera              {nullptr};
  DescriptorSetHandle     descriptor_set_main_view {kInvalidRenderResourceHandle};
};

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

struct UBLightData {
  alignas(4) uint directional_lights_count{0};
  alignas(4) uint point_lights_count{0};
  alignas(4) uint spot_lights_count{0};
};

class Renderer {
 public:
  Renderer(RenderDevice& device, Vector<UniquePtr<IRenderFeature>> features);

  void Render(CommandBuffer& command_buffer, const Camera& camera, RenderQueue render_queue, float time,
              uint32_t frame_in_flight);

  LightEnvironment& GetLightEnvironment();

  rg::RenderGraph& GetRenderGraph();
  Vector<UniquePtr<IRenderFeature>>& GetFeatures();

 private:
  void CreateDescriptorSets();
  void CreateBuffers();
  void WriteDescriptors();

  void UpdateBuffers(uint32_t frame, const Camera& camera, float time);
  void UpdateBlackboard(uint32_t frame, const Camera& camera, float time);

 private:
  RenderDevice&                     device_;

  LightEnvironment                  light_environment_;

  rg::Blackboard                    blackboard_;
  rg::RenderGraph                   render_graph_;
  Vector<UniquePtr<IRenderFeature>> features_;

  /* Descriptor sets */
  PerFrameData<DescriptorSet>       frame_set_;
  PerFrameData<BufferHandle>        ub_frame_{kInvalidRenderResourceHandle};

  PerFrameData<DescriptorSet>       main_view_set_;
  PerFrameData<BufferHandle>        ub_main_view_{kInvalidRenderResourceHandle};

  PerFrameData<DescriptorSet>       scene_set_;
  
  PerFrameData<BufferHandle>        ub_light_{kInvalidRenderResourceHandle};
  PerFrameData<BufferHandle>        sb_directional_lights_{kInvalidRenderResourceHandle};
  PerFrameData<BufferHandle>        sb_point_lights_{kInvalidRenderResourceHandle};
  PerFrameData<BufferHandle>        sb_spot_lights_{kInvalidRenderResourceHandle};
};

}  // namespace vulture