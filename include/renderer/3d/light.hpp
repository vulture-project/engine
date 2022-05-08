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

// struct LightSourceSpecs {
//   enum class Type {
//     kAmbient,
//     kPoint,

//     kTotal
//   };

//   Type type;

//   glm::vec3 ambient;
//   glm::vec3 diffuse;
//   glm::vec3 specular;

//   float attenuation_linear;
//   float attenuation_quadratic;

//   LightSourceSpecs(Type type, const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular,
//                    float attenuation_linear, float attenuation_quadratic)
//       : type(type),
//         ambient(ambient),
//         diffuse(diffuse),
//         specular(specular),
//         attenuation_linear(attenuation_linear),
//         attenuation_quadratic(attenuation_quadratic) {}
// };

enum class LightType {
  kInvalid = -1,
  kDirectional,
  kPoint,
  kSpot,

  kTotal
};

struct LightColorSpecs {
  glm::vec3 ambient{0};
  glm::vec3 diffuse{0};
  glm::vec3 specular{0};

  LightColorSpecs() = default;
  LightColorSpecs(const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular)
      : ambient(ambient), diffuse(diffuse), specular(specular) {}
};

struct LightAttenuationSpecs {
  float linear{0};
  float quadratic{0};

  LightAttenuationSpecs() = default;
  LightAttenuationSpecs(float linear, float quadratic) : linear(linear), quadratic(quadratic) {}
  LightAttenuationSpecs(float range) : linear(2.0f / range), quadratic(1.0f / (range * range)) {}
};

struct DirectionalLightSpecs {
  LightColorSpecs color_specs;

  DirectionalLightSpecs() = default;
  DirectionalLightSpecs(const LightColorSpecs& color_specs) : color_specs(color_specs) {}
};

struct PointLightSpecs {
  LightColorSpecs color_specs;
  LightAttenuationSpecs attenuation_specs;

  PointLightSpecs() = default;
  PointLightSpecs(const LightColorSpecs& color_specs, const LightAttenuationSpecs& attenuation_specs)
      : color_specs(color_specs), attenuation_specs(attenuation_specs) {}
};

struct SpotLightSpecs {
  LightColorSpecs color_specs;
  LightAttenuationSpecs attenuation_specs;
  float inner_cone_cosine{1};
  float outer_cone_cosine{1};

  SpotLightSpecs() = default;
  SpotLightSpecs(const LightColorSpecs& color_specs, const LightAttenuationSpecs& attenuation_specs,
                 float inner_cone_cosine, float outer_cone_cosine)
      : color_specs(color_specs),
        attenuation_specs(attenuation_specs),
        inner_cone_cosine(inner_cone_cosine),
        outer_cone_cosine(outer_cone_cosine) {}
};

}  // namespace vulture