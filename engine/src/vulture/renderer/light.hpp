/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file light.hpp
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

namespace vulture {

struct DirectionalLightSpecification {
  alignas(16) glm::vec3 color     {0};
  alignas(4)  float      intensity {1};

  DirectionalLightSpecification(const glm::vec3& color = glm::vec3{0}, float intensity = 1)
      : color(color), intensity(intensity) {}
};

struct PointLightSpecification {
  alignas(16) glm::vec3 color     {0};
  alignas(4)  float      intensity {1};
  alignas(4)  float      range     {1};

  PointLightSpecification(const glm::vec3& color = glm::vec3{0}, float intensity = 1, float range = 1)
      : color(color), intensity(intensity), range(range) {}
};

struct SpotLightSpecification {
  alignas(16) glm::vec3 color      {0};
  alignas(4)  float      intensity  {1};
  alignas(4)  float      range      {1};
  alignas(4)  float      inner_cone {0};
  alignas(4)  float      outer_cone {0};
};

struct DirectionalLight {
  DirectionalLightSpecification specification {};
  alignas(16) glm::vec3        direction    {1, 0, 0};

  DirectionalLight(const DirectionalLightSpecification& specification, const glm::vec3& direction)
      : specification(specification), direction(direction) {}
};

struct PointLight {
  PointLightSpecification specification {};
  alignas(16) glm::vec3  position     {0};

  PointLight(const PointLightSpecification& specification, const glm::vec3& position)
      : specification(specification), position(position) {}
};

struct SpotLight {
  SpotLightSpecification specification {};
  alignas(16) glm::vec3 position     {0};
  alignas(16) glm::vec3 direction    {1, 0, 0};

  SpotLight(const SpotLightSpecification& specification, const glm::vec3& position, const glm::vec3& direction)
      : specification(specification), position(position), direction(direction) {}
};

struct LightEnvironment {
  Vector<DirectionalLight> directional_lights;
  Vector<PointLight>       point_lights;
  Vector<SpotLight>        spot_lights;
};

}  // namespace vulture