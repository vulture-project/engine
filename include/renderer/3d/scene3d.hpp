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

constexpr glm::vec3 kDefaultForwardVector{0, 0, -1};
constexpr glm::vec3 kDefaultUpVector{0, 1, 0};
constexpr glm::vec3 kDefaultRightVector{1, 0, 0};

/**
 * @brief Specifies a 3D camera.
 * @warning There must be one main camera selected in the scene @see Scene3D::SetMainCamera() to render!
 */
struct CameraNode3D : public SceneNode3D {
  PerspectiveCameraSpecs specs;

  CameraNode3D(const PerspectiveCameraSpecs& specs, const Transform& transform = Transform())
      : SceneNode3D(transform), specs(specs) {}

  glm::mat4 CalculateProjectionViewMatrix() const { return CalculateProjectionMatrix() * CalculateViewMatrix(); }
  glm::mat4 CalculateProjectionMatrix() const { return specs.CalculateProjectionMatrix(); }
  glm::mat4 CalculateViewMatrix() const { return transform.CalculateInverseMatrix(); }

  glm::vec3 CalculateForwardVector() const {
    return transform.CalculateRotationMatrix() * glm::vec4(kDefaultForwardVector, 1);
  }

  glm::vec3 CalculateUpVector() const { return transform.CalculateRotationMatrix() * glm::vec4(kDefaultUpVector, 1); }

  glm::vec3 CalculateRightVector() const {
    return transform.CalculateRotationMatrix() * glm::vec4(kDefaultRightVector, 1);
  }
};

constexpr uint32_t kMaxPointLightSources = 10;
constexpr uint32_t kMaxSpotLightSources = 2;

/**
 * @brief Represents on of the three types of light sources: Directional, Point and Spot lights.
 *
 * @attention There are currently limitations on the numbers of different type light sources you can
 * use for rendering (Renderer3D will just use the maximum number of light sources possible, you can
 * still have as much LightSourceNode3D instances in a scene as you want)!
 * 1) There can be only ONE Directional Light
 * 2) There can be @see kMaxPointLightSources Point Lights
 * 3) There can be @see kMaxSpotLightSources Spot Lights
 */
class LightSourceNode3D : public SceneNode3D {
 public:
  LightSourceNode3D(const LightSourceSpecs& specs, const Transform& transform = Transform())
      : SceneNode3D(transform), specs_(specs) {}

  LightSourceNode3D(const DirectionalLightSpecs& specs, const Transform& transform = Transform())
      : SceneNode3D(transform), specs_(specs) {}

  LightSourceNode3D(const PointLightSpecs& specs, const Transform& transform = Transform())
      : SceneNode3D(transform), specs_(specs) {}

  LightSourceNode3D(const SpotLightSpecs& specs, const Transform& transform = Transform())
      : SceneNode3D(transform), specs_(specs) {}

  LightType GetType() const { return specs_.type; }

  const LightSourceSpecs& GetLightSpecs() const { return specs_; }

  void SetLightSpecs(const DirectionalLightSpecs& specs) { specs_.directional = specs; }

  void SetLightSpecs(const PointLightSpecs& specs) { specs_.point = specs; }

  void SetLightSpecs(const SpotLightSpecs& specs) { specs_.spot = specs; }

  void SetEnabled(bool enabled) { enabled_ = enabled; }
  bool IsEnabled() const { return enabled_; }

 private:
  LightSourceSpecs specs_;
  bool enabled_{true};
};

struct MeshNode3D : public SceneNode3D {
  SharedPtr<Mesh> mesh;

  MeshNode3D(SharedPtr<Mesh> mesh, const Transform& transform = Transform()) : SceneNode3D(transform), mesh(mesh) {}
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

  void AddMesh(MeshNode3D* mesh);
  void RemoveMesh(MeshNode3D* mesh);
  const std::list<MeshNode3D*>& GetMeshes() const;

 private:
  CameraNode3D* main_camera_{nullptr};

  std::list<CameraNode3D*> cameras_;
  std::list<LightSourceNode3D*> light_sources_;
  std::list<MeshNode3D*> meshes_;
};

}  // namespace vulture