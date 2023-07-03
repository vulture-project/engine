/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file mesh.hpp
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

#include <vulture/renderer/geometry/geometry.hpp>
#include <vulture/renderer/graphics_api/render_device.hpp>
#include <vulture/renderer/material_system/material.hpp>

namespace vulture {

/************************************************************************************************
 * SUBMESH
 ************************************************************************************************/
class Submesh {
public:
  Submesh() = default;
  Submesh(uint32_t vertex_count, uint32_t index_count, SharedPtr<Material> material = nullptr, bool dynamic = false);
  Submesh(const Geometry& geometry, SharedPtr<Material> material = nullptr, bool dynamic = false);
  ~Submesh();

  Submesh(Submesh&& other);
  Submesh& operator=(Submesh&& other);

  Submesh(const Submesh& other) = delete;
  Submesh& operator=(const Submesh& other) = delete;

  Geometry& GetGeometry();
  const Geometry& GetGeometry() const;

  void UpdateDeviceBuffers(RenderDevice& device);
  BufferHandle GetVertexBuffer() const;
  BufferHandle GetIndexBuffer() const;

  void SetMaterial(SharedPtr<Material> material);
  Material& GetMaterial() const;

private:
  void DeleteBuffers();

private:
  RenderDevice* device_{nullptr};

  Geometry geometry_{};

  bool dynamic_{false};
  BufferHandle vertex_buffer_{kInvalidRenderResourceHandle};
  BufferHandle index_buffer_{kInvalidRenderResourceHandle};

  mutable SharedPtr<Material> material_{nullptr};
};

/************************************************************************************************
 * MESH
 ************************************************************************************************/
class Mesh : public IAsset {
public:
  Mesh() = default;
  Mesh(RenderDevice& device, const Geometry& geometry, SharedPtr<Material> material, bool dynamic = false);

  Vector<Submesh>& GetSubmeshes();
  const Vector<Submesh>& GetSubmeshes() const;

  void CalculateBoundingBox();
  const AABB& GetBoundingBox() const;

  void UpdateDeviceBuffers(RenderDevice& device);

private:
  Vector<Submesh> submeshes_;

  AABB bounding_box_{};
};

}  // namespace vulture
