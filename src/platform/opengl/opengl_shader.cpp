/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file opengl_shader.cpp
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

#include "core/logger.hpp"
#include "platform/opengl/opengl_shader.hpp"

#include <alloca.h>
#include <glad/glad.h>

#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>
#include <string>

using namespace vulture;

struct ShaderProgramSource {
  std::string vertex_source;
  std::string fragment_source;
};

enum class ShaderType { kInvalid = -1, kVertex, kFragment, kTotal };

struct ShaderInfo {
  ShaderType type{ShaderType::kInvalid};
  const char* name{nullptr};
};

static const ShaderInfo kShadersInfo[] = {{ShaderType::kVertex, "vertex"}, {ShaderType::kFragment, "fragment"}};

static ShaderProgramSource ParseShader(const std::string& filename) {
  LOG_INFO(Renderer, "Parsing OpenGL shader source file \"{}\"...", filename);

  std::ifstream stream(filename);
  if (stream.fail()) {
    LOG_ERROR(Renderer, "Failed to open shader source file \"{}\"", filename);
    return {nullptr, nullptr};
  }

  std::stringstream ss[static_cast<int32_t>(ShaderType::kTotal)];
  std::string cur_line;
  int32_t cur_type;

  while (std::getline(stream, cur_line)) {
    if (cur_line.find("#shader") != std::string::npos) {
      for (int32_t i = 0; i < static_cast<int32_t>(ShaderType::kTotal); ++i) {
        if (cur_line.find(kShadersInfo[i].name) != std::string::npos) {
          cur_type = i;
        }
      }
    } else {
      ss[cur_type] << cur_line << '\n';
    }
  }

  LOG_INFO(Renderer, "Successfully parsed OpenGL shader source file \"{}\"", filename);

  return {ss[static_cast<int32_t>(ShaderType::kVertex)].str(), ss[static_cast<int32_t>(ShaderType::kFragment)].str()};
}

static uint32_t CompileShader(uint32_t type, const std::string& source) {
  LOG_INFO(Renderer, "Compiling OpenGL shader (type={})...", type);

  uint32_t id = glCreateShader(type);
  const char* c_source = source.c_str();

  glShaderSource(id, /*count=*/1, &c_source, /*length=*/nullptr);
  glCompileShader(id);

  int32_t result = 0;
  glGetShaderiv(id, GL_COMPILE_STATUS, &result);
  if (result == GL_FALSE) {
    int32_t length = 0;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);

    char* message = (char*)alloca(length * sizeof(char));
    glGetShaderInfoLog(id, length, &length, message);

    LOG_ERROR(Renderer, "Failed to compile OpenGL shader (type={}), OpenGL message:\n{}", type, message);

    glDeleteShader(id);
    return 0;
  }

  LOG_INFO(Renderer, "Successfully compiled OpenGL shader (type={}, id={})", type, id);

  return id;
}

static uint32_t CreateShader(const std::string& vertex_shader, const std::string& fragment_shader) {
  uint32_t vs = CompileShader(GL_VERTEX_SHADER, vertex_shader);
  uint32_t fs = CompileShader(GL_FRAGMENT_SHADER, fragment_shader);

  uint32_t program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);
  glValidateProgram(program);

  glDeleteShader(vs);
  glDeleteShader(fs);

  return program;
}

OpenGLShader::OpenGLShader(const std::string& filename) {
  ShaderProgramSource src = ParseShader(filename);

  id_ = CreateShader(src.vertex_source.c_str(), src.fragment_source.c_str());
  SetAttributeLocations();
}

OpenGLShader::OpenGLShader(const std::string& vertex_shader, const std::string& fragment_shader) {
  id_ = CreateShader(vertex_shader, fragment_shader);
  SetAttributeLocations();
}

OpenGLShader::~OpenGLShader() { glDeleteProgram(id_); }

void OpenGLShader::Bind() const { glUseProgram(id_); }

void OpenGLShader::Unbind() const { glUseProgram(0); }

const AttributeLocationMap& OpenGLShader::GetAttributeLocations() const { return attribute_locations_; }

static void CheckUniformLoadSuccess(GLint uniform_location, const std::string& uniform_name) {
  if (uniform_location == -1) {
    // LOG_WARN(Renderer, "Failed to load uniform \"{}\"", uniform_name);
  }
}

void OpenGLShader::LoadUniformInt(int value, const std::string& name) {
  glUniform1i(GetUniformLocation(name), value);
}

void OpenGLShader::LoadUniformFloat(float value, const std::string& name) {
  glUniform1f(GetUniformLocation(name), value);
}

void OpenGLShader::LoadUniformFloat2(const glm::vec2& value, const std::string& name) {
  glUniform2f(GetUniformLocation(name), value.x, value.y);
}

void OpenGLShader::LoadUniformFloat3(const glm::vec3& value, const std::string& name) {
  glUniform3f(GetUniformLocation(name), value.x, value.y, value.z);
}

void OpenGLShader::LoadUniformFloat4(const glm::vec4& value, const std::string& name) {
  glUniform4f(GetUniformLocation(name), value.x, value.y, value.z, value.w);
}

void OpenGLShader::LoadUniformMat4(const glm::mat4& value, const std::string& name) {
  glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void OpenGLShader::SetAttributeLocations() {
  int32_t attribs_count = 0;
  glGetProgramiv(id_, GL_ACTIVE_ATTRIBUTES, &attribs_count);

  const uint32_t kAttribNameMaxLength = 128;
  char attrib_name[kAttribNameMaxLength];
  int32_t attrib_name_length = 0;

  int32_t attrib_size = 0;
  GLenum attrib_type;

  for (int32_t i = 0; i < attribs_count; i++) {
    glGetActiveAttrib(id_, static_cast<uint32_t>(i), kAttribNameMaxLength, &attrib_name_length, &attrib_size,
                      &attrib_type, attrib_name);
    assert(attrib_name_length != 0);

    int32_t location = glGetAttribLocation(id_, attrib_name);
    assert(location != -1);

    attribute_locations_[attrib_name] = static_cast<uint32_t>(location);
  }
}

GLint OpenGLShader::GetUniformLocation(const std::string& name) {
  Bind();
  GLint location = glGetUniformLocation(id_, name.c_str());
  CheckUniformLoadSuccess(location, name);

  return location;
}