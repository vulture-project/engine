/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file shader.hpp
 * @date 2022-04-27
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
#include <string>

#include "core/core.hpp"
#include "renderer/buffer.hpp"

namespace vulture {

class Shader {
 public:
  static SharedPtr<Shader> Create(const std::string& filename);
  static SharedPtr<Shader> Create(const std::string& vertex_shader, const std::string& fragment_shader);

  virtual ~Shader() = default;

  virtual void Bind() const = 0;
  virtual void Unbind() const = 0;

  virtual const AttributeLocationMap& GetAttributeLocations() const = 0;

  /**
   * @attention Uniform loading functions bind Shader!
   */
  virtual void LoadUniformInt(const std::string& name, int value) = 0;

  /**
   * @attention Uniform loading functions bind Shader!
   */
  virtual void LoadUniformFloat(const std::string& name, float value) = 0;

  /**
   * @attention Uniform loading functions bind Shader!
   */
  virtual void LoadUniformFloat2(const std::string& name, const glm::vec2& value) = 0;

  /**
   * @attention Uniform loading functions bind Shader!
   */
  virtual void LoadUniformFloat3(const std::string& name, const glm::vec3& value) = 0;

  /**
   * @attention Uniform loading functions bind Shader!
   */
  virtual void LoadUniformFloat4(const std::string& name, const glm::vec4& value) = 0;

  /**
   * @attention Uniform loading functions bind Shader!
   */
  virtual void LoadUniformMat4(const std::string& name, const glm::mat4& value) = 0;
};

}  // namespace vulture