/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file shader_reflection.hpp
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

#include <span>
#include <utility>
#include <vector>
#include <vulture/renderer/graphics_api/shader_module.hpp>
#include <vulture/renderer/material_system/shader_data_type.hpp>

namespace vulture {

class ShaderReflection {
 public:
  struct VertexAttribute {
    uint32_t       location{0};
    ShaderDataType type{ShaderDataType::kInvalid};
    String         name;
  };

  struct Member {
    ShaderDataType type{ShaderDataType::kInvalid};
    String         name;
    uint32_t       size{0};
    uint32_t       offset{0};
    bool           is_array{false};
    bool           is_array_variable_size{false};
    uint32_t       array_size{0};   // For scalar size arrays
  };

  struct PushConstant {
    ShaderModuleType shader_module{ShaderModuleType::kInvalid};
    String           name;
    uint32_t         offset{0};
    uint32_t         size{0};
    Vector<Member>   members;
  };

  struct UniformBuffer {
    ShaderStageFlags shader_stages{0};
    String           name;
    uint32_t         size{0};
    uint32_t         set{0};
    uint32_t         binding{0};
    Vector<Member>   members;
  };

  struct StorageBuffer {
    ShaderStageFlags shader_stages{0};
    String           name;
    uint32_t         set{0};
    uint32_t         binding{0};
    Vector<Member>   members;
  };

  struct Sampler2D {
    ShaderStageFlags shader_stages{0};
    String           name;
    uint32_t         array_size{1};
    uint32_t         set{0};
    uint32_t         binding{0};
  };

 public:
  ShaderReflection() = default;

  void AddShaderModule(ShaderModuleType shader_module, const Vector<uint32_t>& binary);

  const Vector<VertexAttribute>& GetVertexAttributes() const;
  const Vector<PushConstant>&    GetPushConstants() const;
  const Vector<UniformBuffer>&   GetUniformBuffers() const;
  const Vector<StorageBuffer>&   GetStorageBuffers() const;
  const Vector<Sampler2D>&       GetSampler2Ds() const;

  void PrintData() const;

 private:
  Vector<VertexAttribute> vertex_attributes_;
  Vector<PushConstant>    push_constants_;
  Vector<UniformBuffer>   uniform_buffers_;
  Vector<StorageBuffer>   storage_buffers_;
  Vector<Sampler2D>       sampler2Ds_;
};

}  // namespace vulture
