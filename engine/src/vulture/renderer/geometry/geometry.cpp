/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file geometry.cpp
 * @date 2023-03-19
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

#include <vulture/renderer/geometry/geometry.hpp>

using namespace vulture;

AABB::AABB(const glm::vec3& min, const glm::vec3& max) : min(min), max(max) {}

Geometry::Geometry(uint32_t vertex_count, uint32_t index_count) {
  vertices_.resize(vertex_count);
  indices_.resize(index_count);
}

Vector<Vertex3D>& Geometry::GetVertices() { return vertices_; }
const Vector<Vertex3D>& Geometry::GetVertices() const { return vertices_; }

Vector<uint32_t>& Geometry::GetIndices() { return indices_; }
const Vector<uint32_t>& Geometry::GetIndices() const { return indices_; }

void Geometry::CalculateBoundingBox() {
  for (const auto& vertex : GetVertices()) {
    bounding_box_.min.x = std::min(bounding_box_.min.x, vertex.position.x);
    bounding_box_.min.y = std::min(bounding_box_.min.y, vertex.position.y);
    bounding_box_.min.z = std::min(bounding_box_.min.z, vertex.position.z);

    bounding_box_.max.x = std::max(bounding_box_.max.x, vertex.position.x);
    bounding_box_.max.y = std::max(bounding_box_.max.y, vertex.position.y);
    bounding_box_.max.z = std::max(bounding_box_.max.z, vertex.position.z);
  }
}

const AABB& Geometry::GetBoundingBox() const { return bounding_box_; }