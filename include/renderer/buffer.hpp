/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file buffer.hpp
 * @date 2022-04-27
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

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "core/core.hpp"

namespace vulture {

//================================================================
// BufferDataType
//================================================================
enum class BufferDataType : uint8_t {
  kInvalid = 0,
  kFloat,
  kFloat2,
  kFloat3,
  kFloat4,
  kInt,
  kInt2,
  kInt3,
  kInt4,
  kMat3,
  kMat4,
  kBool,

  kTotal
};

struct BufferDataTypeSpec {
  BufferDataType type{BufferDataType::kInvalid};
  const char* typeStr{nullptr};
  uint32_t size{0};
  uint32_t components_count{0};

  static const BufferDataTypeSpec Get(BufferDataType type);
};

const BufferDataTypeSpec kShaderDataTypeSpecs[] = {
    {BufferDataType::kInvalid, "Invalid", 0, 0},

    {BufferDataType::kFloat,  "Float",  1 * sizeof(float), 1},
    {BufferDataType::kFloat2, "Float2", 2 * sizeof(float), 2},
    {BufferDataType::kFloat3, "Float3", 3 * sizeof(float), 3},
    {BufferDataType::kFloat4, "Float4", 4 * sizeof(float), 4},

    {BufferDataType::kInt,  "Int",  1 * sizeof(int32_t), 1},
    {BufferDataType::kInt2, "Int2", 2 * sizeof(int32_t), 2},
    {BufferDataType::kInt3, "Int3", 3 * sizeof(int32_t), 3},
    {BufferDataType::kInt4, "Int4", 4 * sizeof(int32_t), 4},

    {BufferDataType::kMat3, "Mat3", 3 * 3 * sizeof(float), 3},
    {BufferDataType::kMat4, "Mat4", 4 * 4 * sizeof(float), 4},

    {BufferDataType::kBool, "Bool", 1 * sizeof(bool), 1},
};

//================================================================
// Vertex Buffer Layout
//================================================================
/**
 * @brief Layout specification of a vertex attribute in a @ref VertexBuffer.
 */
struct VertexBufferAttributeSpec {
  std::string name;
  BufferDataType type{BufferDataType::kInvalid};
  uint32_t offset{0};     ///< Sets automatically by @ref VertexBufferLayout.
  bool normalize{false};  ///< Normalize integers to floats in range [0.0, 1.0]

  VertexBufferAttributeSpec() = default;

  VertexBufferAttributeSpec(BufferDataType type, const std::string& name, bool normalize = false)
      : name(name), type(type), offset(0), normalize(normalize) {}

  uint32_t getSize() const { return kShaderDataTypeSpecs[static_cast<uint8_t>(type)].size; }

  uint32_t getComponentsCount() const { return kShaderDataTypeSpecs[static_cast<uint8_t>(type)].components_count; }
};

/**
 * @brief Specifies the layout of all attributes inside a single @ref
 * VertexBuffer.
 *
 * Suppose each vertex contains position, color and normal with the following
 * layout
 * -------------------------------------------|------------------------------------------|------
 * | pos0       | color0         | normal0    | pos1       | color1         |
 * normal1    | ... |
 * -------------------------------------------|------------------------------------------|------
 * |_________________________________________|
 * |________________________________________| |_____ vertex0 (stride=40) vertex1
 * (stride=40) Attributes:
 *     {
 *         name: "pos",
 *         type: Float3,
 *         offset: 0,
 *         normalize: false
 *     },
 *     {
 *         name: "color",
 *         type: Int4,
 *         offset: 12,
 *         normalize: true
 *     },
 *     {
 *         name: "normal",
 *         type: Float3,
 *         offset: 28,
 *         normalize: false
 *     }
 */
class VertexBufferLayout {
 public:
  VertexBufferLayout() = default;

  VertexBufferLayout(std::initializer_list<VertexBufferAttributeSpec> attributes) : attributes_(attributes) {
    ComputeAttrOffsetsAndStride();
  }

  uint32_t stride() const { return stride_; }
  const std::vector<VertexBufferAttributeSpec>& attributes() const { return attributes_; }

 private:
  void ComputeAttrOffsetsAndStride() {
    size_t cur_offset = 0;

    for (auto& attribute : attributes_) {
      attribute.offset = cur_offset;
      cur_offset += attribute.getSize();
    }

    stride_ = cur_offset;
  }

 private:
  std::vector<VertexBufferAttributeSpec> attributes_;
  uint32_t stride_{0};
};

//================================================================
// VertexBuffer and IndexBuffer
//================================================================
/**
 * @brief Buffer containing some per-vertex info.
 *
 * Can be used to store one or several per-vertex attributes (@see
 * VertexBufferAttributeSpec). The layout of the attributes in the
 * buffer must be specified with a @ref VertexBufferLayout.
 */
class VertexBuffer {
 public:
  static SharedPtr<VertexBuffer> Create(void* data, uint32_t size);

 public:
  virtual ~VertexBuffer() = default;

  virtual void Bind() const = 0;
  virtual void Unbind() const = 0;

  virtual const VertexBufferLayout& GetLayout() const = 0;
  virtual void SetLayout(const VertexBufferLayout& layout) = 0;
};

class IndexBuffer {
 public:
  static SharedPtr<IndexBuffer> Create(uint32_t* indices, uint32_t count);

 public:
  virtual ~IndexBuffer() = default;

  virtual void Bind() const = 0;
  virtual void Unbind() const = 0;

  virtual uint32_t GetCount() const = 0;
};

//================================================================
// VertexArray
//================================================================
using AttributeLocationMap = std::map<std::string, uint32_t>;

class VertexArray {
 public:
  static SharedPtr<VertexArray> Create();

 public:
  virtual ~VertexArray() = default;

  virtual void Bind() const = 0;
  virtual void Unbind() const = 0;

  virtual void SetAttributeLocations(const AttributeLocationMap& locations) = 0;

  /**
   * @brief Adds a completely set vertex buffer to the vertex array.
   *
   * @param vertex_buffer
   *
   * @warning The vertex buffer to be added must have a layout!
   */
  virtual void AddVertexBuffer(SharedPtr<VertexBuffer> vertex_buffer) = 0;

  virtual void SetIndexBuffer(SharedPtr<IndexBuffer> index_buffer) = 0;

  virtual const std::vector<SharedPtr<VertexBuffer>>& GetVertexBuffers() const = 0;
  virtual const IndexBuffer* GetIndexBuffer() const = 0;
};

}  // namespace vulture