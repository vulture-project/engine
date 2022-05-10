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

#include "core/logger.hpp"
#include "renderer/3d/renderer3d.hpp"

#include "renderer/3d/3d_default_shader_names.hpp"

using namespace vulture;

ScopePtr<RendererAPI> Renderer3D::rendererAPI_;

void Renderer3D::Init() {
  rendererAPI_ = RendererAPI::Create();
  rendererAPI_->Init();
}

void Renderer3D::SetViewport(const Viewport& viewport) { rendererAPI_->SetViewport(viewport); }

void Renderer3D::RenderScene(Scene3D* scene) {
  assert(scene);

  rendererAPI_->Clear(glm::vec4{0, 0, 0, 0});

  if (scene->GetMainCamera() == nullptr) {
    LOG_WARN(Renderer, "No main camera specified for the scene!");
    return;
  }

  for (const auto& model : scene->GetModels()) {
    auto shader = model->mesh->GetMaterial()->GetShader();
    shader->Bind();

    SetUpCamera(scene, shader);
    SetUpLights(scene, shader);

    /* Setting up transformation matrices */
    shader->LoadUniformMat4(scene->GetMainCamera()->CalculateProjectionViewMatrix(), kUniformNameProjectionView);
    shader->LoadUniformMat4(model->transform.CalculateMatrix(), kUniformNameModel);

    /* Setting up material info */
    model->mesh->GetMaterial()->LoadUniformsToShader();

    /* Drawing */
    rendererAPI_->Draw(*model->mesh->GetVertexArray());
  }
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