/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file mesh.hpp
 * @date 2022-04-28
 *
 * The MIT License (MIT)
 * Copyright (c) vulture-project
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

#include <vector>

#include "core/core.hpp"
#include "renderer/buffer.hpp"
#include "renderer/material.hpp"

namespace vulture {

class Submesh {
 public:
  Submesh() = default;
  Submesh(SharedPtr<VertexArray> vertex_array, SharedPtr<Material> material)
      : vertex_array_(vertex_array), material_(material) {
    assert(material_->GetShader());
    vertex_array_->SetAttributeLocations(material_->GetShader()->GetAttributeLocations());
  }

  VertexArray* GetVertexArray() { return vertex_array_.get(); }
  Material* GetMaterial() { return material_.get(); }

 private:
  SharedPtr<VertexArray> vertex_array_{nullptr};
  SharedPtr<Material> material_{nullptr};
};

class Mesh {
 public:
  Mesh() = default;

  Mesh(SharedPtr<VertexArray> vertex_array, SharedPtr<Material> material) {
    assert(material->GetShader());
    submeshes_.emplace_back(vertex_array, material);
  }

  std::vector<Submesh>& GetSubmeshes() { return submeshes_; }

 private:
  std::vector<Submesh> submeshes_;
};

/**
 * @brief Create a skybox from 6 images.
 *
 * The order of side images is the following:
 * 1) Right
 * 2) Left
 * 3) Top
 * 4) Bottom
 * 5) Front
 * 6) Back
 *
 * @param faces_filenames
 * @return SharedPtr<Mesh>
 */
SharedPtr<Mesh> CreateSkyboxMesh(const std::array<std::string, 6>& faces_filenames);

}  // namespace vulture