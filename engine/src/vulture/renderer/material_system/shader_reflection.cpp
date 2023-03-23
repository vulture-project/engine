/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file shader_reflection.cpp
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

#include <algorithm>
#include <iostream>

#include <spirv_cross/spirv_glsl.hpp>
#include <vulture/renderer/material_system/shader_reflection.hpp>

using namespace vulture;

const char* BaseTypeToString(spirv_cross::SPIRType::BaseType type) {
  switch (type) {
    case spirv_cross::SPIRType::BaseType::Int:     { return "int"; }
    case spirv_cross::SPIRType::BaseType::UInt:    { return "uint"; }
    case spirv_cross::SPIRType::BaseType::Float:   { return "float"; }
    case spirv_cross::SPIRType::BaseType::Double:  { return "double"; }
    case spirv_cross::SPIRType::BaseType::Boolean: { return "bool"; }
    default:                                       { return "none_type"; }
  }
}

ShaderDataType SPIRTypeToShaderDataType(spirv_cross::SPIRType type) {
  spirv_cross::SPIRType::BaseType base_type = type.basetype;
  uint32_t                        vec_size  = type.vecsize;
  uint32_t                        columns   = type.columns;

  // Matrix
  if (columns > 1) {
    if (base_type != spirv_cross::SPIRType::Float || vec_size != columns) {
      return ShaderDataType::kInvalid;
    }

    switch (columns) {
      case 2:  { return ShaderDataType::kMat2; }
      case 3:  { return ShaderDataType::kMat3; }
      case 4:  { return ShaderDataType::kMat4; }
      default: { return ShaderDataType::kInvalid; }
    }
  }

  // Vector
  if (vec_size > 1) {
    if (base_type == spirv_cross::SPIRType::Float) {
      switch (vec_size) {
        case 2:  { return ShaderDataType::kVec2; }
        case 3:  { return ShaderDataType::kVec3; }
        case 4:  { return ShaderDataType::kVec4; }
        default: { return ShaderDataType::kInvalid; }
      }
    } else if (base_type == spirv_cross::SPIRType::Int) {
      switch (vec_size) {
        case 2:  { return ShaderDataType::kIVec2; }
        case 3:  { return ShaderDataType::kIVec3; }
        case 4:  { return ShaderDataType::kIVec4; }
        default: { return ShaderDataType::kInvalid; }
      }
    }

    return ShaderDataType::kInvalid;
  }

  // Scalar
  switch (base_type) {
    case spirv_cross::SPIRType::Boolean: { return ShaderDataType::kBoolean; }
    case spirv_cross::SPIRType::Int:     { return ShaderDataType::kInt; }
    case spirv_cross::SPIRType::UInt:    { return ShaderDataType::kUInt; }
    case spirv_cross::SPIRType::Float:   { return ShaderDataType::kFloat; }
    default:                             { return ShaderDataType::kInvalid; }
  }

  return ShaderDataType::kInvalid;
}

