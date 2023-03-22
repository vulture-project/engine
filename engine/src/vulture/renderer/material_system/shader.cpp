/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file shader.cpp
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

#include <vulture/renderer/material_system/shader.hpp>

using namespace vulture;

namespace vulture {
namespace detail {

bool ReadBinaryFile(const StringView filename, Vector<uint32_t>& output) {
  std::ifstream file(filename, std::ios::ate | std::ios::binary);
  if (!file.is_open()) {
    return false;
  }

  uint64_t file_size = file.tellg();
  output.resize(file_size / 4);

  file.seekg(0);
  file.read(reinterpret_cast<char*>(output.data()), file_size);

  file.close();

  return true;
}

}  // namespace detail
}  // namespace vulture

Shader::Shader(RenderDevice& device) : device_(device) {}

Shader::~Shader() {
  for (uint32_t i = 0; i < pipeline_description_.descriptor_sets_count; ++i) {
    device_.DeleteDescriptorSetLayout(pipeline_description_.descriptor_set_layouts[i]);
  }

  for (uint32_t i = 0; i < pipeline_description_.shader_modules_count; ++i) {
    device_.DeleteShaderModule(pipeline_description_.shader_modules[i]);
  }

  if (IsBuilt()) {
    device_.DeletePipeline(pipeline_);
  }
}

bool Shader::Load(const StringView filename) {
  LOG_INFO("Loading shader \"{}\"", filename);

  YAML::Node root = YAML::LoadFile(filename.data());
  if (!root.IsDefined()) {
    LOG_ERROR("Couldn't open shader file \"{}\"!", filename);
    return false;
  }

  /* Name */
  YAML::Node name_node = root["name"];
  if (!name_node) {
    LOG_ERROR("No declaration of \"name\" found!");
    return false;
  }

  name_ = name_node.as<std::string>();

  /* Render Pass */
  YAML::Node render_pass_node = root["target_render_pass"];
  if (!render_pass_node) {
    LOG_ERROR("No declaration of \"target_render_pass\" found!");
    return false;
  }

  target_pass_id_ = GeneratePassIdFromString(render_pass_node.as<std::string>());

  /* Pipeline Description */
  if (!ParsePipelineDescription(root)) {
    return false;
  }

  if (!ParseShaderSources(root)) {
    return false;
  }

  if (!DeclarePushConstants()) {
    return false;
  }

  if (!CreateDescriptorSetLayouts()) {
    return false;
  }

  return true;
}

bool Shader::ParsePipelineDescription(YAML::Node& root) {
  /* Vertex Format */
  VertexFormat vertex_format = VertexFormat::kVertex3D;  // default
  YAML::Node vertex_format_node = root["vertex_format"];
  if (vertex_format_node) {
    // TODO: (tralf-strues) add other formats when they are added
  }

  switch (vertex_format) {
    case VertexFormat::kVertex3D: {
      pipeline_description_.input_vertex_data_info = Vertex3D::GetVertexDataInfo();
      break;
    }

    default: {
      LOG_ERROR("Invalid VertexFormat specified \"{}\"", vertex_format_node.as<std::string>());
      return false;
    }
  };

#define PARSE_ENUM_VALUE(Str, PipelineField, Enum, DefaultValue)          \
  YAML::Node Str##_node = root[#Str];                                     \
  pipeline_description_.PipelineField = Enum::DefaultValue; /* default */ \
  if (Str##_node) {                                                       \
    Enum Str = StrTo##Enum(Str##_node.as<std::string>().c_str());         \
    if (Str != Enum::kCount) {                                            \
      pipeline_description_.PipelineField = Str;                          \
    }                                                                     \
  }

#define PARSE_BOOL_VALUE(Str, PipelineField, DefaultValue)          \
  YAML::Node Str##_node = root[#Str];                               \
  pipeline_description_.PipelineField = DefaultValue; /* default */ \
  if (Str##_node) {                                                 \
    pipeline_description_.PipelineField = Str##_node.as<bool>();    \
  }

  PARSE_ENUM_VALUE(topology, input_assembly_info.topology, Topology, kTriangleList);

  /* Rasterization */
  PARSE_ENUM_VALUE(cull, rasterization_info.cull_mode, CullMode, kBackOnly);
  PARSE_ENUM_VALUE(front_face, rasterization_info.front_face, FrontFace, kCounterClockwise);
  PARSE_ENUM_VALUE(polygon_mode, rasterization_info.polygon_mode, PolygonMode, kFill);
  
  /* Depth Test */
  PARSE_BOOL_VALUE(depth_test_enable, depth_test_description.test_enable, true);
  PARSE_BOOL_VALUE(depth_write_enable, depth_test_description.write_enable, true);
  PARSE_ENUM_VALUE(depth_compare, depth_test_description.compare_op, CompareOperation, kLess);

  /* Blend */
  PARSE_BOOL_VALUE(blend_enable, blend_description.enable, false);

  PARSE_ENUM_VALUE(blend_src_color_factor, blend_description.src_color_blend_factor, ColorBlendFactor, kSrcAlpha);
  PARSE_ENUM_VALUE(blend_dst_color_factor, blend_description.dst_color_blend_factor, ColorBlendFactor, kDstAlpha);
  PARSE_ENUM_VALUE(blend_color_operation, blend_description.color_blend_op, ColorBlendOperation, kAdd);

  PARSE_ENUM_VALUE(blend_src_alpha_factor, blend_description.src_alpha_blend_factor, ColorBlendFactor, kOne);
  PARSE_ENUM_VALUE(blend_dst_alpha_factor, blend_description.dst_alpha_blend_factor, ColorBlendFactor, kZero);
  PARSE_ENUM_VALUE(blend_alpha_operation, blend_description.alpha_blend_op, ColorBlendOperation, kAdd);

#undef PARSE_ENUM_VALUE
#undef PARSE_BOOL_VALUE

  return true;
}

