/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file material_pass.hpp
 * @date 2023-03-18
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

#pragma once

#include <unordered_map>
#include <vulture/renderer/material_system/shader.hpp>
#include <vulture/renderer/sampler.hpp>
#include <vulture/renderer/texture.hpp>

namespace vulture {

constexpr uint32_t kFrameDescriptorSetIdx    = 0;
constexpr uint32_t kViewDescriptorSetIdx     = 1;
constexpr uint32_t kSceneDescriptorSetIdx    = 2;
constexpr uint32_t kMaterialDescriptorSetIdx = 3;

class MaterialPass {
 public:
   struct TextureSampler {
    SharedPtr<Texture> texture{nullptr};
    SharedPtr<Sampler> sampler{nullptr};
    uint32_t           binding{0};
  };

  explicit MaterialPass(RenderDevice& device, SharedPtr<Shader> shader);
  ~MaterialPass();

  MaterialPass(MaterialPass&& other) = default;
  MaterialPass& operator=(MaterialPass&& other) = delete;

  MaterialPass(const MaterialPass& other) = delete;
  MaterialPass& operator=(const MaterialPass& other) = delete;

  inline Shader& GetShader() { return *shader_.get(); }
  inline const Shader& GetShader() const { return *shader_.get(); }

  template<typename T>
  T& GetProperty(const StringView name);

  TextureSampler& GetTextureSampler(const StringView name);

  // TODO: (tralf-strues) come up with some sort of caching system to only update if changed
  DescriptorSetHandle WriteDescriptorSet();

 private:
  struct PropertyBuffer {
    const ShaderReflection::UniformBuffer* reflected_uniform_buffer{nullptr};

    BufferHandle handle{kInvalidRenderResourceHandle};
    char* buffer{nullptr};
  };

 private:
  void CreateDescriptorSet();
  void CreateUniformBuffer(PropertyBuffer& property_buffer);

 private:
  RenderDevice& device_;
  SharedPtr<Shader> shader_;

  DescriptorSetHandle descriptor_set_{kInvalidRenderResourceHandle};
  
  /* Texture samplers */
  HashMap<StringView, TextureSampler> texture_samplers_;

  /* Uniform buffers for properties per shader stage */
  uint32_t property_buffers_count_{0};
  PropertyBuffer property_buffers_[kMaxPipelineShaderModules];
};

#include <vulture/renderer/material_system/material_pass.ipp>

}  // namespace vulture