void ShaderReflection::AddShaderModule(ShaderModuleType shader_module, const std::vector<uint32_t>& binary) {
  std::vector<uint32_t> copy_binary = binary;
  spirv_cross::Compiler compiler(std::move(copy_binary));
  spirv_cross::ShaderResources resources = compiler.get_shader_resources();

  ShaderStageBit stage_bit = GetShaderStageBitFromShaderModuleType(shader_module);

  /* Vertex attributes */
  if (shader_module == ShaderModuleType::kVertex) {
    for (const auto& resource : resources.stage_inputs) {
      VertexAttribute& attribute = vertex_attributes_.emplace_back();
      attribute.location = compiler.get_decoration(resource.id, spv::DecorationLocation);
      attribute.type     = SPIRTypeToShaderDataType(compiler.get_type(resource.type_id));
      attribute.name     = resource.name;
    }

    std::sort(vertex_attributes_.begin(), vertex_attributes_.end(), [](const auto& first, const auto& second) {
      return first.location < second.location;
    });
  }

  /* Push constants */
  for (const auto& resource : resources.push_constant_buffers) {
    const auto& type         = compiler.get_type(resource.base_type_id);
    uint32_t    member_count = static_cast<uint32_t>(type.member_types.size());

    PushConstant& push_constant = push_constants_.emplace_back();
    push_constant.shader_module = shader_module;
    push_constant.name          = resource.name;
    push_constant.offset        = compiler.get_decoration(resource.id, spv::DecorationOffset);
    push_constant.size          = static_cast<uint32_t>(compiler.get_declared_struct_size(type));
    push_constant.members.resize(member_count);

    for (uint32_t i = 0; i < member_count; ++i) {
      Member& member = push_constant.members[i];
      member.type   = SPIRTypeToShaderDataType(compiler.get_type(type.member_types[i]));
      member.name   = compiler.get_member_name(type.self, i);
      member.size   = static_cast<uint32_t>(compiler.get_declared_struct_member_size(type, i));
      member.offset = compiler.type_struct_member_offset(type, i);
    }
  }

  /* Uniform buffers */
  for (const auto& resource : resources.uniform_buffers) {
    const auto& type         = compiler.get_type(resource.base_type_id);
    uint32_t    member_count = static_cast<uint32_t>(type.member_types.size());

    // In case the buffer has already been declared in another shader stage.
    bool already_exists = false;
    for (auto& uniform_buffer : uniform_buffers_) {
      if (uniform_buffer.name == resource.name) {
        uniform_buffer.shader_stages |= stage_bit;
        already_exists = true;
        break;
      }
    }

    if (!already_exists) {
      UniformBuffer& uniform_buffer = uniform_buffers_.emplace_back();
      uniform_buffer.shader_stages  = stage_bit;
      uniform_buffer.name           = resource.name;
      uniform_buffer.size           = static_cast<uint32_t>(compiler.get_declared_struct_size(type));
      uniform_buffer.set            = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
      uniform_buffer.binding        = compiler.get_decoration(resource.id, spv::DecorationBinding);
      uniform_buffer.members.resize(member_count);

      for (uint32_t i = 0; i < member_count; ++i) {
        Member& member = uniform_buffer.members[i];
        member.type   = SPIRTypeToShaderDataType(compiler.get_type(type.member_types[i]));
        member.name   = compiler.get_member_name(type.self, i);
        member.size   = static_cast<uint32_t>(compiler.get_declared_struct_member_size(type, i));
        member.offset = compiler.type_struct_member_offset(type, i);
      }
    }
  }

  /* Sampler2D */
  for (const auto& resource : resources.sampled_images) {
    const auto& type = compiler.get_type(resource.base_type_id);

    // In case the sampler has already been declared in another shader stage.
    bool already_exists = false;
    for (auto& sampler : sampler2Ds_) {
      if (sampler.name == resource.name) {
        sampler.shader_stages |= stage_bit;
        already_exists = true;
        break;
      }
    }

    if (!already_exists) {
      Sampler2D& sampler = sampler2Ds_.emplace_back();
      sampler.shader_stages = stage_bit;
      sampler.name          = resource.name;
      sampler.array_size    = (type.array[0] > 0) ? type.array[0] : 1;
      sampler.set           = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
      sampler.binding       = compiler.get_decoration(resource.id, spv::DecorationBinding);
    }
  }
}

const std::vector<ShaderReflection::VertexAttribute>& ShaderReflection::GetVertexAttributes() const {
  return vertex_attributes_;
}

const std::vector<ShaderReflection::PushConstant>& ShaderReflection::GetPushConstants() const {
  return push_constants_;
}

const std::vector<ShaderReflection::UniformBuffer>& ShaderReflection::GetUniformBuffers() const {
  return uniform_buffers_;
}

const std::vector<ShaderReflection::Sampler2D>& ShaderReflection::GetSampler2Ds() const {
  return sampler2Ds_;
}

void PrintMembers(const std::vector<ShaderReflection::Member>& members) {
  for (const auto& member : members) {
    std::cout << "    - " << ShaderDataTypeToStr(member.type) << " \"" << member.name << "\"\n";
    std::cout << "      (size = " << member.size << ", offset = " << member.offset << ")\n";
  }
}

void ShaderReflection::PrintData() const {
  std::cout << "====Vertex attributes====\n";
  for (const auto& attribute : vertex_attributes_) {
    std::cout << "[" << attribute.location << "] ";
    std::cout << ShaderDataTypeToStr(attribute.type) << " ";
    std::cout << "\"" << attribute.name << "\"\n";
  }

  std::cout << "====Push constants====\n";
  for (const auto& push_constant : push_constants_) {
    std::cout << ((push_constant.shader_module == ShaderModuleType::kVertex) ? "Vertex Shader" : "Fragment Shader") << "\n";
    std::cout << "---------------\n";

    std::cout << "\"" << push_constant.name << "\" ";
    std::cout << "(size = " << push_constant.size << ", offset = " << push_constant.offset << ")\n";

    PrintMembers(push_constant.members);
  }

  std::cout << "====Uniform buffers====\n";
  for (const auto& uniform_buffer : uniform_buffers_) {
    std::cout << "layout(set = " << uniform_buffer.set << ", binding = " << uniform_buffer.binding << ") ";
    std::cout << "\"" << uniform_buffer.name << "\" ";
    std::cout << "(size = " << uniform_buffer.size << ")\n";

    PrintMembers(uniform_buffer.members);
  }

  std::cout << "====Sampler2Ds====\n";
  for (const auto& sampler : sampler2Ds_) {
    std::cout << "layout(set = " << sampler.set << ", binding = " << sampler.binding << ") ";
    std::cout << "\"" << sampler.name << "\" ";
    std::cout << "(array size = " << sampler.array_size << ")\n";
  }
}
