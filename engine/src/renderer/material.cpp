/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file material.cpp
 * @date 2022-05-07
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

#include "renderer/material.hpp"

using namespace vulture;

Material::Material(SharedPtr<Shader> shader) : shader_(shader) {}

void Material::LoadUniformsToShader() {
  shader_->Bind();

  for (const auto& uniform_int : uniforms_int_) {
    shader_->LoadUniformInt(uniform_int.second, uniform_int.first);
  }

  for (const auto& uniform_float : uniforms_float_) {
    shader_->LoadUniformFloat(uniform_float.second, uniform_float.first);
  }

  for (const auto& uniform_float2 : uniforms_float2_) {
    shader_->LoadUniformFloat2(uniform_float2.second, uniform_float2.first);
  }

  for (const auto& uniform_float3 : uniforms_float3_) {
    shader_->LoadUniformFloat3(uniform_float3.second, uniform_float3.first);
  }

  for (const auto& uniform_float4 : uniforms_float4_) {
    shader_->LoadUniformFloat4(uniform_float4.second, uniform_float4.first);
  }

  for (const auto& uniform_mat4 : uniforms_mat4_) {
    shader_->LoadUniformMat4(uniform_mat4.second, uniform_mat4.first);
  }

  for (const auto& uniform_bool : uniforms_bool_) {
    shader_->LoadUniformBool(uniform_bool.second, uniform_bool.first);
  }

  uint32_t texture_slot = 0;
  for (const auto& [name, texture] : textures_) {
    texture->Bind(texture_slot);
    shader_->LoadUniformInt(texture_slot, name);
    ++texture_slot;
  }

  texture_slot = 0;
  for (const auto& [name, cube_map] : cube_maps_) {
    cube_map->Bind(texture_slot);
    shader_->LoadUniformInt(texture_slot, name);
    ++texture_slot;
  }
}

void Material::SetShader(SharedPtr<Shader> shader) { shader_ = shader; }
Shader* Material::GetShader() { return shader_.get(); }

void Material::AddTexture(SharedPtr<Texture> texture, const std::string& name) { textures_[name] = texture; }
void Material::AddCubeMap(SharedPtr<CubeMap> cube_map, const std::string& name) { cube_maps_[name] = cube_map; }