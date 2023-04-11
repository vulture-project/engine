/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file material_pass.cpp
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

#include <vulture/renderer/material_system/material_pass.hpp>

using namespace vulture;

MaterialPass::MaterialPass(RenderDevice& device, SharedPtr<Shader> shader)
    : device_(device),
      shader_(shader),
      material_used_(shader_->DescriptorSetUsed(Shader::kMaterialSetBit)),
      descriptor_set_idx_(material_used_ ? shader_->GetDescriptorSetIdx(Shader::kMaterialSetBit) : 0) {
  if (material_used_) {
    for (const auto& uniform_buffer : shader->GetReflection().GetUniformBuffers()) {
      assert(property_buffers_count_ < kMaxPipelineShaderModules);

      if (uniform_buffer.set != descriptor_set_idx_) {
        continue;
      }

      PropertyBuffer& property_buffer = property_buffers_[property_buffers_count_++];
      property_buffer.reflected_uniform_buffer = &uniform_buffer;
      property_buffer.handle                  = kInvalidRenderResourceHandle;
      property_buffer.buffer                  = new char[uniform_buffer.size];
    }

    for (const auto& sampler2d : shader_->GetReflection().GetSampler2Ds()) {
      if (sampler2d.set != descriptor_set_idx_) {
        continue;
      }

      TextureSampler& texture_sampler = texture_samplers_[sampler2d.name];
      texture_sampler.texture = nullptr;
      texture_sampler.sampler = nullptr;
      texture_sampler.binding = sampler2d.binding;
    }
  }
}

MaterialPass::~MaterialPass() {
  if (ValidRenderHandle(descriptor_set_)) {
    device_.DeleteDescriptorSet(descriptor_set_);
  }

  for (uint32_t i = 0; i < property_buffers_count_; ++i) {
    if (ValidRenderHandle(property_buffers_[i].handle)) {
      device_.DeleteBuffer(property_buffers_[i].handle);
    }

    delete[] property_buffers_[i].buffer;

    property_buffers_[i].reflected_uniform_buffer = nullptr;
    property_buffers_[i].handle                  = kInvalidRenderResourceHandle;
    property_buffers_[i].buffer                  = nullptr;
  }
}

MaterialPass::MaterialPass(MaterialPass&& other)
    : device_(other.device_), material_used_(other.material_used_), descriptor_set_idx_(other.descriptor_set_idx_) {
  shader_                 = std::move(other.shader_);
  descriptor_set_         = std::move(other.descriptor_set_);
  texture_samplers_       = std::move(other.texture_samplers_);
  property_buffers_count_ = std::move(other.property_buffers_count_);

  for (uint32_t i = 0; i < property_buffers_count_; ++i) {
    property_buffers_[i].reflected_uniform_buffer = other.property_buffers_[i].reflected_uniform_buffer;
    property_buffers_[i].handle                  = other.property_buffers_[i].handle;
    property_buffers_[i].buffer                  = other.property_buffers_[i].buffer;

    other.property_buffers_[i].reflected_uniform_buffer = nullptr;
    other.property_buffers_[i].handle                  = kInvalidRenderResourceHandle;
    other.property_buffers_[i].buffer                  = nullptr;
  }
}

MaterialPass::TextureSampler& MaterialPass::GetTextureSampler(const StringView name) {
  auto it = texture_samplers_.find(name);
  assert(it != texture_samplers_.end());

  return it->second;
}

void MaterialPass::SetTextureSampler(const StringView name, SharedPtr<Texture> texture, SharedPtr<Sampler> sampler) {
  assert(texture);

  auto& texture_sampler = GetTextureSampler(name);
  texture_sampler.texture = texture;

  if (sampler) {
    texture_sampler.sampler = sampler;
  } else {
    texture_sampler.sampler = CreateShared<Sampler>(device_, SamplerSpecification{});
  }
}

DescriptorSetHandle MaterialPass::WriteDescriptorSet() {
  if (!material_used_) {
    return kInvalidRenderResourceHandle;
  }

  if (descriptor_set_ == kInvalidRenderResourceHandle) {
    CreateDescriptorSet();
  }

  for (uint32_t i = 0; i < property_buffers_count_; ++i) {
    PropertyBuffer& property_buffer = property_buffers_[i];

    if (property_buffer.handle == kInvalidRenderResourceHandle) {
      CreateUniformBuffer(property_buffer);
    }

    uint32_t size = property_buffer.reflected_uniform_buffer->size;    
    device_.LoadBufferData(property_buffer.handle, 0, size, property_buffer.buffer);

    device_.WriteDescriptorUniformBuffer(descriptor_set_, property_buffer.reflected_uniform_buffer->binding,
                                         property_buffer.handle, 0, size);
  }

  for (const auto& [name, texture_sampler] : texture_samplers_) {
    device_.WriteDescriptorSampler(descriptor_set_, texture_sampler.binding,
                                   texture_sampler.texture->GetHandle(),
                                   texture_sampler.sampler->GetHandle());
  }

  return descriptor_set_;
}

DescriptorSetHandle MaterialPass::GetDescriptorSet() const {
  VULTURE_ASSERT(ValidRenderHandle(descriptor_set_),
                 "Trying to get material pass' descriptor set without first "
                 "creating it (see WriteDescriptorSet function)");

  return descriptor_set_;
}

void MaterialPass::CreateDescriptorSet() {
  const PipelineDescription& pipeline_description = shader_->GetPipelineDescription();

  assert(pipeline_description.descriptor_sets_count > descriptor_set_idx_);

  descriptor_set_ = device_.CreateDescriptorSet(pipeline_description.descriptor_set_layouts[descriptor_set_idx_]);
  VULTURE_ASSERT(ValidRenderHandle(descriptor_set_), "Invalid handle");
}

void MaterialPass::CreateUniformBuffer(PropertyBuffer& property_buffer) {
  property_buffer.handle = device_.CreateStaticUniformBuffer(property_buffer.reflected_uniform_buffer->size);
  VULTURE_ASSERT(ValidRenderHandle(property_buffer.handle), "Invalid handle");
}