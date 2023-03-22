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
  glm::vec3 color     {0};
  float     intensity {1};

  DirectionalLightSpecification(const glm::vec3& color = glm::vec3{0}, float intensity = 1)
      : color(color), intensity(intensity) {}
};

struct PointLightSpecification {
  glm::vec3 color     {0};
  float     intensity {1};
  float     range     {1};
};

struct SpotLightSpecification {
  glm::vec3 color      {0};
  float     intensity  {1};
  float     range      {1};
  float     inner_cone {0};
  float     outer_cone {0};
};

struct DirectionalLight {
  DirectionalLightSpecification specification {};
  glm::vec3                     direction     {1, 0, 0};

  char                          _padding[1]   {0};
};

struct PointLight {
  PointLightSpecification specification {};
  glm::vec3               position      {0};
};

struct SpotLight {
  SpotLightSpecification specification {};
  glm::vec3              position      {0};
  glm::vec3              direction     {1, 0, 0};

  char                   _padding[3]   {0};
};

struct LightEnvironment {
  uint32_t                 directional_lights_count;
  Vector<DirectionalLight> directional_lights;

  uint32_t                 point_lights_count;
  Vector<PointLight>       point_lights;

  uint32_t                 spot_lights_count;
  Vector<SpotLight>        spot_lights;
};

}  // namespace vulture