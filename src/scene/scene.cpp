/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file scene.cpp
 * @date 2022-05-16
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

#include "scene/scene.hpp"

#include "core/logger.hpp"
#include "renderer/3d/renderer3d.hpp"

using namespace vulture;

EntityRegistry& Scene::GetEntityRegistry() { return entities_; }

void Scene::OnUpdate(float timestep) {
  /* Script update */
  auto scripts = GetView<ScriptComponent>(entities_);
  for (auto [entity, script] : scripts) {
    script->script->OnUpdate(entity, timestep);
  }
}

void Scene::Render() {
  /* Scene3D sync */
  auto lights = GetView<LightSourceComponent>(entities_);
  for (auto [entity, light] : lights) {
    // TODO: (tralf-strues) Add component create listeners to not check the existing of runtime nodes
    if (!light->runtime_node) {
      light->runtime_node = new LightSourceNode3D(light->specs);
      scene_.AddLightSource(light->runtime_node);
    }

    assert(entity.HasComponent<TransformComponent>());
    light->runtime_node->transform = entity.GetComponent<TransformComponent>()->transform;
  }

  auto models = GetView<MeshComponent>(entities_);
  for (auto [entity, model] : models) {
    // TODO: (tralf-strues) Add component create listeners to not check the existing of runtime nodes
    if (!model->runtime_node) {
      model->runtime_node = new ModelNode3D(model->mesh);
      scene_.AddModel(model->runtime_node);
    }

    assert(entity.HasComponent<TransformComponent>());
    model->runtime_node->transform = entity.GetComponent<TransformComponent>()->transform;
  }

  auto cameras = GetView<CameraComponent>(entities_);
  bool main_camera_found = false;
  for (auto [entity, camera] : cameras) {
    // TODO: (tralf-strues) Add component create listeners to not check the existing of runtime nodes
    if (!camera->runtime_node) {
      camera->runtime_node = new CameraNode3D(camera->specs);
      scene_.AddCamera(camera->runtime_node);
    }

    assert(entity.HasComponent<TransformComponent>());
    camera->runtime_node->transform = entity.GetComponent<TransformComponent>()->transform;

    if (camera->is_main) {
      if (!main_camera_found) {
        scene_.SetMainCamera(camera->runtime_node);
        main_camera_found = true;
      } else {
        LOG_WARN(Scene, "Second main camera entity detected! Using the first main camera detected.");
      }
    }
  }

  /* Rendering */
  Renderer3D::RenderScene(&scene_);
}

EntityHandle Scene::CreateEntity() {
  EntityHandle entity = vulture::CreateEntity(entities_);
  entity.AddComponent<HierarchyComponent>();
  return entity;
}

EntityHandle Scene::CreateChildEntity(EntityHandle parent) {
  EntityHandle entity = vulture::CreateEntity(entities_);
  entity.AddComponent<HierarchyComponent>(parent);
  return entity;
}

glm::mat4 Scene::ComputeWorldSpaceTransformMatrix(EntityHandle entity) {
  if (!entity.HasComponent<TransformComponent>()) {
    return glm::mat4(1);
  }

  auto parent = entity.GetComponent<HierarchyComponent>()->parent;

  return parent ? ComputeWorldSpaceTransformMatrix(parent.value())
                : entity.GetComponent<TransformComponent>()->transform.CalculateMatrix();
}
