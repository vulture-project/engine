/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file material.hpp
 * @date 2022-05-03
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
#include <map>
#include <string>

#include "renderer/shader.hpp"
#include "renderer/texture.hpp"

namespace vulture {

class Material {
 public:
  Material() = default;
  Material(SharedPtr<Shader> shader);

  void LoadUniformsToShader();

  void SetShader(SharedPtr<Shader> shader);
  Shader* GetShader();

  void AddTexture(const std::string& name, SharedPtr<Texture> texture);

  template <typename T, typename... Args>
  inline void SetUniform(const T& value, const std::string& format, Args&&... args) {
    SetUniform(value, fmt::format(format, std::forward<Args>(args)...));
  }

  template <typename T>
  inline void SetUniform(const T& value, const std::string& name) {
    if      constexpr (std::is_same<T, int>())       { uniforms_int_   [name] = value; }
    else if constexpr (std::is_same<T, float>())     { uniforms_float_ [name] = value; }
    else if constexpr (std::is_same<T, glm::vec2>()) { uniforms_float2_[name] = value; }
    else if constexpr (std::is_same<T, glm::vec3>()) { uniforms_float3_[name] = value; }
    else if constexpr (std::is_same<T, glm::vec4>()) { uniforms_float4_[name] = value; }
    else if constexpr (std::is_same<T, glm::mat4>()) { uniforms_mat4_  [name] = value; }
    else {
      static_assert(std::is_same<T, std::false_type>(), "Unsupported uniform data type in Material::SetUniform()");
    }
  }

 private:
  SharedPtr<Shader> shader_{nullptr};
  std::map<std::string, SharedPtr<Texture>> textures_;

  /* TODO: Think of an alternative way of storing this */
  std::map<std::string, int>       uniforms_int_;
  std::map<std::string, float>     uniforms_float_;
  std::map<std::string, glm::vec2> uniforms_float2_;
  std::map<std::string, glm::vec3> uniforms_float3_;
  std::map<std::string, glm::vec4> uniforms_float4_;
  std::map<std::string, glm::mat4> uniforms_mat4_;
};

}  // namespace vulture