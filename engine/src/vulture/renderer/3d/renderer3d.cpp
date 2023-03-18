/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file renderer3d.cpp
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

#include <vulture/core/logger.hpp>
#include <vulture/renderer/3d/3d_default_shader_names.hpp>
#include <vulture/renderer/3d/renderer3d.hpp>

using namespace vulture;

const int kOutlineWidth = 5;

Renderer3D::~Renderer3D() {
  delete gbuffer_;
}

void Renderer3D::Init() {
  rendererAPI_ = RendererAPI::Create();
  rendererAPI_->Init();

  screen_quad_ = CreateQuad();

  CreateShaders();
  CreateFramebuffers(1, 1);
}

void Renderer3D::SetViewport(const Viewport& viewport) { rendererAPI_->SetViewport(viewport); }

void Renderer3D::OnFramebufferResize(uint32_t width, uint32_t height) {
  gbuffer_->Resize(width, height);

  outline_mask_fb_->Resize(width, height);
  jfa_fbs_[0]->Resize(width, height);
  jfa_fbs_[1]->Resize(width, height);
}

void Renderer3D::RenderScene(Scene3D* scene, Framebuffer* framebuffer, DebugRenderMode render_mode) {
  assert(scene);

  info_.render_mode   = render_mode;
  info_.viewport      = rendererAPI_->GetViewport();
  info_.frame_time_ms = 0.0f;
  info_.fps           = 0.0f;
  info_.draw_calls    = 0;
  info_.materials     = 0;
  info_.meshes        = scene->GetMeshes().size();
  info_.cameras       = scene->GetCameras().size();
  info_.light_sources = scene->GetLightSources().size();

  clock_t time_start = clock();

  if (scene->GetMainCamera() == nullptr) {
    LOG_WARN(Renderer, "No main camera specified for the scene!");
    return;
  }

  DeferredGeometryPass(scene);
  DeferredLightingPass(scene, framebuffer, render_mode);
  
  gbuffer_->BlitDepthAttachment(framebuffer);
  ForwardPass(scene, framebuffer, render_mode);

  /* Post-Processing */
  OutlineMaskPass(scene);
  OutlineSeedsInitPass();
  uint32_t jfa_fb_idx = OutlineJFAPasses();
  OutlineCombinePass(framebuffer, jfa_fb_idx);

  info_.frame_time_ms  = static_cast<float>(clock() - time_start) / CLOCKS_PER_SEC; // in seconds
  info_.fps            = 1 / info_.frame_time_ms;
  info_.frame_time_ms *= 1000.f; // convert to milliseconds
}

void Renderer3D::DeferredGeometryPass(Scene3D* scene) {
  gbuffer_->Bind();
  rendererAPI_->SetViewport(Viewport{0, 0, gbuffer_->GetFramebufferSpec().width, gbuffer_->GetFramebufferSpec().height});

  rendererAPI_->Clear({0, 0, 0, 1});

  for (const auto& mesh : scene->GetMeshes()) {
    for (auto& submesh : mesh->mesh->GetSubmeshes()) {
      auto shader = submesh.GetMaterial()->GetShader();
      if (shader->geometry_pass_ != GeometryPass::kDeferred) {
        continue;
      }

      shader->Bind();
      shader->SetUpPipeline();

      SetUpCamera(scene, shader);

      /* Setting up transformation matrices */
      shader->LoadUniformMat4(scene->GetMainCamera()->CalculateProjectionMatrix(), kUniformNameProjection);
      shader->LoadUniformMat4(scene->GetMainCamera()->CalculateViewMatrix(), kUniformNameView);
      shader->LoadUniformMat4(mesh->transform.CalculateMatrix(), kUniformNameModel);

      /* Setting up material info */
      submesh.GetMaterial()->LoadUniformsToShader();

      /* Drawing */
      rendererAPI_->Draw(*submesh.GetVertexArray());
      ++info_.draw_calls;

      shader->Unbind();
    }
  }

  gbuffer_->Unbind();
}

void Renderer3D::DeferredLightingPass(Scene3D* scene, Framebuffer* framebuffer, DebugRenderMode render_mode) {
  framebuffer->Bind();
  rendererAPI_->SetViewport(Viewport{0, 0, framebuffer->GetFramebufferSpec().width, framebuffer->GetFramebufferSpec().height});

  rendererAPI_->Clear(glm::vec4{0, 0, 0, 1});

  deferred_lighting_shader_->Bind();
  deferred_lighting_shader_->SetUpPipeline();

  SetUpLights(scene, deferred_lighting_shader_.get());
  SetUpCamera(scene, deferred_lighting_shader_.get());

  gbuffer_->BindColorAttachmentAsTexture(0, 0);
  gbuffer_->BindColorAttachmentAsTexture(1, 1);
  gbuffer_->BindColorAttachmentAsTexture(2, 2);

  deferred_lighting_shader_->LoadUniformInt(0, "gPosition");
  deferred_lighting_shader_->LoadUniformInt(1, "gNormal");
  deferred_lighting_shader_->LoadUniformInt(2, "gDiffuseSpec");

  /* Setting up render mode */
  deferred_lighting_shader_->LoadUniformInt(static_cast<int>(render_mode), kUniformNameRenderMode);

  FullscreenDraw();

  framebuffer->Unbind();
}

