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

#include "core.hpp"
#include "renderer/buffer.hpp"
#include "renderer/material.hpp"

namespace vulture {

class Mesh {
 public:
  Mesh() = default;
  Mesh(const SharedPtr<VertexArray>& vertex_array, const SharedPtr<Material>& material)
      : vertex_array_(vertex_array), material_(material) {
    assert(material_->GetShader());
    vertex_array_->SetAttributeLocations(material_->GetShader()->GetAttributeLocations());
  }

  /* FIXME: Change return value! */
  SharedPtr<VertexArray> GetVertexArray() { return vertex_array_; }
  SharedPtr<Material> GetMaterial() { return material_; }

 private:
  SharedPtr<VertexArray> vertex_array_{nullptr};
  SharedPtr<Material> material_{nullptr};
};

}  // namespace vulture