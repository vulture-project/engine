/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file cube_map.cpp
 * @date 2022-05-14
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

#include <vulture/platform/opengl/opengl_cube_map.hpp>
#include <vulture/renderer/cube_map.hpp>
#include <vulture/renderer/renderer_api.hpp>

using namespace vulture;

SharedPtr<CubeMap> CubeMap::Create(const std::array<std::string, 6>& faces_filenames)
{
  switch (RendererAPI::GetAPI()) {
    case RendererAPI::API::kOpenGL: { return CreateShared<OpenGLCubeMap>(faces_filenames); }
    default:                        { assert(!"Unsupported RendererAPI"); }
  }

  return nullptr;
}