void Renderer3D::ForwardPass(Scene3D* scene, Framebuffer* framebuffer, DebugRenderMode render_mode) {
  framebuffer->Bind();
  rendererAPI_->SetViewport(Viewport{0, 0, framebuffer->GetFramebufferSpec().width, framebuffer->GetFramebufferSpec().height});

  for (const auto& mesh : scene->GetMeshes()) {
    for (auto& submesh : mesh->mesh->GetSubmeshes()) {
      auto shader = submesh.GetMaterial()->GetShader();
      if (shader->geometry_pass_ != GeometryPass::kForward) {
        continue;
      }

      shader->Bind();
      shader->SetUpPipeline();

      SetUpCamera(scene, shader);
      SetUpLights(scene, shader);

      /* Setting up render mode */
      shader->LoadUniformInt(static_cast<int>(render_mode), kUniformNameRenderMode);

      /* Setting up transformation matrices */
      shader->LoadUniformMat4(scene->GetMainCamera()->CalculateProjectionMatrix(), kUniformNameProjection);
      shader->LoadUniformMat4(scene->GetMainCamera()->CalculateViewMatrix(), kUniformNameView);
      shader->LoadUniformMat4(mesh->transform.CalculateMatrix(), kUniformNameModel);

      /* Setting up material info */
      submesh.GetMaterial()->LoadUniformsToShader();

      /* Drawing */
      rendererAPI_->Draw(*submesh.GetVertexArray());
      ++info_.draw_calls;

      shader->Unbind();
    }
  }

  framebuffer->Unbind();
}

void Renderer3D::OutlineMaskPass(Scene3D* scene) {
  outline_mask_fb_->Bind();
  rendererAPI_->Clear(glm::vec4{0, 0, 0, 0}, false);

  outline_mask_pass_shader_->Bind();
  outline_mask_pass_shader_->SetUpPipeline();

  for (const auto& mesh_node : scene->GetMeshes()) {
    for (auto& submesh : mesh_node->mesh->GetSubmeshes()) {
      if (mesh_node->outlined) {
        SetUpCamera(scene, outline_mask_pass_shader_.get());

        /* Setting up transformation matrices */
        outline_mask_pass_shader_->LoadUniformMat4(scene->GetMainCamera()->CalculateProjectionViewMatrix(), kUniformNameProjectionView);
        outline_mask_pass_shader_->LoadUniformMat4(mesh_node->transform.CalculateMatrix(), kUniformNameModel);

        /* Drawing */
        rendererAPI_->Draw(*submesh.GetVertexArray());
        ++info_.draw_calls;
      }
    }
  }

  outline_mask_pass_shader_->Unbind();
  outline_mask_fb_->Unbind();
}

void Renderer3D::OutlineSeedsInitPass() {
  jfa_fbs_[0]->Bind();
  rendererAPI_->Clear(glm::vec4{0, 0, 0, 0}, false);

  outline_seeds_init_shader_->Bind();
  outline_seeds_init_shader_->SetUpPipeline();
  outline_seeds_init_shader_->LoadUniformInt(0, "u_outline_mask");

  outline_mask_fb_->BindColorAttachmentAsTexture(0, 0);

  FullscreenDraw();
  outline_seeds_init_shader_->Unbind();

  jfa_fbs_[0]->Unbind();
}

