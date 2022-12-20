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

fennecs::EntityWorld& Scene::GetEntityWorld() { return world_; }

void Scene::OnStart(Dispatcher& dispatcher) {
  // auto scripts = GetView<ScriptComponent>(entities_);
  // for (auto [entity, script] : scripts) {
  //   script->script->OnAttach(entity, dispatcher);
  // }

  fennecs::EntityStream stream = world_.Query<ScriptComponent>();  
  for (fennecs::EntityHandle entity = stream.Next(); !entity.IsNull(); entity = stream.Next()) {
    ScriptComponent& script = entity.Get<ScriptComponent>();
    script.script->OnAttach(entity, dispatcher);
  }
}

void Scene::OnViewportResize(uint32_t width, uint32_t height) {
  // assert(width > 0);
  // assert(height > 0);

  // float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);

  // for (auto [entity, camera] : GetView<CameraComponent>(entities_)) {
  //   if (!camera->fixed_aspect_ratio) {
  //     camera->specs.aspect_ratio = aspect_ratio;
  //   }
  // }

  assert(width > 0);
  assert(height > 0);

  float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);

  fennecs::EntityStream stream = world_.Query<CameraComponent>();  
  for (fennecs::EntityHandle entity = stream.Next(); !entity.IsNull(); entity = stream.Next()) {
    CameraComponent& camera = entity.Get<CameraComponent>();
    if (!camera.fixed_aspect_ratio) {
      camera.specs.aspect_ratio = aspect_ratio;
    }
  }
}

void Scene::OnUpdate(float timestep) {
  // auto scripts = GetView<ScriptComponent>(entities_);
  // for (auto [entity, script] : scripts) {
  //   script->script->OnUpdate(timestep);
  // }

  fennecs::EntityStream stream = world_.Query<ScriptComponent>();  
  for (fennecs::EntityHandle entity = stream.Next(); !entity.IsNull(); entity = stream.Next()) {
    ScriptComponent& script = entity.Get<ScriptComponent>();
    script.script->OnUpdate(timestep);
  }
}

