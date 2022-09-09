#pragma once

#include <glm/glm.hpp>

#include "ECS/component.h"
#include "renderer/camera.hpp"
#include "renderer/core.hpp"
#include "renderer/mesh.hpp"

// class TransformComponent : public ecs::Component<TransformComponent> {
//  public:
//   glm::vec3 translation{0};
//   glm::vec3 rotation{0};
//   glm::vec3 scale{1};

//   TransformComponent(const glm::vec3& translation) : translation{translation} {}
//   TransformComponent(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale)
//       : translation{translation}, rotation{rotation}, scale{scale} {}
// };

// class MeshComponent : public ecs::Component<MeshComponent> {
//  public:
//   SharedPtr<Mesh> mesh;

//   MeshComponent(const SharedPtr<Mesh>& mesh) : mesh{mesh} {}
// };

// class CameraComponent : public ecs::Component<CameraComponent> {
//  public:
//   PerspectiveCamera camera;
//   glm::vec3 forward;

//   CameraComponent(const glm::vec3& forward, float aspect_ratio) : forward(forward), camera(aspect_ratio) {}

//   glm::mat4 GetProjectionTransform() const {
//     return camera.GetProjectionTransform();
//   }
// };

// class PointLightComponent : public ecs::Component<PointLightComponent> {
//  public:
//   glm::vec3 ambient;
//   glm::vec3 diffuse;
//   glm::vec3 specular;

//   PointLightComponent(const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular)
//       : ambient(ambient), diffuse(diffuse), specular(specular) {}
// };