uint32_t Renderer3D::OutlineJFAPasses() {
  uint32_t cur_input_buffer_idx = 0;
  uint32_t cur_output_buffer_idx = 1;

  uint32_t passes_count = static_cast<uint32_t>(log2f(static_cast<float>(kOutlineWidth)));

  outline_jfa_pass_shader_->Bind();
  outline_jfa_pass_shader_->SetUpPipeline();

  outline_jfa_pass_shader_->LoadUniformInt(0, "u_sample");
  outline_jfa_pass_shader_->LoadUniformFloat2(
      glm::vec2(1 / static_cast<float>(info_.viewport.width), 1 / static_cast<float>(info_.viewport.height)),
      "u_pixel_size");

  for (uint32_t pass = 0; pass < passes_count; ++pass) {
    int jump_offset = 1 << (passes_count - pass - 1);
    outline_jfa_pass_shader_->LoadUniformFloat(static_cast<float>(jump_offset), "u_jump_offset");

    jfa_fbs_[cur_output_buffer_idx]->Bind();
    rendererAPI_->Clear(glm::vec4(0, 0, 0, 0));

    jfa_fbs_[cur_input_buffer_idx]->BindColorAttachmentAsTexture(0, 0);

    FullscreenDraw();

    std::swap(cur_input_buffer_idx, cur_output_buffer_idx);
  }

  outline_jfa_pass_shader_->Unbind();
  jfa_fbs_[cur_output_buffer_idx]->Unbind();
  std::swap(cur_input_buffer_idx, cur_output_buffer_idx); // to mitigate the last swap in the loop

  return cur_output_buffer_idx;
}

void Renderer3D::OutlineCombinePass(Framebuffer* framebuffer, uint32_t jfa_fb_idx) {
  framebuffer->Bind();

  outline_combining_shader_->Bind();
  outline_combining_shader_->SetUpPipeline();

  outline_combining_shader_->LoadUniformFloat(kOutlineWidth, "u_outline_width");
  outline_combining_shader_->LoadUniformFloat3(glm::vec3(1.0f, 0.7f, 0.0f), "u_outline_color");

  outline_combining_shader_->LoadUniformInt(0, "u_outline_mask");
  outline_mask_fb_->BindColorAttachmentAsTexture(0, 0);
  
  outline_combining_shader_->LoadUniformInt(1, "u_sample");
  jfa_fbs_[jfa_fb_idx]->BindColorAttachmentAsTexture(0, 1);

  FullscreenDraw();

  outline_combining_shader_->Unbind();
  framebuffer->Unbind();
}

void Renderer3D::FullscreenDraw() {
  rendererAPI_->Draw(*screen_quad_);
}

static void LoadTranslation(Shader* shader, const std::string& name, const Transform& transform) {
  shader->LoadUniformFloat3(transform.translation, "{}.{}", name, kStructMemberNameWSPosition);
}

static void LoadDirection(Shader* shader, const std::string& name, const Transform& transform) {
  glm::vec3 ws_direction = transform.CalculateRotationMatrix() * glm::vec4(kDefaultForwardVector, 1.0);
  shader->LoadUniformFloat3(ws_direction, "{}.{}", name, kStructMemberNameWSDirection);
}

static void LoadLightColorSpecs(Shader* shader, const std::string& name,
                                const LightColorSpecs& specs) {
  shader->LoadUniformFloat3(specs.ambient, "{}.{}", name, kStructMemberNameAmbientColor);
  shader->LoadUniformFloat3(specs.diffuse, "{}.{}", name, kStructMemberNameDiffuseColor);
  shader->LoadUniformFloat3(specs.specular, "{}.{}", name, kStructMemberNameSpecularColor);
}

static void LoadLightAttenuationSpecs(Shader* shader, const std::string& name,
                                      const LightAttenuationSpecs& specs) {
  shader->LoadUniformFloat(specs.linear, "{}.{}", name, kStructMemberNameAttenuationLinear);
  shader->LoadUniformFloat(specs.quadratic, "{}.{}", name, kStructMemberNameAttenuationQuadratic);
}

static void LoadDirectionalLight(Shader* shader, const std::string& name,
                                 const DirectionalLightSpecs& specs, const Transform& transform) {
  LoadDirection(shader, name, transform);
  LoadLightColorSpecs(shader, name, specs.color_specs);
}

static void LoadPointLight(Shader* shader, const std::string& name, const PointLightSpecs& specs,
                           const Transform& transform) {
  LoadTranslation(shader, name, transform);
  LoadLightColorSpecs(shader, name, specs.color_specs);
  LoadLightAttenuationSpecs(shader, name, specs.attenuation_specs);
}

static void LoadSpotLight(Shader* shader, const std::string& name, const SpotLightSpecs& specs,
                          const Transform& transform) {
  LoadTranslation(shader, name, transform);
  LoadDirection(shader, name, transform);

  LoadLightColorSpecs(shader, name, specs.color_specs);
  LoadLightAttenuationSpecs(shader, name, specs.attenuation_specs);

  shader->LoadUniformFloat(specs.inner_cone_cosine, "{}.{}", name, kStructMemberNameSpotInnerConeCosine);
  shader->LoadUniformFloat(specs.outer_cone_cosine, "{}.{}", name, kStructMemberNameSpotOuterConeCosine);
}

