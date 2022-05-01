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

#include "renderer/renderer3d.hpp"

#include "components/components.hpp"

ScopePtr<RendererAPI> Renderer3D::rendererAPI_;

void Renderer3D::Init() {
  rendererAPI_ = RendererAPI::Create();
  rendererAPI_->Init();
}

void Renderer3D::SetViewport(const Viewport& viewport) { rendererAPI_->SetViewport(viewport); }

void Renderer3D::RenderScene(const Scene& scene, const SharedPtr<Shader>& shader) {
  shader->Bind();
  shader->LoadUniformFloat3("u_wsLightPos", scene.light->pos);
  shader->LoadUniformMat4("u_ProjectionView", scene.camera->CalculateTransform());

  glm::mat4 projectionView = scene.camera->CalculateTransform();

  rendererAPI_->Clear(glm::vec4{0, 0, 0, 0});
  for (const auto& mesh : scene.meshes) {
    shader->LoadUniformMat4("u_Model", mesh.CalculateModelTransform());

    glm::mat4 model = mesh.CalculateModelTransform();

    rendererAPI_->Draw(*mesh.mesh->vertex_array);
  }
}