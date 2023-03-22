/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file shader.hpp
 * @date 2023-03-18
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

#include <vulture/asset/asset.hpp>
#include <vulture/renderer/geometry/vertex_formats.hpp>
#include <vulture/renderer/graphics_api/render_device.hpp>
#include <vulture/renderer/material_system/shader_reflection.hpp>
#include <vulture/renderer/render_graph/per_renderpass_data.hpp>

namespace vulture {

/**
 * @brief Graphics/compute pipeline abstraction targeting a specific render pass.
 *
 * Example file for this can look like this (YAML format):
 *     name: ForwardPBR
 *     target_render_pass: ForwardPass
 * 
 *     vert_shader: ["forward_shader_pbr.vert", "forward_shader_pbr.vert.spv"]
 *     frag_shader: ["forward_shader_pbr.frag", "forward_shader_pbr.frag.spv"]
 * 
 *     # Vertex Format
 *     vertex_format: Vertex3D           # default: Vertex3D
 *     topology: TriangleList            # default: TriangleList
 * 
 *     # Rasterization
 *     cull: BackOnly                    # default: BackOnly
 *     front_face: CounterClockwise      # default: CounterClockwise
 *     polygon_mode: Fill                # default: Fill
 * 
 *     # Depth Test
 *     depth_test_enable: true           # default: true
 *     depth_write_enable: true          # default: true
 *     depth_compare: Less               # default: Less
 * 
 *     # Blend (alpha-blending by default, but disabled)
 *     blend_enable: false               # default: false
 * 
 *     blend_src_color_factor: SrcAlpha  # default: SrcAlpha
 *     blend_dst_color_factor: DstAlpha  # default: DstAlpha
 *     blend_color_operation: Add        # default: Add
 * 
 *     blend_src_alpha_factor: SrcAlpha  # default: One
 *     blend_dst_alpha_factor: DstAlpha  # default: Zero
 *     blend_alpha_operation: Add        # default: Add
 */
class Shader : public IAsset {
public:
  Shader(RenderDevice& device);
  ~Shader() override;

  bool Load(const StringView filename);

  PipelineHandle GetPipeline() const;
  bool IsBuilt() const;
  void Build(RenderPassHandle compatible_render_pass, uint32_t subpass_idx = 0);

  RenderPassId GetTargetPassId() const;
  const ShaderReflection& GetReflection() const;
  const PipelineDescription& GetPipelineDescription() const;

private:
  bool ParsePipelineDescription(YAML::Node& root);
  bool ParseShaderSources(YAML::Node& root);
  bool ParseShaderModule(YAML::Node& root, const String& name, ShaderModuleType module_type);
  bool DeclarePushConstants();
  bool CreateDescriptorSetLayouts();

private:
  RenderDevice&       device_;

  String              name_;
  RenderPassId        target_pass_id_;
  ShaderReflection     reflection_;
  PipelineDescription pipeline_description_;
  PipelineHandle      pipeline_{kInvalidRenderResourceHandle};
};

}  // namespace vulture