void Renderer3D::CreateShaders() {
  deferred_lighting_shader_ = Shader::Create("assets/shaders/deferred_lighting.glsl");
  deferred_lighting_shader_->cull_mode_          = CullMode::kNone;
  deferred_lighting_shader_->enable_depth_test_  = false;
  deferred_lighting_shader_->enable_depth_write_ = false;
  deferred_lighting_shader_->enable_blending_    = false;

  /* Outline Shaders */
  outline_mask_pass_shader_                      = Shader::Create("assets/shaders/outlining/mask_pass.glsl");
  outline_mask_pass_shader_->enable_depth_test_  = false;
  outline_mask_pass_shader_->enable_blending_    = false;

  outline_seeds_init_shader_                     = Shader::Create("assets/shaders/outlining/seeds_init.glsl");
  outline_seeds_init_shader_->enable_depth_test_ = false;
  outline_seeds_init_shader_->enable_blending_   = false;

  outline_jfa_pass_shader_                       = Shader::Create("assets/shaders/outlining/jfa_pass.glsl");
  outline_jfa_pass_shader_->enable_depth_test_   = false;
  outline_jfa_pass_shader_->enable_blending_     = false;

  outline_combining_shader_                      = Shader::Create("assets/shaders/outlining/combining.glsl");
  outline_combining_shader_->enable_depth_test_  = false;
  outline_combining_shader_->enable_blending_    = false;
}

void Renderer3D::CreateFramebuffers(uint32_t width, uint32_t height) {
  FramebufferSpec gbuffer_spec{};
  gbuffer_spec.width       = width;
  gbuffer_spec.height      = height;
  gbuffer_spec.attachments = {FramebufferAttachmentFormat::kRGBA32F, //  Position
                              FramebufferAttachmentFormat::kRGBA32F, //  Normal
                              FramebufferAttachmentFormat::kRGBA8,   //  Diffuse (RGB), Specular (A)
                              FramebufferAttachmentFormat::kDepth24Stencil8};
  gbuffer_ = Framebuffer::Create(gbuffer_spec);

  /* Outline Framebuffers */
  FramebufferSpec outline_mask_fb_spec{};
  outline_mask_fb_spec.width       = width;
  outline_mask_fb_spec.height      = height;
  outline_mask_fb_spec.attachments = {FramebufferAttachmentFormat::kRGBA8};
  outline_mask_fb_                 = Framebuffer::Create(outline_mask_fb_spec);

  FramebufferSpec jfa_fbs_spec{};
  jfa_fbs_spec.width       = width;
  jfa_fbs_spec.height      = height;
  jfa_fbs_spec.attachments = {FramebufferAttachmentFormat::kRGBA8};
  jfa_fbs_[0]              = Framebuffer::Create(jfa_fbs_spec);
  jfa_fbs_[1]              = Framebuffer::Create(jfa_fbs_spec);
}

void Renderer3D::SetUpCamera(Scene3D* scene, Shader* shader) {
  assert(scene->GetMainCamera());
  shader->LoadUniformFloat3(scene->GetMainCamera()->transform.translation, kUniformNameWSCamera);
}

void Renderer3D::SetUpLights(Scene3D* scene, Shader* shader) {
  int32_t idx_directional = 0;
  int32_t idx_point = 0;
  int32_t idx_spot = 0;

  for (const auto& light_node : scene->GetLightSources()) {
    if (!light_node->IsEnabled()) {
      continue;
    }

    const auto& light_specs = light_node->GetLightSpecs();
    const auto& transform = light_node->transform;

    switch (light_node->GetType()) {
      case LightType::kDirectional: {
        LoadDirectionalLight(shader, fmt::format("{}[{}]", kUniformNameDirectionalLights, idx_directional),
                             light_specs.directional, transform);
        ++idx_directional;
        break;
      }

      case LightType::kPoint: {
        LoadPointLight(shader, fmt::format("{}[{}]", kUniformNamePointLights, idx_point), light_specs.point, transform);
        ++idx_point;
        break;
      }

      case LightType::kSpot: {
        LoadSpotLight(shader, fmt::format("{}[{}]", kUniformNameSpotLights, idx_spot), light_specs.spot, transform);
        ++idx_spot;
        break;
      }

      default: {
        assert(!"Invalid light source type!");
      }
    }
  }

  shader->LoadUniformInt(idx_directional, kUniformNameDirectionalLightsCount);
  shader->LoadUniformInt(idx_point, kUniformNamePointLightsCount);
  shader->LoadUniformInt(idx_spot, kUniformNameSpotLightsCount);
}

Renderer3D::Info Renderer3D::GetInfo() {
  return info_;
}

RendererAPI* Renderer3D::GetRendererAPI() {
  return rendererAPI_.get();
}