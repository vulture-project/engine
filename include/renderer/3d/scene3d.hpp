/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file scene3d.hpp
 * @date 2022-05-01
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

#pragma once

#include <glm/glm.hpp>

#include "renderer/3d/camera.hpp"
#include "renderer/3d/light.hpp"
#include "renderer/3d/mesh.hpp"
#include "renderer/3d/transform.hpp"

namespace vulture {

//================================================================
// Scene Nodes
//================================================================
struct SceneNode3D {
  Transform transform;

  SceneNode3D() = default;
  SceneNode3D(const Transform& transform) : transform(transform) {}
};

constexpr glm::vec3 kDefaultCameraForwardVector{0, 0, -1};
constexpr glm::vec3 kDefaultCameraUpVector{0, 1, 0};
constexpr glm::vec3 kDefaultCameraRightVector{1, 0, 0};

struct CameraNode3D : public SceneNode3D {
  PerspectiveCameraSpecs specs;

  CameraNode3D(const PerspectiveCameraSpecs& specs, const Transform& transform = Transform())
      : SceneNode3D(transform), specs(specs) {}

  glm::mat4 CalculateProjectionMatrix() const { return specs.CalculateProjectionMatrix(); }
  glm::mat4 CalculateViewMatrix() const { return transform.CalculateInverseMatrix(); }

  glm::vec3 CalculateForwardVector() const {
    return transform.CalculateRotationMatrix() * glm::vec4(kDefaultCameraForwardVector, 1);
  }

  glm::vec3 CalculateUpVector() const {
    return transform.CalculateRotationMatrix() * glm::vec4(kDefaultCameraUpVector, 1);
  }

  glm::vec3 CalculateRightVector() const {
    return transform.CalculateRotationMatrix() * glm::vec4(kDefaultCameraRightVector, 1);
  }
};

struct LightSourceNode3D : public SceneNode3D {
  LightSourceSpecs specs;

  LightSourceNode3D(const LightSourceSpecs& specs, const Transform& transform = Transform())
      : SceneNode3D(transform), specs(specs) {}
};

struct ModelNode3D : public SceneNode3D {
  SharedPtr<Mesh> mesh;

  ModelNode3D(const SharedPtr<Mesh>& mesh, const Transform& transform = Transform())
      : SceneNode3D(transform), mesh(mesh) {}
};

//================================================================
// Scene3D
//================================================================
class Scene3D {
 public:
  Scene3D() = default;
  ~Scene3D();

  void SetMainCamera(CameraNode3D* camera);
  CameraNode3D* GetMainCamera();

  void AddCamera(CameraNode3D* camera);
  void RemoveCamera(CameraNode3D* camera);
  const std::list<CameraNode3D*>& GetCameras() const;

  void AddLightSource(LightSourceNode3D* light_source);
  void RemoveLightSource(LightSourceNode3D* light_source);
  const std::list<LightSourceNode3D*>& GetLightSources() const;

  void AddModel(ModelNode3D* model);
  void RemoveModel(ModelNode3D* model);
  const std::list<ModelNode3D*>& GetModels() const;

 private:
  CameraNode3D* main_camera_{nullptr};

  std::list<CameraNode3D*> cameras_;
  std::list<LightSourceNode3D*> light_sources_;
  std::list<ModelNode3D*> models_;
};

}  // namespace vulture