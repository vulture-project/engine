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

#include <vulture/core/core.hpp>
#include <vulture/renderer/3d/scene3d.hpp>
#include <vulture/renderer/framebuffer.hpp>
#include <vulture/renderer/renderer_api.hpp>
#include <vulture/renderer/shader.hpp>

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

  struct Info {
    DebugRenderMode render_mode{DebugRenderMode::kDefault};
    Viewport viewport{};
    float frame_time_ms{0};
    float fps{0};

    uint32_t draw_calls{0};
    uint32_t materials{0}; // Fixme: (tralf-strues) not supported for now, always 0

    uint32_t meshes{0};
    uint32_t cameras{0};
    uint32_t light_sources{0};
  };

 public:
  ~Renderer3D();

  void Init();
  void SetViewport(const Viewport& viewport);
  void OnFramebufferResize(uint32_t width, uint32_t height);
  void RenderScene(Scene3D* scene, Framebuffer* framebuffer, DebugRenderMode render_mode = DebugRenderMode::kDefault);
  Info GetInfo();

  RendererAPI* GetRendererAPI();

 private:
  void CreateShaders();
  void CreateFramebuffers(uint32_t width, uint32_t height);

  void SetUpCamera(Scene3D* scene, Shader* shader);
  void SetUpLights(Scene3D* scene, Shader* shader);

  void DeferredGeometryPass(Scene3D* scene);
  void DeferredLightingPass(Scene3D* scene, Framebuffer* framebuffer, DebugRenderMode render_mode);

  void ForwardPass(Scene3D* scene, Framebuffer* framebuffer, DebugRenderMode render_mode);

  void OutlineMaskPass(Scene3D* scene);
  void OutlineSeedsInitPass();
  uint32_t OutlineJFAPasses();
  void OutlineCombinePass(Framebuffer* framebuffer, uint32_t jfa_fb_idx);

  void FullscreenDraw();

 private:
  ScopePtr<RendererAPI> rendererAPI_;
  Info info_;

 private:
  SharedPtr<VertexArray> screen_quad_;

  SharedPtr<Shader>      deferred_lighting_shader_;
  Framebuffer*           gbuffer_;

  SharedPtr<Shader>      outline_mask_pass_shader_;
  SharedPtr<Shader>      outline_seeds_init_shader_;
  SharedPtr<Shader>      outline_jfa_pass_shader_;
  SharedPtr<Shader>      outline_combining_shader_;
  Framebuffer*           outline_mask_fb_;
  Framebuffer*           jfa_fbs_[2];
};

}  // namespace vulture