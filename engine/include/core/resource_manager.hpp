/**
 * @author Sergey Zelenkin (https://github.com/vssense)
 * @file resource_manager.hpp
 * @date 2022-05-17
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

#include "core/core.hpp"
#include "renderer/shader.hpp"
#include "renderer/3d/mesh.hpp"
#include "renderer/texture.hpp"

#include <unordered_map>

namespace vulture {

class ResourceManager {
 public:
  static SharedPtr<Shader>  LoadShader (const std::string& path);
  static SharedPtr<Mesh>    LoadMesh   (const std::string& path);
  static SharedPtr<Texture> LoadTexture(const std::string& path);

 private:
  static std::unordered_map<std::string, WeakPtr<Shader>> shaders_;
  static std::unordered_map<std::string, WeakPtr<Mesh>> meshes_;
  static std::unordered_map<std::string, WeakPtr<Texture>> textures_;
};

} // namespace vulture
