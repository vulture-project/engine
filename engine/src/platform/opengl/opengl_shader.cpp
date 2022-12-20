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
  int32_t cur_type{-1};

  while (std::getline(stream, cur_line)) {
    if (cur_line.find("#shader") != std::string::npos) {
      for (int32_t i = 0; i < static_cast<int32_t>(ShaderType::kTotal); ++i) {
        if (cur_line.find(kShadersInfo[i].name) != std::string::npos) {
          cur_type = i;
        }
      }
    } else if (cur_type != -1) {
      ss[cur_type] << cur_line << '\n';
    } else {
      LOG_ERROR(Renderer, "No shader type specification found (like \"#shader vertex\") in {}!", filename);
      return {nullptr, nullptr};
    }
  }

  LOG_INFO(Renderer, "Successfully parsed OpenGL shader source file \"{}\"", filename);

  // printf("Vertex shader:\n%s\n",   ss[static_cast<int32_t>(ShaderType::kVertex)].str().c_str());
  // printf("Fragment shader:\n%s\n", ss[static_cast<int32_t>(ShaderType::kFragment)].str().c_str());

  return {ss[static_cast<int32_t>(ShaderType::kVertex)].str(), ss[static_cast<int32_t>(ShaderType::kFragment)].str()};
}

static uint32_t CompileShader(uint32_t type, const std::string& source) {
  LOG_INFO(Renderer, "Compiling OpenGL shader (type={})...", type);

  uint32_t id = glCreateShader(type);
  const char* c_source = source.c_str();

  GL_CALL(glShaderSource(id, /*count=*/1, &c_source, /*length=*/nullptr));
  GL_CALL(glCompileShader(id));

  int32_t result = 0;
  GL_CALL(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
  if (result == GL_FALSE) {
    int32_t length = 0;
    GL_CALL(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));

    char* message = (char*)alloca(length * sizeof(char));
    GL_CALL(glGetShaderInfoLog(id, length, &length, message));

    LOG_ERROR(Renderer, "Failed to compile OpenGL shader (type={}), OpenGL message:\n{}", type, message);

    GL_CALL(glDeleteShader(id));
    return 0;
  }

  LOG_INFO(Renderer, "Successfully compiled OpenGL shader (type={}, id={})", type, id);

  return id;
}

static uint32_t CreateShader(const std::string& vertex_shader, const std::string& fragment_shader) {
  uint32_t vs = CompileShader(GL_VERTEX_SHADER, vertex_shader);
  uint32_t fs = CompileShader(GL_FRAGMENT_SHADER, fragment_shader);

  uint32_t program = glCreateProgram();
  GL_CALL(glAttachShader(program, vs));
  GL_CALL(glAttachShader(program, fs));
  GL_CALL(glLinkProgram(program));
  GL_CALL(glValidateProgram(program));

  GL_CALL(glDeleteShader(vs));
  GL_CALL(glDeleteShader(fs));

  return program;
}

GLenum GetGLCullMode(CullMode cull_mode) {
  switch (cull_mode) {
    case CullMode::kFrontOnly:    { return GL_FRONT; }
    case CullMode::kBackOnly:     { return GL_BACK; }
    case CullMode::kFrontAndBack: { return GL_FRONT_AND_BACK; }

    default: { ASSERT(false, "Invalid CullMode!"); }
  };
}

GLenum GetGLDepthFunc(CompareOperation compare_op) {
  switch (compare_op) {
    case CompareOperation::kNever:          { return GL_NEVER; }
    case CompareOperation::kLess:           { return GL_LESS; }
    case CompareOperation::kEqual:          { return GL_EQUAL; }
    case CompareOperation::kLessOrEqual:    { return GL_LEQUAL; }
    case CompareOperation::kGreater:        { return GL_GREATER; }
    case CompareOperation::kNotEqual:       { return GL_NOTEQUAL; }
    case CompareOperation::kGreaterOrEqual: { return GL_GEQUAL; }
    case CompareOperation::kAlways:         { return GL_ALWAYS; }

    default: { ASSERT(false, "Invalid CompareOperation!"); }
  };
}

GLenum GetGLBlendEquation(ColorBlendOperation blend_operation) {
  switch (blend_operation) {
    case ColorBlendOperation::kAdd:             { return GL_FUNC_ADD; }
    case ColorBlendOperation::kSubtract:        { return GL_FUNC_SUBTRACT; }
    case ColorBlendOperation::kReverseSubtract: { return GL_FUNC_REVERSE_SUBTRACT; }
    case ColorBlendOperation::kMin:             { return GL_MIN; }
    case ColorBlendOperation::kMax:             { return GL_MAX; }

    default: { ASSERT(false, "Invalid ColorBlendOperation!"); }
  };
}

