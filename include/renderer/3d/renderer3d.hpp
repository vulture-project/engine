/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file renderer3d.hpp
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
#include "renderer/renderer_api.hpp"
#include "renderer/shader.hpp"
#include "renderer/3d/scene3d.hpp"

namespace vulture {

class Renderer3D {
 public:
  enum class DebugRenderMode {
    kDefault   = 0,
    kNormals   = 1,
    kNormalMap = 2,
    kTangents  = 3,
    kDepth     = 4,

    kTotal
  };

 public:
  static void Init();
  static void SetViewport(const Viewport& viewport);
  static void RenderScene(Scene3D* scene, DebugRenderMode render_mode = DebugRenderMode::kDefault);

 private:
  static void SetUpCamera(Scene3D* scene, Shader* shader);
  static void SetUpLights(Scene3D* scene, Shader* shader);

 private:
  static ScopePtr<RendererAPI> rendererAPI_;
};

}  // namespace vulture