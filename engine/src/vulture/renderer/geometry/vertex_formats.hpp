/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file vertex_formats.hpp
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

#include <glm/glm.hpp>
#include <vulture/renderer/graphics_api/pipeline.hpp>

namespace vulture {

enum class VertexFormat : uint32_t{
  kVertex3D,

  kCustom
};

struct Vertex3D {
  glm::vec3 position;
  glm::vec2 tex_coords;
  glm::vec3 normal;
  glm::vec3 tangent;

  static const InputVertexDataInfo* GetVertexDataInfo() {
    static InputVertexDataInfo info{{InputVertexDataInfo::BindingInfo{}}};
    
    info.bindings[0] = {0, sizeof(Vertex3D), {{0, DataFormat::kR32G32B32_SFLOAT, offsetof(Vertex3D, position)},
                                              {1, DataFormat::kR32G32_SFLOAT,    offsetof(Vertex3D, tex_coords)},
                                              {2, DataFormat::kR32G32B32_SFLOAT, offsetof(Vertex3D, normal)},
                                              {3, DataFormat::kR32G32B32_SFLOAT, offsetof(Vertex3D, tangent)}}};

    return &info;
  }
};

}  // namespace vulture
