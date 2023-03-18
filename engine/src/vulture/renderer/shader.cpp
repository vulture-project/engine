/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file shader.cpp
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

#include <fstream>
#include <sstream>
#include <vulture/core/logger.hpp>
#include <vulture/platform/opengl/opengl_shader.hpp>
#include <vulture/renderer/renderer_api.hpp>
#include <vulture/renderer/shader.hpp>

using namespace vulture;

SharedPtr<Shader> Shader::Create(const std::string& filename) {
  switch (RendererAPI::GetAPI()) {
    case RendererAPI::API::kOpenGL: { return CreateShared<OpenGLShader>(filename); }
    default:                        { assert(!"Unsupported RendererAPI"); }
  }

  return nullptr;
}

// static SharedPtr<Shader> Create(const std::string& filename_vs, const std::string& filename_fs) {
//   switch (RendererAPI::GetAPI()) {
//     case RendererAPI::API::kOpenGL: {
//       std::ifstream file_vs(filename_vs);
//       if (file_vs.fail()) {
//         LOG_WARN(Renderer, "Unable to locate file \"{}\"", filename_vs);
//         return nullptr;
//       }

//       std::ifstream file_fs(filename_fs);
//       if (file_fs.fail()) {
//         LOG_WARN(Renderer, "Unable to locate file \"{}\"", filename_fs);
//         return nullptr;
//       }

//       std::stringstream buffer;

//       buffer << file_vs.rdbuf();
//       std::string vertex_shader = buffer.str();

//       buffer << file_fs.rdbuf();
//       std::string fragment_shader = buffer.str();

//       return CreateShared<OpenGLShader>(vertex_shader, fragment_shader);
//     }
//     default: {
//       assert(!"Unsupported RendererAPI");
//     }
//   }

//   return nullptr;
// }

SharedPtr<Shader> Shader::Create(const std::string& vertex_shader, const std::string& fragment_shader) {
  switch (RendererAPI::GetAPI()) {
    case RendererAPI::API::kOpenGL: { return CreateShared<OpenGLShader>(vertex_shader, fragment_shader); }
    default:                        { assert(!"Unsupported RendererAPI"); }
  }

  return nullptr;
}