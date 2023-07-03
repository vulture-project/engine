/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file descriptor.hpp
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

#include <cstdint>
#include <vulture/renderer/graphics_api/shader_module.hpp>
#include <vector>

namespace vulture {

/* Descriptor Set Layout */
enum class DescriptorType : uint32_t {
  kInvalid,
  kUniformBuffer,
  kStorageBuffer,
  kInputAttachment,
  kTextureSampler,

  // TODO: kDynamicUniformBuffer

  kTotalTypes
};

struct DescriptorSetLayoutBindingInfo {
  uint32_t         binding_idx     {0};
  DescriptorType   descriptor_type {DescriptorType::kInvalid};
  // TODO: Several descriptors are not currently supported (used for arrays of descriptors)
  // const uint32_t   descriptors_count  {1};
  ShaderStageFlags shader_stages   {kShaderStageBitNone};
};

struct DescriptorSetLayoutInfo {
  std::vector<DescriptorSetLayoutBindingInfo> bindings_layout_info;
};

}  // namespace vulture