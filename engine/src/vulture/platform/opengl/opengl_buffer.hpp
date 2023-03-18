/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file opengl_buffer.hpp
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

#include <vulture/renderer/buffer.hpp>

namespace vulture {

class OpenGLVertexBuffer : public VertexBuffer {
 public:
  OpenGLVertexBuffer(const void* data, uint32_t size);

  virtual ~OpenGLVertexBuffer() override;

  virtual void Bind() const override;
  virtual void Unbind() const override;

  virtual const VertexBufferLayout& GetLayout() const override;
  virtual void SetLayout(const VertexBufferLayout& layout) override;

 private:
  uint32_t id_{0};
  VertexBufferLayout layout_;
};

class OpenGLIndexBuffer : public IndexBuffer {
 public:
  OpenGLIndexBuffer(const uint32_t* indices, uint32_t count);

  virtual ~OpenGLIndexBuffer() override;

  virtual void Bind() const override;
  virtual void Unbind() const override;

  virtual uint32_t GetCount() const override;

 private:
  uint32_t id_{0};
  uint32_t count_{0};
};

class OpenGLVertexArray : public VertexArray {
 public:
  OpenGLVertexArray();
  virtual ~OpenGLVertexArray() override;

  virtual void Bind() const override;
  virtual void Unbind() const override;

  virtual void SetAttributeLocations(const AttributeLocationMap& locations) override;

  virtual void AddVertexBuffer(SharedPtr<VertexBuffer> vertex_buffer) override;
  virtual void SetIndexBuffer(SharedPtr<IndexBuffer> index_buffer) override;

  virtual const std::vector<SharedPtr<VertexBuffer>>& GetVertexBuffers() const override;
  virtual const IndexBuffer* GetIndexBuffer() const override;

 private:
  uint32_t id_{0};
  std::vector<SharedPtr<VertexBuffer>> vertex_buffers_;
  SharedPtr<IndexBuffer> index_buffer_{nullptr};
};

}  // namespace vulture