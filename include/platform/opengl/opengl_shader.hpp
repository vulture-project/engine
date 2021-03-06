/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file opengl_shader.hpp
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

#include "renderer/shader.hpp"

namespace vulture {

class OpenGLShader : public Shader {
 public:
  OpenGLShader(const std::string& filename);
  OpenGLShader(const std::string& vertex_shader, const std::string& fragment_shader);

  virtual ~OpenGLShader() override;

  virtual void Bind() const override;
  virtual void Unbind() const override;

  virtual const AttributeLocationMap& GetAttributeLocations() const override;

  virtual void LoadUniformInt(int value, const std::string& name) override;
  virtual void LoadUniformFloat(float value, const std::string& name) override;
  virtual void LoadUniformFloat2(const glm::vec2& value, const std::string& name) override;
  virtual void LoadUniformFloat3(const glm::vec3& value, const std::string& name) override;
  virtual void LoadUniformFloat4(const glm::vec4& value, const std::string& name) override;
  virtual void LoadUniformMat4(const glm::mat4& value, const std::string& name) override;
  virtual void LoadUniformBool(bool value, const std::string& name) override;

 private:
  void SetAttributeLocations();
  int GetUniformLocation(const std::string& name);

 private:
  uint32_t id_{0};
  AttributeLocationMap attribute_locations_;
};

}  // namespace vulture