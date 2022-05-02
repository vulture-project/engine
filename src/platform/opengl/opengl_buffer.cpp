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

#include <glad/glad.h>

#include "platform/opengl/opengl_buffer.hpp"

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
    default:                      { assert(false);   }
  }

  return 0;
}

OpenGLVertexBuffer::OpenGLVertexBuffer(void* data, uint32_t size) {
  glGenBuffers(1, &id_);
  glBindBuffer(GL_ARRAY_BUFFER, id_);
  glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

OpenGLVertexBuffer::~OpenGLVertexBuffer() { glDeleteBuffers(1, &id_); }

void OpenGLVertexBuffer::Bind() const { glBindBuffer(GL_ARRAY_BUFFER, id_); }

void OpenGLVertexBuffer::Unbind() const { glBindBuffer(GL_ARRAY_BUFFER, 0); }

const VertexBufferLayout& OpenGLVertexBuffer::GetLayout() const { return layout_; }

void OpenGLVertexBuffer::SetLayout(const VertexBufferLayout& layout) { layout_ = layout; }

OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* indices, uint32_t count) : count_(count) {
  glGenBuffers(1, &id_);

  /*
   * GL_ELEMENT_ARRAY_BUFFER is invalid when no VAO is bound.
   */
  glBindBuffer(GL_ARRAY_BUFFER, id_);
  glBufferData(GL_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
}

OpenGLIndexBuffer::~OpenGLIndexBuffer() { glDeleteBuffers(1, &id_); }

void OpenGLIndexBuffer::Bind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_); }

void OpenGLIndexBuffer::Unbind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }

uint32_t OpenGLIndexBuffer::GetCount() const { return count_; }

OpenGLVertexArray::OpenGLVertexArray() { glGenVertexArrays(1, &id_); }

OpenGLVertexArray::~OpenGLVertexArray() { glDeleteVertexArrays(1, &id_); }

void OpenGLVertexArray::Bind() const { glBindVertexArray(id_); }

void OpenGLVertexArray::Unbind() const { glBindVertexArray(0); }

void OpenGLVertexArray::AddVertexBuffer(const SharedPtr<VertexBuffer>& vertexBuffer) {
  glBindVertexArray(id_);
  vertexBuffer->Bind();

  const VertexBufferLayout& layout = vertexBuffer->GetLayout();
  for (const auto& attribute : layout.attributes()) {
    const BufferDataTypeSpec dataTypeSpec = BufferDataTypeSpec::Get(attribute.type);

    glEnableVertexAttribArray(vertex_attrib_number_);
    glVertexAttribPointer(vertex_attrib_number_, dataTypeSpec.components_count, GetOpenGLType(attribute.type),
                          attribute.normalize, layout.stride(), reinterpret_cast<const void*>(attribute.offset));

    ++vertex_attrib_number_;
  }

  vertex_buffers_.push_back(vertexBuffer);
}

void OpenGLVertexArray::SetIndexBuffer(const SharedPtr<IndexBuffer>& indexBuffer) {
  glBindVertexArray(id_);
  indexBuffer->Bind();
  index_buffer_ = indexBuffer;
}

const std::vector<SharedPtr<VertexBuffer>>& OpenGLVertexArray::GetVertexBuffers() const { return vertex_buffers_; }

const SharedPtr<IndexBuffer>& OpenGLVertexArray::GetIndexBuffer() const { return index_buffer_; }