/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file shader_module.hpp
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
#include <string>

namespace vulture {

enum class ShaderModuleType : uint32_t {
  kInvalid,
  kVertex,
  kFragment,
};

enum ShaderStageBit : uint32_t {
  kShaderStageBitNone        = 0x0000'0000,

  kShaderStageBitVertex      = 0x0000'0001,
  /* TODO: other stages */
  kShaderStageBitFragment    = 0x0000'0010,
  kShaderStageBitAllGraphics = 0x0000'001F,
};

using ShaderStageFlags = uint32_t;

enum class ShaderLanguage : uint32_t {
  kInvalid,
  kVulkanGLSL,
  /* NOTE: in the future, one should only use VulkanGLSL and on demand the engine will convert it to OpenglGLSL */
  kOpenglGLSL,
  /* kHLSL */
};

inline ShaderStageBit GetShaderStageBitFromShaderModuleType(ShaderModuleType type) {
  switch (type) {
    case ShaderModuleType::kVertex:   { return kShaderStageBitVertex; }
    case ShaderModuleType::kFragment: { return kShaderStageBitFragment; }

    default: { assert(!"Invalid ShaderModuleType"); }
  }
}

}  // namespace vulture