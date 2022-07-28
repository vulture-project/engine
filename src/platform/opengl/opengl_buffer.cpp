/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file opengl_buffer.cpp
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

#include "platform/opengl/opengl_buffer.hpp"

#include <glad/glad.h>

#include "core/logger.hpp"

using namespace vulture;

GLenum GetOpenGLType(BufferDataType type) {
  switch (type)
  {
    case BufferDataType::kFloat:  { return GL_FLOAT; }
    case BufferDataType::kFloat2: { return GL_FLOAT; }
    case BufferDataType::kFloat3: { return GL_FLOAT; }
    case BufferDataType::kFloat4: { return GL_FLOAT; }
    case BufferDataType::kMat3:   { return GL_FLOAT; }
    case BufferDataType::kMat4:   { return GL_FLOAT; }
    case BufferDataType::kInt:    { return GL_INT;   }
    case BufferDataType::kInt2:   { return GL_INT;   }
    case BufferDataType::kInt3:   { return GL_INT;   }
    case BufferDataType::kInt4:   { return GL_INT;   }
    case BufferDataType::kBool:   { return GL_BOOL;  }
    default:                      { assert(!"Invalid BufferDataType for OpenGL!");   }
  }

  return 0;
}

OpenGLVertexBuffer::OpenGLVertexBuffer(const void* data, uint32_t size) {
  GL_CALL(glGenBuffers(1, &id_));
  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, id_));
  GL_CALL(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
}

OpenGLVertexBuffer::~OpenGLVertexBuffer() { GL_CALL(glDeleteBuffers(1, &id_)); }

void OpenGLVertexBuffer::Bind() const { GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, id_)); }

void OpenGLVertexBuffer::Unbind() const { GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0)); }

const VertexBufferLayout& OpenGLVertexBuffer::GetLayout() const { return layout_; }

void OpenGLVertexBuffer::SetLayout(const VertexBufferLayout& layout) { layout_ = layout; }

OpenGLIndexBuffer::OpenGLIndexBuffer(const uint32_t* indices, uint32_t count) : count_(count) {
  GL_CALL(glGenBuffers(1, &id_));

  /*
   * GL_ELEMENT_ARRAY_BUFFER is invalid when no VAO is bound.
   */
  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, id_));
  GL_CALL(glBufferData(GL_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW));
}

OpenGLIndexBuffer::~OpenGLIndexBuffer() { GL_CALL(glDeleteBuffers(1, &id_)); }

void OpenGLIndexBuffer::Bind() const { GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_)); }

void OpenGLIndexBuffer::Unbind() const { GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0)); }

uint32_t OpenGLIndexBuffer::GetCount() const { return count_; }

OpenGLVertexArray::OpenGLVertexArray() { GL_CALL(glGenVertexArrays(1, &id_)); }

OpenGLVertexArray::~OpenGLVertexArray() { GL_CALL(glDeleteVertexArrays(1, &id_)); }

void OpenGLVertexArray::Bind() const { GL_CALL(glBindVertexArray(id_)); }

void OpenGLVertexArray::Unbind() const { GL_CALL(glBindVertexArray(0)); }

void OpenGLVertexArray::SetAttributeLocations(const AttributeLocationMap& locations) {
  GL_CALL(glBindVertexArray(id_));

  for (const auto& vertex_buffer : vertex_buffers_) {
    const VertexBufferLayout& layout = vertex_buffer->GetLayout();
    vertex_buffer->Bind();

    for (const auto& attribute : vertex_buffer->GetLayout().attributes()) {
      const BufferDataTypeSpec data_type_spec = BufferDataTypeSpec::Get(attribute.type);

      auto location_it = locations.find(attribute.name);

      // TODO: Mat3 and Mat4 should be treated otherwise!
      if (location_it != locations.end()) {
        GL_CALL(glEnableVertexAttribArray(location_it->second));
        GL_CALL(glVertexAttribPointer(location_it->second, data_type_spec.components_count,
                                      GetOpenGLType(attribute.type), attribute.normalize, layout.stride(),
                                      reinterpret_cast<const void*>(attribute.offset)));
      } else {
        LOG_WARN(Renderer, "Vertex buffer attribute \"{}\" doesn't have a corresponding attribute location!",
                 attribute.name);
      }
    }

    vertex_buffer->Unbind();
  }
}

void OpenGLVertexArray::AddVertexBuffer(SharedPtr<VertexBuffer> vertex_buffer) {
  Bind();
  vertex_buffer->Bind();
  vertex_buffers_.push_back(vertex_buffer);
  Unbind();
}

void OpenGLVertexArray::SetIndexBuffer(SharedPtr<IndexBuffer> index_buffer) {
  Bind();
  index_buffer->Bind();
  index_buffer_ = index_buffer;
  Unbind();
}

const std::vector<SharedPtr<VertexBuffer>>& OpenGLVertexArray::GetVertexBuffers() const { return vertex_buffers_; }

const IndexBuffer* OpenGLVertexArray::GetIndexBuffer() const { return index_buffer_.get(); }