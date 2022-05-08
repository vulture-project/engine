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
  rendererAPI_->Clear(glm::vec4{0, 0, 0, 0});

  for (const auto& model : scene->GetModels()) {
    auto shader = model->mesh->GetMaterial()->GetShader();
    shader->Bind();

    SetUpCamera(scene, shader);
    SetUpLights(scene, shader);

    /* Setting up transformation matrices */
    shader->LoadUniformMat4(kUniformNameProjectionView, scene->GetMainCamera()->CalculateProjectionMatrix() *
                                                            scene->GetMainCamera()->CalculateViewMatrix());

    shader->LoadUniformMat4(kUniformNameModel, model->transform.CalculateMatrix());

    /* Setting up material info */
    model->mesh->GetMaterial()->LoadUniformsToShader();

    /* Drawing */
    rendererAPI_->Draw(*model->mesh->GetVertexArray());
  }
}

void Renderer3D::SetUpCamera(Scene3D* scene, const SharedPtr<Shader>& shader) {
  assert(scene->GetMainCamera());
  shader->LoadUniformFloat3(kUniformNameWSCamera, scene->GetMainCamera()->transform.translation);
}

void Renderer3D::SetUpLights(Scene3D* scene, const SharedPtr<Shader>& shader) {
  static char uniform_name[kMaxUniformNameLength];  // NOTE: Not needed once start using uniform buffers

  uint32_t i = 0;
  for (const auto& light_source : scene->GetLightSources()) {
    std::snprintf(uniform_name, kMaxUniformNameLength, "%s[%u].%s", kUniformNamePointLights, i,
                  kStructMemberNameAmbientColor);
    shader->LoadUniformFloat3(uniform_name, light_source->specs.ambient);

    std::snprintf(uniform_name, kMaxUniformNameLength, "%s[%u].%s", kUniformNamePointLights, i,
                  kStructMemberNameDiffuseColor);
    shader->LoadUniformFloat3(uniform_name, light_source->specs.diffuse);

    std::snprintf(uniform_name, kMaxUniformNameLength, "%s[%u].%s", kUniformNamePointLights, i,
                  kStructMemberNameSpecularColor);
    shader->LoadUniformFloat3(uniform_name, light_source->specs.specular);

    std::snprintf(uniform_name, kMaxUniformNameLength, "%s[%u].%s", kUniformNamePointLights, i,
                  kStructMemberNameWSPosition);
    shader->LoadUniformFloat3(uniform_name, light_source->transform.translation);

    std::snprintf(uniform_name, kMaxUniformNameLength, "%s[%u].%s", kUniformNamePointLights, i,
                  kStructMemberNameAttenuationLinear);
    shader->LoadUniformFloat(uniform_name, light_source->specs.attenuation_linear);

    std::snprintf(uniform_name, kMaxUniformNameLength, "%s[%u].%s", kUniformNamePointLights, i,
                  kStructMemberNameAttenuationQuadratic);
    shader->LoadUniformFloat(uniform_name, light_source->specs.attenuation_quadratic);

    ++i;
  }

  shader->LoadUniformInt(kUniformNamePointLightsCount, scene->GetLightSources().size());
}