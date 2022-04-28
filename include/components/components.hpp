#pragma once

#include <glm/glm.hpp>

#include "ECS/component.h"
#include "renderer/core.hpp"
#include "renderer/mesh.hpp"

class TransformComponent : ecs::Component<TransformComponent> {
  public:
    TransformComponent(const glm::vec3& translation) : translation{translation} {}
    TransformComponent(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale) : 
        translation{translation}, rotation{rotation}, scale{scale} {}

    glm::vec3 translation{0};
    glm::vec3 rotation{0};
    glm::vec3 scale{1};
};

class MeshComponent : ecs::Component<MeshComponent> {
  public:
    MeshComponent(const SharedPtr<Mesh>& mesh) : mesh{mesh} {}

    SharedPtr<Mesh> mesh;
};