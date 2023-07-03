/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file geometry.hpp
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

#include <vulture/core/core.hpp>
#include <vulture/renderer/geometry/vertex_formats.hpp>

namespace vulture {

struct AABB {
  glm::vec3 min{0.0f};
  glm::vec3 max{0.0f};

  AABB() = default;
  AABB(const glm::vec3& min, const glm::vec3& max);
};

class Geometry {
 public:
  Geometry(uint32_t vertex_count = 0, uint32_t index_count = 0);

  Vector<Vertex3D>& GetVertices();
  const Vector<Vertex3D>& GetVertices() const;

  Vector<uint32_t>& GetIndices();
  const Vector<uint32_t>& GetIndices() const;

  void CalculateBoundingBox();
  const AABB& GetBoundingBox() const;

  static Geometry CreateCube();

 private:
  Vector<Vertex3D> vertices_;
  Vector<uint32_t> indices_;

  AABB bounding_box_{};
};

}  // namespace vulture
