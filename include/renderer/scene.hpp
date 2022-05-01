/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file scene.hpp
 * @date 2022-04-28
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

// #include "scene_node.hpp"

// class Scene {
//   public:
//     Scene(ecs::Entity* root_entity) : root{new SceneNode{root_entity}} {}
//   public:
//     SceneNode* root = nullptr;
//     ecs::Entity* camera = nullptr;
//     Vector<ecs::Entity*> light_sources;
// };

#pragma once

#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "renderer/camera.hpp"
#include "renderer/light.hpp"
#include "renderer/mesh.hpp"

struct MeshNode {
  SharedPtr<Mesh> mesh;

  glm::vec3 pos{0.0f};
  glm::vec3 rotation{0.0f};
  glm::vec3 scale{1.0f};

  MeshNode(const SharedPtr<Mesh>& mesh, const glm::vec3& pos) : mesh(mesh), pos(pos) {}

  glm::mat4 CalculateModelTransform() const {
    glm::mat4 translationT = glm::translate(glm::identity<glm::mat4>(), pos);
    glm::mat4 rotationT    = glm::toMat4(glm::quat(rotation));
    glm::mat4 scaleT       = glm::scale(glm::identity<glm::mat4>(), scale);
    return translationT * rotationT * scaleT;
  }
};

struct CameraNode {
  PerspectiveCameraSpecs camera_specs;

  glm::vec3 pos;
  glm::vec3 forward{1, 0, 0};

  CameraNode(const PerspectiveCameraSpecs& camera_specs, const glm::vec3& pos = glm::vec3{0.0f},
             const glm::vec3& forward = glm::vec3{1, 0, 0})
      : camera_specs(camera_specs), pos(pos), forward(forward) {}

  glm::mat4 CalculateProjectionTransform() const { return camera_specs.CalculateProjectionTransform(); }

  glm::mat4 CalculateViewTransform() const { return glm::lookAt(pos, pos + forward, glm::vec3{0, 1, 0}); }

  glm::mat4 CalculateTransform() const { return CalculateProjectionTransform() * CalculateViewTransform(); }
};

struct PointLightNode {
  PointLightSpecs light_specs;
  glm::vec3 pos;

  PointLightNode(const PointLightSpecs& light_specs, const glm::vec3& pos) : light_specs(light_specs), pos(pos) {}
};

struct Scene {
  std::list<MeshNode> meshes;
  SharedPtr<CameraNode> camera;
  SharedPtr<PointLightNode> light;
};