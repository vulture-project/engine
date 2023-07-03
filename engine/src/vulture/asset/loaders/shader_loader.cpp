/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file shader_loader.cpp
 * @date 2023-03-22
 * 
 * The MIT License (MIT)
 * Copyright (c) 2022 Nikita Mochalov
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

#include <vulture/asset/detail/asset_loader_registrar.hpp>
#include <vulture/asset/loaders/shader_loader.hpp>
#include <vulture/renderer/material_system/shader.hpp>

namespace vulture {

ShaderLoader::ShaderLoader(RenderDevice& device) : device_(device) {}

StringView ShaderLoader::Extension() const {
  return StringView{".shader"};
}

SharedPtr<IAsset> ShaderLoader::Load(const String& path) {
  SharedPtr<Shader> shader = CreateShared<Shader>(device_);
  if (!shader->Load(path)) {
    return nullptr;
  }

  return shader;
}

}  // namespace vulture