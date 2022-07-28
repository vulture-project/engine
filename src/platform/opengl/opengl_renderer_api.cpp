/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file opengl_renderer_api.cpp
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

#include "platform/opengl/opengl_renderer_api.hpp"

using namespace vulture;

void OpenGLRendererAPI::Init() {
  // TODO:
  GL_CALL(glEnable(GL_DEPTH_TEST));
  GL_CALL(glDepthFunc(GL_LEQUAL)); // FIXME: only needed for some types of objects (e.g. skyboxes)
}

void OpenGLRendererAPI::SetViewport(const Viewport& viewport) {
  GL_CALL(glViewport(viewport.x, viewport.y, viewport.width, viewport.height));
  viewport_ = viewport;
}

Viewport OpenGLRendererAPI::GetViewport() const { return viewport_; }

void OpenGLRendererAPI::Clear(const glm::vec4& color) {
  GL_CALL(glClearColor(color.r, color.g, color.b, color.a));
  GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void OpenGLRendererAPI::Draw(const VertexArray& vertexArray) {
  vertexArray.Bind();
  GL_CALL(glDrawElements(GL_TRIANGLES, vertexArray.GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr));
  vertexArray.Unbind();
}