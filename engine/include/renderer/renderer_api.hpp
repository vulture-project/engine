/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file renderer_api.hpp
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

#include "core/core.hpp"
#include "renderer/buffer.hpp"

namespace vulture {

struct Viewport {
  uint32_t x{0};
  uint32_t y{0};
  uint32_t width{0};
  uint32_t height{0};
};

class RendererAPI {
 public:
  enum class API { kInvalid = 0, kOpenGL };

  static ScopePtr<RendererAPI> Create();
  static API GetAPI();
  static void SetAPI(API api);

 public:
  virtual ~RendererAPI() = default;

  virtual void Init() = 0;

  virtual void SetViewport(const Viewport& viewport) = 0;
  virtual Viewport GetViewport() const = 0;

  virtual void Clear(const glm::vec4& color, bool clear_depth = true) = 0;
  virtual void Draw(const VertexArray& vertex_array) = 0;

 private:
  static API api_;
};

}  // namespace vulture