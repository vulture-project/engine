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

#include <vulture/core/logger.hpp>
#include <vulture/scene/scene.hpp>

using namespace vulture;

fennecs::EntityWorld& Scene::GetEntityWorld() { return world_; }

void Scene::OnStart(Dispatcher& dispatcher) {
  fennecs::EntityStream stream = world_.Query<ScriptComponent>();  
  for (fennecs::EntityHandle entity = stream.Next(); !entity.IsNull(); entity = stream.Next()) {
    ScriptComponent& script = entity.Get<ScriptComponent>();
    script.script->OnAttach(entity, dispatcher);
  }
}

void Scene::OnViewportResize(uint32_t width, uint32_t height) {
  assert(width > 0);
  assert(height > 0);

  float aspect = static_cast<float>(width) / static_cast<float>(height);

  fennecs::EntityStream stream = world_.Query<CameraComponent>();  
  for (fennecs::EntityHandle entity = stream.Next(); !entity.IsNull(); entity = stream.Next()) {
    CameraComponent& camera = entity.Get<CameraComponent>();
    if (!camera.fixed_aspect) {
      camera.camera.OnUpdateAspect(aspect);
    }
  }
}

fennecs::EntityHandle Scene::GetMainCamera() {
  fennecs::EntityStream camera_stream = world_.Query<CameraComponent>();
  for (fennecs::EntityHandle entity = camera_stream.Next(); !entity.IsNull(); entity = camera_stream.Next()) {
    CameraComponent& camera = entity.Get<CameraComponent>();
    assert(entity.Has<TransformComponent>());

    if (camera.is_main) {
      return entity;
    }
  }

  return fennecs::EntityHandle::Null();
}

void Scene::OnUpdate(float timestep) {
  fennecs::EntityStream camera_stream = world_.Query<CameraComponent>();
  for (fennecs::EntityHandle entity = camera_stream.Next(); !entity.IsNull(); entity = camera_stream.Next()) {
    CameraComponent& camera_component = entity.Get<CameraComponent>();
    camera_component.camera.OnUpdateProjection();
    camera_component.camera.OnUpdateTransform(ComputeWorldSpaceTransform(entity));
  }

  fennecs::EntityStream stream = world_.Query<ScriptComponent>();  
  for (fennecs::EntityHandle entity = stream.Next(); !entity.IsNull(); entity = stream.Next()) {
    ScriptComponent& script = entity.Get<ScriptComponent>();
    script.script->OnUpdate(timestep);
  }
}

void Scene::Render(Renderer& renderer, CommandBuffer& command_buffer,
                   uint32_t current_frame, float time) {
  fennecs::EntityHandle main_camera_handle = GetMainCamera();
  assert(!main_camera_handle.IsNull());

  const Camera& main_camera = main_camera_handle.Get<CameraComponent>().camera;

  /* Lights */
  LightEnvironment& lights = renderer.GetLightEnvironment();

  lights.directional_lights.clear();
  fennecs::EntityStream directional_light_stream = world_.Query<DirectionalLightSpecification>();
  for (auto entity = directional_light_stream.Next(); !entity.IsNull(); entity = directional_light_stream.Next()) {
    const auto& specs     = entity.Get<DirectionalLightSpecification>();
    const auto& transform = entity.Get<TransformComponent>().transform;

    lights.directional_lights.emplace_back(specs, transform.CalculateForward());
  }

  lights.point_lights.clear();
  fennecs::EntityStream point_light_stream = world_.Query<PointLightSpecification>();
  for (auto entity = point_light_stream.Next(); !entity.IsNull(); entity = point_light_stream.Next()) {
    const auto& specs     = entity.Get<PointLightSpecification>();
    const auto& transform = entity.Get<TransformComponent>().transform;

    lights.point_lights.emplace_back(specs, transform.position);
  }

  lights.spot_lights.clear();
  fennecs::EntityStream spot_light_stream = world_.Query<SpotLightSpecification>();
  for (auto entity = spot_light_stream.Next(); !entity.IsNull(); entity = spot_light_stream.Next()) {
    const auto& specs     = entity.Get<SpotLightSpecification>();
    const auto& transform = entity.Get<TransformComponent>().transform;

    lights.spot_lights.emplace_back(specs, transform.position, transform.CalculateForward());
  }

  /* Render Queue */
  RenderQueue render_queue{};

  fennecs::EntityStream mesh_stream = world_.Query<MeshComponent, TransformComponent>();
  for (auto entity = mesh_stream.Next(); !entity.IsNull(); entity = mesh_stream.Next()) {
    MeshComponent& mesh_component = entity.Get<MeshComponent>();
    Transform&     transform      = entity.Get<TransformComponent>().transform;

    render_queue.renderables.emplace_back(
        RenderQueue::Renderable{mesh_component.mesh, ComputeWorldSpaceMatrix(entity)});
  }

  renderer.Render(command_buffer, main_camera, std::move(render_queue), time, current_frame);
}

fennecs::EntityHandle Scene::CreateEntity(const std::string& name) {
  fennecs::EntityHandle entity = world_.AddEntity();
  entity = world_.Attach<HierarchyComponent>(entity);
  entity = world_.Attach<NameComponent>(entity, name);
  return entity;
}

fennecs::EntityHandle Scene::CreateChildEntity(fennecs::EntityHandle& parent, const std::string& name) {
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
    glm::vec4 local_position = glm::vec4(parent_transform.scale * local_transform.position, 1.0f);

    return Transform(
        parent_transform.position + glm::vec3(parent_transform.CalculateRotationMatrix() * local_position),
        parent_transform.rotation * local_transform.rotation, parent_transform.scale * local_transform.scale);
  }

  return local_transform;
}
