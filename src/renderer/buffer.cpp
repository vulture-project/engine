/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file buffer.cpp
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
#include "renderer/renderer_api.hpp"

using namespace vulture;

const BufferDataTypeSpec BufferDataTypeSpec::Get(BufferDataType type) {
  return kShaderDataTypeSpecs[static_cast<uint8_t>(type)];
}

SharedPtr<VertexBuffer> VertexBuffer::Create(const void* data, uint32_t size) {
  switch (RendererAPI::GetAPI()) {
    case RendererAPI::API::kOpenGL: { return CreateShared<OpenGLVertexBuffer>(data, size); }
    default:                        { assert(!"Unsupported RendererAPI"); }
  }

  return nullptr;
}

SharedPtr<IndexBuffer> IndexBuffer::Create(const uint32_t* indices, uint32_t count) {
  switch (RendererAPI::GetAPI()) {
    case RendererAPI::API::kOpenGL: { return CreateShared<OpenGLIndexBuffer>(indices, count); }
    default:                        { assert(!"Unsupported RendererAPI"); }
  }

  return nullptr;
}

SharedPtr<VertexArray> VertexArray::Create() {
  switch (RendererAPI::GetAPI()) {
    case RendererAPI::API::kOpenGL: { return CreateShared<OpenGLVertexArray>(); }
    default:                        { assert(!"Unsupported RendererAPI"); }
  }

  return nullptr;
}