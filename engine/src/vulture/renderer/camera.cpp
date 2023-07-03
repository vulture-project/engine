/**
 * @author Nikita Mochalov (github.com/tralf-strues)s
 * @file camera.cpp
 * @date 2022-05-10
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

#include <vulture/renderer/camera.hpp>

using namespace vulture;

PerspectiveCameraSpecification::PerspectiveCameraSpecification(float aspect) : aspect(aspect) {}

glm::mat4 PerspectiveCameraSpecification::CalculateProjectionMatrix() const {
  return glm::perspective(glm::radians(fov), aspect, near_plane, far_plane);
}

OrthographicCameraSpecification::OrthographicCameraSpecification(float size, float aspect)
    : size(size), aspect(aspect) {}

glm::mat4 OrthographicCameraSpecification::CalculateProjectionMatrix() const {
  return glm::ortho(-size * aspect, size * aspect, -size, size, near_plane, far_plane);
}

Camera::Camera(const PerspectiveCameraSpecification& specs, SharedPtr<Texture> render_texture)
    : projection_type(CameraProjectionType::kPerspective),
      perspective_specification(specs),
      render_texture(render_texture) {}

Camera::Camera(const OrthographicCameraSpecification& specs, SharedPtr<Texture> render_texture)
    : projection_type(CameraProjectionType::kOrthographic),
      orthographic_specification(specs),
      render_texture(render_texture) {}

const glm::mat4& Camera::ViewMatrix() const { return view_; }
const glm::mat4& Camera::ProjMatrix() const { return proj_; }
const glm::mat4& Camera::TransformMatrix() const { return transform_matrix_; }
const glm::vec3& Camera::Position() const { return position_; }

float Camera::NearPlane() const {
  switch (projection_type) {
    case CameraProjectionType::kPerspective:  { return perspective_specification.near_plane; }
    case CameraProjectionType::kOrthographic: { return orthographic_specification.near_plane; }

    default: { VULTURE_ASSERT(false, "Invalid CameraProjectionType"); }
  }

  return 0.0f;
}

float Camera::FarPlane() const {
  switch (projection_type) {
    case CameraProjectionType::kPerspective:  { return perspective_specification.far_plane; }
    case CameraProjectionType::kOrthographic: { return orthographic_specification.far_plane; }

    default: { VULTURE_ASSERT(false, "Invalid CameraProjectionType"); }
  }

  return 0.0f;
}

void Camera::CalculateFrustumCorners(glm::vec3* out_corners) const {
  VULTURE_ASSERT(out_corners, "Nullptr output array");

  // FIXME: (tralf-strues) find a better way to calculate the inverse matrix (or corners altogether)
  const auto inverse = glm::inverse(ProjMatrix() * ViewMatrix());

  const glm::vec3 kFrustumCornersNDC[8] = {
      glm::vec3(-1.0f, 1.0f, 0.0f),  glm::vec3(1.0f, 1.0f, 0.0f),   glm::vec3(1.0f, -1.0f, 0.0f),
      glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(-1.0f, 1.0f, 1.0f),  glm::vec3(1.0f, 1.0f, 1.0f),
      glm::vec3(1.0f, -1.0f, 1.0f),  glm::vec3(-1.0f, -1.0f, 1.0f),
  };

  for (uint32_t i = 0; i < 8; i++) {
    glm::vec4 corner = inverse * glm::vec4(kFrustumCornersNDC[i], 1.0f);
    out_corners[i] = corner / corner.w;
  }
}

void Camera::OnUpdateAspect(float aspect) {
  perspective_specification.aspect  = aspect;
  orthographic_specification.aspect = aspect;
}

void Camera::OnUpdateTransform(const Transform& transform) {
  view_             = transform.CalculateInverseMatrix();
  transform_matrix_ = transform.CalculateMatrix();
  position_         = transform.position;
}

void Camera::OnUpdateProjection() {
  switch (projection_type) {
    case CameraProjectionType::kPerspective:  { proj_ = perspective_specification.CalculateProjectionMatrix(); break; }
    case CameraProjectionType::kOrthographic: { proj_ = orthographic_specification.CalculateProjectionMatrix(); break; }

    default: { VULTURE_ASSERT(false, "Invalid CameraProjectionType"); }
  };
}