GLenum GetGLBlendFactor(ColorBlendFactor blend_factor) {
  switch (blend_factor) {
    case ColorBlendFactor::kZero:              { return GL_ZERO; }
    case ColorBlendFactor::kOne:               { return GL_ONE; }

    case ColorBlendFactor::kSrcColor:          { return GL_SRC_COLOR; }
    case ColorBlendFactor::kOneMinusSrcColor:  { return GL_ONE_MINUS_SRC_COLOR; }
    case ColorBlendFactor::kDstColor:          { return GL_DST_COLOR; }
    case ColorBlendFactor::kOneMinusDstColor:  { return GL_ONE_MINUS_DST_COLOR; }
    
    case ColorBlendFactor::kSrcAlpha:          { return GL_SRC_ALPHA; }
    case ColorBlendFactor::kOneMinusSrcAlpha:  { return GL_ONE_MINUS_SRC_ALPHA; }
    case ColorBlendFactor::kDstAlpha:          { return GL_DST_ALPHA; }
    case ColorBlendFactor::kOneMinusDstAlpha:  { return GL_ONE_MINUS_DST_ALPHA; }

    default: { ASSERT(false, "Invalid ColorBlendFactor!"); }
  };
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

OpenGLShader::~OpenGLShader() { GL_CALL(glDeleteProgram(id_)); }

void OpenGLShader::Bind() const { GL_CALL(glUseProgram(id_)); }

void OpenGLShader::Unbind() const { GL_CALL(glUseProgram(0)); }

void OpenGLShader::SetUpPipeline() const {
  /* Face culling */
  if (cull_mode_ == CullMode::kNone) {
    glDisable(GL_CULL_FACE);
  } else {
    glEnable(GL_CULL_FACE);
    glCullFace(GetGLCullMode(cull_mode_));
  }

  /* Depth testing */
  if (!enable_depth_test_) {
    glDisable(GL_DEPTH_TEST);
  } else {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GetGLDepthFunc(depth_compare_op_));
  }

  glDepthMask(enable_depth_write_);

  /* Color blending */
  if (!enable_blending_) {
    glDisable(GL_BLEND);
  } else {
    glEnable(GL_BLEND);

    glBlendEquation(GetGLBlendEquation(blend_op_));
    glBlendFunc(GetGLBlendFactor(src_blend_factor_), GetGLBlendFactor(dst_blend_factor_));
  }
}

const AttributeLocationMap& OpenGLShader::GetAttributeLocations() const { return attribute_locations_; }

static void CheckUniformLoadSuccess(GLint uniform_location, const std::string& uniform_name) {
  if (uniform_location == -1) {
    // LOG_WARN(Renderer, "Failed to load uniform \"{}\"", uniform_name);
  }
}

void OpenGLShader::LoadUniformInt(int value, const std::string& name) {
  GL_CALL(glUniform1i(GetUniformLocation(name), value));
}

void OpenGLShader::LoadUniformFloat(float value, const std::string& name) {
  GL_CALL(glUniform1f(GetUniformLocation(name), value));
}

void OpenGLShader::LoadUniformFloat2(const glm::vec2& value, const std::string& name) {
  GL_CALL(glUniform2f(GetUniformLocation(name), value.x, value.y));
}

void OpenGLShader::LoadUniformFloat3(const glm::vec3& value, const std::string& name) {
  GL_CALL(glUniform3f(GetUniformLocation(name), value.x, value.y, value.z));
}

void OpenGLShader::LoadUniformFloat4(const glm::vec4& value, const std::string& name) {
  GL_CALL(glUniform4f(GetUniformLocation(name), value.x, value.y, value.z, value.w));
}

void OpenGLShader::LoadUniformMat4(const glm::mat4& value, const std::string& name) {
  GL_CALL(glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value)));
}

void OpenGLShader::LoadUniformBool(bool value, const std::string& name) {
  GL_CALL(glUniform1i(GetUniformLocation(name), value));
}

void OpenGLShader::SetAttributeLocations() {
  int32_t attribs_count = 0;
  GL_CALL(glGetProgramiv(id_, GL_ACTIVE_ATTRIBUTES, &attribs_count));

  const uint32_t kAttribNameMaxLength = 128;
  char attrib_name[kAttribNameMaxLength];
  int32_t attrib_name_length = 0;

  int32_t attrib_size = 0;
  GLenum attrib_type;

  for (int32_t i = 0; i < attribs_count; i++) {
    GL_CALL(glGetActiveAttrib(id_, static_cast<uint32_t>(i), kAttribNameMaxLength, &attrib_name_length, &attrib_size,
                              &attrib_type, attrib_name));
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