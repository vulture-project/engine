/**
 * @author Sergey Zelenkin (https://github.com/vssense)
 * @file resource_manager.cpp
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

#include "core/logger.hpp"
#include "core/resource_manager.hpp"
#include "resource_loaders/parse_obj.hpp"

namespace vulture {

std::unordered_map<std::string, WeakPtr<Shader>> ResourceManager::shaders_{};
std::unordered_map<std::string, WeakPtr<Mesh>> ResourceManager::meshes_{};
std::unordered_map<std::string, WeakPtr<Texture>> ResourceManager::textures_{};

SharedPtr<Shader> ResourceManager::LoadShader(const std::string& path) {
  LOG_INFO(ResourceManager, "Loading shader from {}", path);
  
  if (auto it = shaders_.find(path); it != shaders_.end()) {
    LOG_INFO(ResourceManager, "Shader was already loaded");
    return SharedPtr<Shader>(it->second);
  }

  LOG_INFO(ResourceManager, "Load from file");
  SharedPtr<Shader> shader = Shader::Create(path);
  shaders_[path] = WeakPtr<Shader>(shader);

  return shader;
}

SharedPtr<Mesh> ResourceManager::LoadMesh(const std::string& path) {
  LOG_INFO(ResourceManager, "Loading mesh from {}", path);
  if (auto it = meshes_.find(path); it != meshes_.end()) {
    LOG_INFO(ResourceManager, "Mesh was already loaded");
    return SharedPtr<Mesh>(it->second);
  }

  LOG_INFO(ResourceManager, "Load from file");
  SharedPtr<Mesh> mesh = ParseMeshWavefront(path);
  meshes_[path] = WeakPtr<Mesh>(mesh);

  return mesh;
}

SharedPtr<Texture> ResourceManager::LoadTexture(const std::string& path) {
  LOG_INFO(ResourceManager, "Loading texture from {}", path);

  if (auto it = textures_.find(path); it != textures_.end()) {
    LOG_INFO(ResourceManager, "Texture was already loaded");
    return SharedPtr<Texture>(it->second);
  }

  LOG_INFO(ResourceManager, "Load from file");
  SharedPtr<Texture> texture = Texture::Create(path);
  textures_[path] = WeakPtr<Texture>(texture);

  return texture;
}

} // namespace vulture