bool Shader::ParseShaderModule(YAML::Node& root, const String& name, ShaderModuleType module_type) {
  YAML::Node module_node = root[name];
  if (module_node) {
    if (module_node.size() != 2) {
      LOG_ERROR("Shader module declaration must contain two filepaths: source and binary!");
      return false;
    }

    Vector<uint32_t> binary;
    String path = module_node[1].as<std::string>();
    if (!detail::ReadBinaryFile(path, binary)) {
      LOG_ERROR("Shader file \"{}\" not found!", path);
      return false;
    }

    reflection_.AddShaderModule(module_type, binary);

    uint32_t module_idx = pipeline_description_.shader_modules_count;
    pipeline_description_.shader_modules[module_idx] = device_.CreateShaderModule(
        module_type, binary.size() * sizeof(uint32_t), reinterpret_cast<const uint32_t*>(binary.data()));
    ++pipeline_description_.shader_modules_count;

    return true;
  }

  return false;
}

bool Shader::ParseShaderSources(YAML::Node& root) {
  ParseShaderModule(root, "vert_shader", ShaderModuleType::kVertex);
  ParseShaderModule(root, "frag_shader", ShaderModuleType::kFragment);

  return true;
}

bool Shader::DeclarePushConstants() {
  for (const auto& push_constant : reflection_.GetPushConstants()) {
    PushConstantRange range{};
    range.offset        = push_constant.offset;
    range.size          = push_constant.size;
    range.shader_stages = GetShaderStageBitFromShaderModuleType(push_constant.shader_module);

    pipeline_description_.push_constant_ranges[pipeline_description_.push_constant_ranges_count++] = range;
  }

  return true;
}

bool Shader::CreateDescriptorSetLayouts() {
  std::array<DescriptorSetLayoutInfo, kMaxPipelineDescriptorSets> layout_infos;
  
  for (const auto& uniform_buffer : reflection_.GetUniformBuffers()) {
    DescriptorSetLayoutBindingInfo binding_info{};
    binding_info.binding_idx     = uniform_buffer.binding;
    binding_info.descriptor_type = DescriptorType::kUniformBuffer;
    binding_info.shader_stages   = uniform_buffer.shader_stages;

    layout_infos[uniform_buffer.set].bindings_layout_info.emplace_back(binding_info);
  }

  for (const auto& sampler2d : reflection_.GetSampler2Ds()) {
    DescriptorSetLayoutBindingInfo binding_info{};
    binding_info.binding_idx     = sampler2d.binding;
    binding_info.descriptor_type = DescriptorType::kTextureSampler;
    binding_info.shader_stages   = sampler2d.shader_stages;

    layout_infos[sampler2d.set].bindings_layout_info.emplace_back(binding_info);
  }

  for (uint32_t i = 0; i < layout_infos.size(); ++i) {
    if (layout_infos[i].bindings_layout_info.size() > 0) {
      pipeline_description_.descriptor_set_layouts[i] = device_.CreateDescriptorSetLayout(layout_infos[i]);
      
      ++pipeline_description_.descriptor_sets_count;
    }
  }

  return true;
}

PipelineHandle Shader::GetPipeline() const { return pipeline_; }

bool Shader::IsBuilt() const {
  return ValidRenderHandle(pipeline_);
}

void Shader::Build(RenderPassHandle compatible_render_pass, uint32_t subpass_idx) {
  if (ValidRenderHandle(pipeline_)) {
    device_.DeletePipeline(pipeline_);
  }

  pipeline_ = device_.CreatePipeline(pipeline_description_, compatible_render_pass, subpass_idx);
}

RenderPassId Shader::GetTargetPassId() const { return target_pass_id_; }

const ShaderReflection& Shader::GetReflection() const { return reflection_; }

const PipelineDescription& Shader::GetPipelineDescription() const { return pipeline_description_; }