void Scene::Render(Renderer3D::DebugRenderMode render_mode) {
  // auto lights = GetView<LightSourceComponent>(entities_);
  // for (auto [entity, light] : lights) {
  //   // TODO: (tralf-strues) Add component create listeners to not check the existence of runtime nodes
  //   if (!light->runtime_node) {
  //     light->runtime_node = new LightSourceNode3D(light->specs);
  //     scene_.AddLightSource(light->runtime_node);
  //   }

  //   assert(entity.HasComponent<TransformComponent>());
  //   light->runtime_node->transform = ComputeWorldSpaceTransform(entity);
  //   light->runtime_node->SetLightSpecs(light->specs);
  // }

  fennecs::EntityStream lights_stream = world_.Query<LightSourceComponent>();
  for (fennecs::EntityHandle entity = lights_stream.Next(); !entity.IsNull(); entity = lights_stream.Next()) {
    LightSourceComponent& light = entity.Get<LightSourceComponent>();
    if (!light.runtime_node) {
      light.runtime_node = new LightSourceNode3D(light.specs);
      scene_.AddLightSource(light.runtime_node);
    }

    assert(entity.Has<TransformComponent>());
    light.runtime_node->transform = ComputeWorldSpaceTransform(entity);
    light.runtime_node->SetLightSpecs(light.specs);
  }

  // auto meshes = GetView<MeshComponent>(entities_);
  // for (auto [entity, mesh] : meshes) {
  //   // TODO: (tralf-strues) Add component create listeners to not check the existence of runtime nodes
  //   if (!mesh->runtime_node) {
  //     mesh->runtime_node = new MeshNode3D(mesh->mesh);
  //     scene_.AddMesh(mesh->runtime_node);
  //   }

  //   assert(entity.HasComponent<TransformComponent>());
  //   mesh->runtime_node->transform = ComputeWorldSpaceTransform(entity);
  // }

  fennecs::EntityStream mesh_stream = world_.Query<MeshComponent>();
  for (fennecs::EntityHandle entity = mesh_stream.Next(); !entity.IsNull(); entity = mesh_stream.Next()) {
    MeshComponent& mesh = entity.Get<MeshComponent>();
    if (!mesh.runtime_node) {
      mesh.runtime_node = new MeshNode3D(mesh.mesh);
      scene_.AddMesh(mesh.runtime_node);
    }

    assert(entity.Has<TransformComponent>());
    mesh.runtime_node->transform = ComputeWorldSpaceTransform(entity);
  }

  // auto cameras = GetView<CameraComponent>(entities_);
  // bool main_camera_found = false;
  // for (auto [entity, camera] : cameras) {
  //   // TODO: (tralf-strues) Add component create listeners to not check the existence of runtime nodes
  //   if (!camera->runtime_node) {
  //     camera->runtime_node = new CameraNode3D(camera->specs);
  //     scene_.AddCamera(camera->runtime_node);
  //   }

  //   assert(entity.HasComponent<TransformComponent>());
  //   camera->runtime_node->transform = ComputeWorldSpaceTransform(entity);
  //   camera->runtime_node->specs     = camera->specs;

  //   if (camera->is_main) {
  //     if (!main_camera_found) {
  //       scene_.SetMainCamera(camera->runtime_node);
  //       main_camera_found = true;
  //     } else {
  //       LOG_WARN(Scene, "Second main camera entity detected! Using the first main camera detected.");
  //     }
  //   }
  // }

  bool main_camera_found = false;
  fennecs::EntityStream camera_stream = world_.Query<CameraComponent>();
  for (fennecs::EntityHandle entity = camera_stream.Next(); !entity.IsNull(); entity = camera_stream.Next()) {
    CameraComponent& camera = entity.Get<CameraComponent>();
    if (!camera.runtime_node) {
      camera.runtime_node = new CameraNode3D(camera.specs);
      scene_.AddCamera(camera.runtime_node);
    }

    assert(entity.Has<TransformComponent>());
    camera.runtime_node->transform = ComputeWorldSpaceTransform(entity);
    camera.runtime_node->specs     = camera.specs;

    if (camera.is_main) {
      if (!main_camera_found) {
        scene_.SetMainCamera(camera.runtime_node);
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
  Renderer3D::RenderScene(&scene_, render_mode);
}

fennecs::EntityHandle Scene::CreateEntity(const std::string& name) {
  // EntityHandle entity = vulture::CreateEntity(entities_);
  // entity.AddComponent<HierarchyComponent>();
  // entity.AddComponent<NameComponent>(name);
  // return entity;

  fennecs::EntityHandle entity = world_.AddEntity();
  entity = world_.Attach<HierarchyComponent>(entity);
  entity = world_.Attach<NameComponent>(entity, name);
  return entity;
}

fennecs::EntityHandle Scene::CreateChildEntity(fennecs::EntityHandle& parent, const std::string& name) {
  // EntityHandle entity = vulture::CreateEntity(entities_);
  // entity.AddComponent<HierarchyComponent>(parent);
  // entity.AddComponent<NameComponent>(name);

  // if (!parent.HasComponent<HierarchyComponent>()) {
  //   parent.AddComponent<HierarchyComponent>();
  // }

  // parent.GetComponent<HierarchyComponent>()->children.push_back(entity);

  // return entity;

  fennecs::EntityHandle entity = world_.AddEntity();
  entity = world_.Attach<HierarchyComponent>(entity, parent);
  entity = world_.Attach<NameComponent>(entity, name);

  if (!parent.Has<HierarchyComponent>()) {
    parent = world_.Attach<HierarchyComponent>(fennecs::EntityHandle{parent});
  }
  HierarchyComponent& hierarchy = parent.Get<HierarchyComponent>();
  hierarchy.children.push_back(entity);

  return entity;
}

glm::mat4 Scene::ComputeWorldSpaceMatrix(fennecs::EntityHandle entity) {
  return ComputeWorldSpaceTransform(entity).CalculateMatrix();
}

Transform Scene::ComputeWorldSpaceTransform(fennecs::EntityHandle entity) {
  if (!entity.Has<TransformComponent>()) {
    return Transform{};
  }

  const Transform& local_transform = entity.Get<TransformComponent>().transform;

  auto parent = entity.Get<HierarchyComponent>().parent;
  if (parent.has_value()) {
    const Transform& parent_transform = ComputeWorldSpaceTransform(parent.value());
    glm::vec4 local_translation = glm::vec4(parent_transform.scale * local_transform.translation, 1.0f);

    return Transform(
        parent_transform.translation + glm::vec3(parent_transform.CalculateRotationMatrix() * local_translation),
        parent_transform.rotation * local_transform.rotation, parent_transform.scale * local_transform.scale);
  }

  return local_transform;
}
