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

#include <spirv_glsl.hpp>
#include <vulture/renderer/material_system/shader_reflection.hpp>

using namespace vulture;

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
    case spirv_cross::SPIRType::Struct:  { return ShaderDataType::kStruct; }
    case spirv_cross::SPIRType::Boolean: { return ShaderDataType::kBoolean; }
    case spirv_cross::SPIRType::Int:     { return ShaderDataType::kInt; }
    case spirv_cross::SPIRType::UInt:    { return ShaderDataType::kUInt; }
    case spirv_cross::SPIRType::Float:   { return ShaderDataType::kFloat; }
    default:                             { return ShaderDataType::kInvalid; }
  }

  return ShaderDataType::kInvalid;
}

void ReflectMembers(spirv_cross::CompilerGLSL& compiler, const spirv_cross::SPIRType& parent_type,
                    Vector<ShaderReflection::Member>& members) {
  uint32_t member_count = static_cast<uint32_t>(parent_type.member_types.size());
  members.resize(member_count);

  for (uint32_t i = 0; i < member_count; ++i) {
    const auto& member_type = compiler.get_type(parent_type.member_types[i]);

    ShaderReflection::Member& member = members[i];
    member.type   = SPIRTypeToShaderDataType(member_type);
    member.name   = compiler.get_member_name(parent_type.self, i);
    member.size   = static_cast<uint32_t>(compiler.get_declared_struct_member_size(parent_type, i));
    member.offset = compiler.type_struct_member_offset(parent_type, i);

    if (member_type.array.size() > 0) {
      // Array, FIXME: (tralf-strues) Only one-dimensional arrays are supported at the moment!
      member.is_array               = true;
      member.array_size             = member_type.array[0];
      member.is_array_variable_size = (member.array_size == 0);
    }
  }
}

void vulture::ShaderReflection::AddShaderModule(ShaderModuleType shader_module, const Vector<uint32_t>& binary) {
  spirv_cross::CompilerGLSL compiler(binary.data(), binary.size());
  spirv_cross::ShaderResources resources = compiler.get_shader_resources();

  ShaderStageBit stage_bit = GetShaderStageBitFromShaderModuleType(shader_module);

  /* Vertex attributes */
  if (shader_module == ShaderModuleType::kVertex) {
    for (const auto& resource : resources.stage_inputs) {
      VertexAttribute& attribute = vertex_attributes_.emplace_back();
      attribute.location         = compiler.get_decoration(resource.id, spv::DecorationLocation);
      attribute.type             = SPIRTypeToShaderDataType(compiler.get_type(resource.type_id));
      attribute.name             = resource.name;
    }

    std::sort(vertex_attributes_.begin(), vertex_attributes_.end(),
              [](const auto& first, const auto& second) { return first.location < second.location; });
  }

  /* Push constants */
  for (const auto& resource : resources.push_constant_buffers) {
    const auto& type = compiler.get_type(resource.base_type_id);

    PushConstant& push_constant = push_constants_.emplace_back();
    push_constant.shader_module = shader_module;
    push_constant.name          = resource.name;
    push_constant.offset        = compiler.get_decoration(resource.id, spv::DecorationOffset);
    push_constant.size          = static_cast<uint32_t>(compiler.get_declared_struct_size(type));

    ReflectMembers(compiler, type, push_constant.members);
  }

  /* Uniform buffers */
  for (const auto& resource : resources.uniform_buffers) {
    const auto& type = compiler.get_type(resource.base_type_id);

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
      
      ReflectMembers(compiler, type, uniform_buffer.members);
    }
  }

  std::sort(uniform_buffers_.begin(), uniform_buffers_.end(), [](const auto& first, const auto& second) {
    return (first.set < second.set && first.binding <= second.binding) ||
           (first.set <= second.set && first.binding < second.binding);
  });

  /* Storage buffers */
  for (const auto& resource : resources.storage_buffers) {
    const auto& type = compiler.get_type(resource.base_type_id);

    // In case the buffer has already been declared in another shader stage.
    bool already_exists = false;
    for (auto& storage_buffer : storage_buffers_) {
      if (storage_buffer.name == resource.name) {
        storage_buffer.shader_stages |= stage_bit;
        already_exists = true;
        break;
      }
    }

    if (!already_exists) {
      StorageBuffer& storage_buffer = storage_buffers_.emplace_back();
      storage_buffer.shader_stages  = stage_bit;
      storage_buffer.name           = resource.name;
      storage_buffer.set            = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
      storage_buffer.binding        = compiler.get_decoration(resource.id, spv::DecorationBinding);
      
      ReflectMembers(compiler, type, storage_buffer.members);
    }
  }

  std::sort(storage_buffers_.begin(), storage_buffers_.end(), [](const auto& first, const auto& second) {
    return (first.set < second.set && first.binding <= second.binding) ||
           (first.set <= second.set && first.binding < second.binding);
  });

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
      sampler.arrayed       = type.image.arrayed;
      sampler.array_size    = (type.array[0] > 0) ? type.array[0] : 1;
      sampler.set           = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
      sampler.binding       = compiler.get_decoration(resource.id, spv::DecorationBinding);
    }
  }

  std::sort(sampler2Ds_.begin(), sampler2Ds_.end(), [](const auto& first, const auto& second) {
    return (first.set < second.set && first.binding <= second.binding) ||
           (first.set <= second.set && first.binding < second.binding);
  });

  VULTURE_ASSERT(resources.separate_images.empty(),   "Seperate images are not supported at the moment!");
  VULTURE_ASSERT(resources.separate_samplers.empty(), "Seperate samplers are not supported at the moment!");
  VULTURE_ASSERT(resources.storage_images.empty(),    "Storage images are not supported at the moment!");
}

