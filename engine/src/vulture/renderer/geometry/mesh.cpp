/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file mesh.cpp
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

#include <vulture/renderer/geometry/mesh.hpp>

using namespace vulture;

/************************************************************************************************
 * SUBMESH
 ************************************************************************************************/
Submesh::Submesh(uint32_t vertex_count, uint32_t index_count, SharedPtr<Material> material, bool dynamic)
    : geometry_(vertex_count, index_count), dynamic_(dynamic), material_(material) {
  VULTURE_ASSERT(!dynamic_, "Dynamic meshes are not supported at the moment");
}

Submesh::Submesh(const Geometry& geometry, SharedPtr<Material> material, bool dynamic)
    : geometry_(geometry), dynamic_(dynamic), material_(material) {
  VULTURE_ASSERT(!dynamic_, "Dynamic meshes are not supported at the moment");
  /* TODO: (tralf-strues) dynamic buffers */
}

Submesh::~Submesh() {
  DeleteBuffers();
}

Submesh::Submesh(Submesh&& other)
    : device_(other.device_),
      geometry_(std::move(other.geometry_)),
      dynamic_(other.dynamic_),
      vertex_buffer_(other.vertex_buffer_),
      index_buffer_(other.index_buffer_),
      material_(other.material_) {
  other.vertex_buffer_ = kInvalidRenderResourceHandle;
  other.index_buffer_  = kInvalidRenderResourceHandle;
  other.material_      = nullptr;
}

Submesh& Submesh::operator=(Submesh&& other) {
  if (this != &other) {
    DeleteBuffers();

    device_        = other.device_;
    geometry_      = std::move(other.geometry_);
    dynamic_       = other.dynamic_;
    vertex_buffer_ = other.vertex_buffer_;
    index_buffer_  = other.index_buffer_;
    material_      = other.material_;
    
    other.vertex_buffer_ = kInvalidRenderResourceHandle;
    other.index_buffer_  = kInvalidRenderResourceHandle;
    other.material_      = nullptr;
  }

  return *this;
}

void Submesh::DeleteBuffers() {
  if (ValidRenderHandle(vertex_buffer_)) {
    VULTURE_ASSERT(device_, "Vertex buffer created without a valid RenderDevice");
    device_->DeleteBuffer(vertex_buffer_);
  }

  if (ValidRenderHandle(index_buffer_)) {
    VULTURE_ASSERT(device_, "Index buffer created without a valid RenderDevice");
    device_->DeleteBuffer(index_buffer_);
  }
}

Geometry& Submesh::GetGeometry() { return geometry_; }
const Geometry& Submesh::GetGeometry() const { return geometry_; }

void Submesh::UpdateDeviceBuffers(RenderDevice& device) {
  if ((device_ != nullptr) && (device_ != &device)) {
    DeleteBuffers();
  }

  device_ = &device;

  uint32_t vertex_count = geometry_.GetVertices().size();
  if (!ValidRenderHandle(vertex_buffer_)) {
    vertex_buffer_ = device_->CreateStaticVertexBuffer<Vertex3D>(vertex_count);
  }

  device_->LoadBufferData<Vertex3D>(vertex_buffer_, 0, vertex_count, geometry_.GetVertices().data());

  uint32_t index_count = geometry_.GetIndices().size();
  if (!ValidRenderHandle(index_buffer_)) {
    index_buffer_ = device_->CreateStaticIndexBuffer(geometry_.GetIndices().size());
  }

  device_->LoadBufferData<uint32_t>(index_buffer_, 0, index_count, geometry_.GetIndices().data());
}

BufferHandle Submesh::GetVertexBuffer() const { return vertex_buffer_; }
BufferHandle Submesh::GetIndexBuffer() const { return index_buffer_; }

void Submesh::SetMaterial(SharedPtr<Material> material) { material_ = material; }

Material& Submesh::GetMaterial() const {
  VULTURE_ASSERT(material_, "Material is not set!");
  return *material_.get();
}

/************************************************************************************************
 * MESH
 ************************************************************************************************/
Mesh::Mesh(RenderDevice& device, const Geometry& geometry, SharedPtr<Material> material, bool dynamic) {
  Submesh& submesh = submeshes_.emplace_back(geometry, material, dynamic);
  submesh.UpdateDeviceBuffers(device);
}

Vector<Submesh>& Mesh::GetSubmeshes() { return submeshes_; }
const Vector<Submesh>& Mesh::GetSubmeshes() const { return submeshes_; }

void Mesh::CalculateBoundingBox() {
  /* TODO: (tralf-strues) */
  VULTURE_ASSERT(false, "Not implemented");
}

const AABB& Mesh::GetBoundingBox() const {
  VULTURE_ASSERT(false, "Not implemented");
  return bounding_box_;
}

void Mesh::UpdateDeviceBuffers(RenderDevice& device) {
  for (auto& submesh : submeshes_) {
    submesh.UpdateDeviceBuffers(device);
  }
}