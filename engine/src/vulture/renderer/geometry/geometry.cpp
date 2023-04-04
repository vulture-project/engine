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

Geometry Geometry::CreateCube() {
  static const glm::vec3 kCubeVertexPositions[] = {
    // front
    glm::vec3(-1.0, -1.0,  1.0),
    glm::vec3( 1.0, -1.0,  1.0),
    glm::vec3( 1.0,  1.0,  1.0),
    glm::vec3(-1.0,  1.0,  1.0),
    // back
    glm::vec3(-1.0, -1.0, -1.0),
    glm::vec3( 1.0, -1.0, -1.0),
    glm::vec3( 1.0,  1.0, -1.0),
    glm::vec3(-1.0,  1.0, -1.0)
  };

  static const uint32_t kCubeIndices[] = {
		// front
		0, 1, 2,
		2, 3, 0,
		// right
		1, 5, 6,
		6, 2, 1,
		// back
		7, 6, 5,
		5, 4, 7,
		// left
		4, 0, 3,
		3, 7, 4,
		// bottom
		4, 5, 1,
		1, 0, 4,
		// top
		3, 2, 6,
		6, 7, 3
  };

  Geometry cube(8, 36);

  for (uint32_t i = 0; i < 8; ++i) {
    cube.GetVertices()[i].position = kCubeVertexPositions[i];
  }

  for (uint32_t i = 0; i < 36; ++i) {
    cube.GetIndices()[i] = kCubeIndices[i];
  }

  return cube;
}