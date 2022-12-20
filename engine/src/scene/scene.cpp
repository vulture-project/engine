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

using namespace vulture;

EntityRegistry& Scene::GetEntityRegistry() { return entities_; }

void Scene::OnStart(Dispatcher& dispatcher) {
  /* Script attach */
  auto scripts = GetView<ScriptComponent>(entities_);
  for (auto [entity, script] : scripts) {
    script->script->OnAttach(entity, dispatcher);
  }
}

void Scene::OnViewportResize(uint32_t width, uint32_t height) {
  assert(width > 0);
  assert(height > 0);

  float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);

  for (auto [entity, camera] : GetView<CameraComponent>(entities_)) {
    if (!camera->fixed_aspect_ratio) {
      camera->specs.aspect_ratio = aspect_ratio;
    }
  }
}

void Scene::OnUpdate(float timestep) {
  /* Script update */
  auto scripts = GetView<ScriptComponent>(entities_);
  for (auto [entity, script] : scripts) {
    script->script->OnUpdate(timestep);
  }
}

void Scene::Render(Renderer3D* renderer, Framebuffer* framebuffer, Renderer3D::DebugRenderMode render_mode) {
  /* Scene3D sync */
  auto lights = GetView<LightSourceComponent>(entities_);
  for (auto [entity, light] : lights) {
    // TODO: (tralf-strues) Add component create listeners to not check the existence of runtime nodes
    if (!light->runtime_node) {
      light->runtime_node = new LightSourceNode3D(light->specs);
      scene_.AddLightSource(light->runtime_node);
    }

    assert(entity.HasComponent<TransformComponent>());
    light->runtime_node->transform = ComputeWorldSpaceTransform(entity);
    light->runtime_node->SetLightSpecs(light->specs);
  }

  auto meshes = GetView<MeshComponent>(entities_);
  for (auto [entity, mesh] : meshes) {
    // TODO: (tralf-strues) Add component create listeners to not check the existence of runtime nodes
    if (!mesh->runtime_node) {
      mesh->runtime_node = new MeshNode3D(mesh->mesh);
      scene_.AddMesh(mesh->runtime_node);
    }

    assert(entity.HasComponent<TransformComponent>());
    mesh->runtime_node->outlined = mesh->outlined;
    mesh->runtime_node->transform = ComputeWorldSpaceTransform(entity);
  }

  auto cameras = GetView<CameraComponent>(entities_);
  bool main_camera_found = false;
  for (auto [entity, camera] : cameras) {
    // TODO: (tralf-strues) Add component create listeners to not check the existence of runtime nodes
    if (!camera->runtime_node) {
      camera->runtime_node = new CameraNode3D(camera->specs);
      scene_.AddCamera(camera->runtime_node);
    }

    assert(entity.HasComponent<TransformComponent>());
    camera->runtime_node->transform = ComputeWorldSpaceTransform(entity);
    camera->runtime_node->specs     = camera->specs;

    if (camera->is_main) {
      if (!main_camera_found) {
        scene_.SetMainCamera(camera->runtime_node);
        main_camera_found = true;
      } else {
        LOG_WARN(Scene, "Second main camera entity detected! Using the first main camera detected.");
      }
    }
  }

  if (!main_camera_found) {
    scene_.SetMainCamera(nullptr);
    LOG_WARN(Scene, "No main camera entity detected!");
  }

  /* Rendering */
  renderer->RenderScene(&scene_, framebuffer, render_mode);
}

EntityHandle Scene::CreateEntity(const std::string& name) {
  EntityHandle entity = vulture::CreateEntity(entities_);
  entity.AddComponent<HierarchyComponent>();
  entity.AddComponent<NameComponent>(name);
  return entity;
}

EntityHandle Scene::CreateChildEntity(EntityHandle parent, const std::string& name) {
  EntityHandle entity = vulture::CreateEntity(entities_);
  entity.AddComponent<HierarchyComponent>(parent);
  entity.AddComponent<NameComponent>(name);

  if (!parent.HasComponent<HierarchyComponent>()) {
    parent.AddComponent<HierarchyComponent>();
  }

  parent.GetComponent<HierarchyComponent>()->children.push_back(entity);

  return entity;
}

glm::mat4 Scene::ComputeWorldSpaceMatrix(EntityHandle entity) {
  return ComputeWorldSpaceTransform(entity).CalculateMatrix();
}

Transform Scene::ComputeWorldSpaceTransform(EntityHandle entity) {
  if (!entity.HasComponent<TransformComponent>()) {
    return {};
  }

  const Transform& local_transform = entity.GetComponent<TransformComponent>()->transform;

  auto parent = entity.GetComponent<HierarchyComponent>()->parent;
  if (parent.has_value()) {
    const Transform& parent_transform = ComputeWorldSpaceTransform(parent.value());
    glm::vec4 local_translation = glm::vec4(parent_transform.scale * local_transform.translation, 1.0f);

    return Transform(
        parent_transform.translation + glm::vec3(parent_transform.CalculateRotationMatrix() * local_translation),
        parent_transform.rotation * local_transform.rotation, parent_transform.scale * local_transform.scale);
  }

  return local_transform;
}
