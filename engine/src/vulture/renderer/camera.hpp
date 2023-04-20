/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file camera.hpp
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

#pragma once

#include <glm/glm.hpp>
#include <vulture/core/core.hpp>
#include <vulture/renderer/texture.hpp>
#include <vulture/renderer/transform.hpp>

namespace vulture {

enum class CameraProjectionType : uint32_t {
  kPerspective,
  kOrthographic
};

struct PerspectiveCameraSpecification {
  float fov        {45.0f};
  float near_plane {0.5f};
  float far_plane  {100.0f};
  float aspect     {0.0f};

  PerspectiveCameraSpecification(float aspect = 0.0f);

  glm::mat4 CalculateProjectionMatrix() const;
};

struct OrthographicCameraSpecification {
  float size       {100.0f};
  float near_plane {0.1f};
  float far_plane  {100.0f};
  float aspect     {0.0f};

  OrthographicCameraSpecification(float size = 100.0f, float aspect = 0.0f);

  glm::mat4 CalculateProjectionMatrix() const;
};

struct Camera {
 public:
  CameraProjectionType           projection_type{CameraProjectionType::kPerspective};
  PerspectiveCameraSpecification  perspective_specification{};
  OrthographicCameraSpecification orthographic_specification{};

  SharedPtr<Texture>             render_texture{nullptr};

 public:
  Camera() = default;
  Camera(const PerspectiveCameraSpecification& specs, SharedPtr<Texture> render_texture = nullptr);
  Camera(const OrthographicCameraSpecification& specs, SharedPtr<Texture> render_texture = nullptr);

  const glm::mat4& ViewMatrix()      const;
  const glm::mat4& ProjMatrix()      const;
  const glm::mat4& TransformMatrix() const;
  const glm::vec3& Position()        const;
  float             NearPlane()       const;
  float             FarPlane()        const;

  void CalculateFrustumCorners(glm::vec3* out_corners) const;

  void OnUpdateAspect(float aspect);
  void OnUpdateTransform(const Transform& transform);
  void OnUpdateProjection();

 private:
  glm::mat4 view_{};
  glm::mat4 proj_{};
  glm::mat4 transform_matrix_{};
  glm::vec3 position_{};
};

}  // namespace vulture