const Vector<vulture::ShaderReflection::VertexAttribute>& vulture::ShaderReflection::GetVertexAttributes() const {
  return vertex_attributes_;
}

const Vector<vulture::ShaderReflection::PushConstant>& vulture::ShaderReflection::GetPushConstants() const {
  return push_constants_;
}

const Vector<vulture::ShaderReflection::UniformBuffer>& vulture::ShaderReflection::GetUniformBuffers() const {
  return uniform_buffers_;
}

const Vector<ShaderReflection::StorageBuffer>& ShaderReflection::GetStorageBuffers() const {
  return storage_buffers_;
}

const Vector<vulture::ShaderReflection::Sampler2D>& vulture::ShaderReflection::GetSampler2Ds() const {
  return sampler2Ds_;
}

void PrintMembers(const Vector<vulture::ShaderReflection::Member>& members) {
  for (const auto& member : members) {
    fmt::print("    - {0} {1}",
               ShaderDataTypeToStr(member.type),
               fmt::styled(member.name, fmt::emphasis::italic | fmt::emphasis::underline));

    if (member.is_array && member.is_array_variable_size) {
      fmt::print("[]");
    } else if (member.is_array) {
      fmt::print("[{0}]", member.array_size);
    }

    fmt::println(" (size = {0}, offset = {1})", member.size, member.offset);
  }
}

void vulture::ShaderReflection::PrintData() const {
  fmt::print(fmt::emphasis::bold | fg(fmt::color::golden_rod), "====Vertex attributes====\n");
  for (const auto& attribute : vertex_attributes_) {
    fmt::println("[{0}] {1} {2}",
                 attribute.location,
                 ShaderDataTypeToStr(attribute.type),
                 fmt::styled(attribute.name, fmt::emphasis::underline | fmt::emphasis::italic));
  }
  fmt::print("\n");

  fmt::print(fmt::emphasis::bold | fg(fmt::color::golden_rod), "====Push constants====\n");
  for (const auto& push_constant : push_constants_) {
    fmt::println(((push_constant.shader_module == ShaderModuleType::kVertex) ? "Vertex Shader\n-------------"
                                                                             : "Fragment Shader\n---------------"));
    fmt::println("* {0} (size = {1}, offset = {2})",
                 fmt::styled(push_constant.name, fmt::emphasis::underline | fmt::emphasis::bold),
                 push_constant.size,
                 push_constant.offset);

    PrintMembers(push_constant.members);
  }
  fmt::print("\n");

  fmt::print(fmt::emphasis::bold | fg(fmt::color::golden_rod), "====Uniform buffers====\n");
  for (const auto& uniform_buffer : uniform_buffers_) {
    fmt::println("* [set = {0}, binding = {1}] {2} (size = {3})",
                 uniform_buffer.set,
                 uniform_buffer.binding,
                 fmt::styled(uniform_buffer.name, fmt::emphasis::underline | fmt::emphasis::bold),
                 uniform_buffer.size);

    PrintMembers(uniform_buffer.members);

    fmt::print("\n");
  }

  fmt::print(fmt::emphasis::bold | fg(fmt::color::golden_rod), "====Storage buffers====\n");
  for (const auto& storage_buffer : storage_buffers_) {
    fmt::println("* [set = {0}, binding = {1}] {2}",
                 storage_buffer.set,
                 storage_buffer.binding,
                 fmt::styled(storage_buffer.name, fmt::emphasis::underline | fmt::emphasis::bold));

    PrintMembers(storage_buffer.members);

    fmt::print("\n");
  }

  fmt::print(fmt::emphasis::bold | fg(fmt::color::golden_rod), "====Sampler2Ds====\n");
  for (const auto& sampler : sampler2Ds_) {
    fmt::println("* [set = {0}, binding = {1}] {2} (array size = {3})",
                 sampler.set,
                 sampler.binding,
                 fmt::styled(sampler.name, fmt::emphasis::underline | fmt::emphasis::bold),
                 sampler.array_size);
  }
  fmt::print("\n");
}
