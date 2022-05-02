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

#include <alloca.h>
#include <glad/glad.h>

#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>
#include <string>

#include "platform/opengl/opengl_shader.hpp"

using namespace vulture;

struct ShaderProgramSource {
  std::string vertex_source;
  std::string fragment_source;
};

static ShaderProgramSource ParseShader(const std::string& filename) {
  enum class ShaderType { kInvalid = -1, kVertex, kFragment, kTotal };

  struct ShaderInfo {
    ShaderType type{ShaderType::kInvalid};
    const char* name{nullptr};
  };

  const ShaderInfo kShadersInfo[] = {{ShaderType::kVertex, "vertex"}, {ShaderType::kFragment, "fragment"}};

  std::ifstream stream(filename);
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

  return {ss[static_cast<int32_t>(ShaderType::kVertex)].str(), ss[static_cast<int32_t>(ShaderType::kFragment)].str()};
}

static uint32_t CompileShader(uint32_t type, const std::string& source) {
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

    std::cout << "Failed to compile shader of type " << type << ":\n\t" << message << std::endl;

    glDeleteShader(id);
    return 0;
  }

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

  std::cout << "Parsed \"" << filename << "\":\n";
  std::cout << "Vertex Shader:\n" << src.vertex_source << "\n";
  std::cout << "Fragment Shader:\n" << src.fragment_source << "\n";

  id_ = CreateShader(src.vertex_source.c_str(), src.fragment_source.c_str());
}

OpenGLShader::OpenGLShader(const std::string& vertex_shader, const std::string& fragment_shader) {
  id_ = CreateShader(vertex_shader, fragment_shader);
}

OpenGLShader::~OpenGLShader() { glDeleteProgram(id_); }

void OpenGLShader::Bind() const { glUseProgram(id_); }

void OpenGLShader::Unbind() const { glUseProgram(0); }

void OpenGLShader::LoadUniformInt(const std::string& name, int value) {
  Bind();
  GLint location = glGetUniformLocation(id_, name.c_str());
  glUniform1i(location, value);
}

void OpenGLShader::LoadUniformFloat(const std::string& name, float value) {
  Bind();
  GLint location = glGetUniformLocation(id_, name.c_str());
  glUniform1f(location, value);
}

void OpenGLShader::LoadUniformFloat2(const std::string& name, const glm::vec2& value) {
  Bind();
  GLint location = glGetUniformLocation(id_, name.c_str());
  glUniform2f(location, value.x, value.y);
}

void OpenGLShader::LoadUniformFloat3(const std::string& name, const glm::vec3& value) {
  Bind();
  GLint location = glGetUniformLocation(id_, name.c_str());
  glUniform3f(location, value.x, value.y, value.z);
}

void OpenGLShader::LoadUniformFloat4(const std::string& name, const glm::vec4& value) {
  Bind();
  GLint location = glGetUniformLocation(id_, name.c_str());
  glUniform4f(location, value.x, value.y, value.z, value.w);
}

void OpenGLShader::LoadUniformMat4(const std::string& name, const glm::mat4& value) {
  Bind();
  GLint location = glGetUniformLocation(id_, name.c_str());
  glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}