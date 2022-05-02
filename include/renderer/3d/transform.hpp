/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file transform.hpp
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

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace vulture {

struct Transform {
  glm::vec3 translation;
  glm::vec3 rotation;
  glm::vec3 scale;

  Transform(const glm::vec3& translation = glm::vec3{0.0f}, const glm::vec3& rotation = glm::vec3{0.0f},
            const glm::vec3& scale = glm::vec3{1.0f})
      : translation(translation), rotation(rotation), scale(scale) {}

  glm::mat4 CalculateMatrix() const {
    return CalculateTranslationMatrix() * CalculateRotationMatrix() * CalculateScaleMatrix();
  }

  /**
   * @brief Calculate inverse matrix to convert back from world space to model space.
   *
   * Calculates the inverse of a transformation matrix:
   * M = T * R * S
   * M^-1 = S^-1 * R^-1 * T^-1 = S^-1 * R^T * T^-1
   *
   * Inverse of a scale matrix is just a scale matrix from vec3{1/sx, 1/sy, 1/sz}.
   *
   * Inverse of a rotation matrix is simply its transpose, because a rotation
   * matrix is orthogonal.
   *
   * Inverse of a translation matrix is simply a translation matrix composed
   * of negative values of the given translation.
   *
   * @return glm::mat4
   */
  glm::mat4 CalculateInverseMatrix() const {
    return glm::scale(glm::identity<glm::mat4>(), glm::vec3{1 / scale.x, 1 / scale.y, 1 / scale.z}) *
           glm::transpose(CalculateRotationMatrix()) *
           glm::translate(glm::identity<glm::mat4>(), -translation);
  }

  glm::mat4 CalculateTranslationMatrix() const { return glm::translate(glm::identity<glm::mat4>(), translation); }
  glm::mat4 CalculateRotationMatrix() const { return glm::toMat4(glm::quat(rotation)); }
  glm::mat4 CalculateScaleMatrix() const { return glm::scale(glm::identity<glm::mat4>(), scale); }
};

}  //  namespace vulture