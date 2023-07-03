/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file descriptor_set.cpp
 * @date 2023-03-21
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

#include <vulture/renderer/descriptor_set.hpp>

using namespace vulture;

DescriptorSet::~DescriptorSet() {
  if (ValidRenderHandle(layout_handle_)) {
    device_->DeleteDescriptorSetLayout(layout_handle_);
  }

  if (ValidRenderHandle(set_handle_)) {
    device_->DeleteDescriptorSet(set_handle_);
  }
}

const DescriptorSetLayoutInfo& DescriptorSet::GetLayoutInfo() const { return layout_info_; }

DescriptorSetLayoutHandle DescriptorSet::GetLayoutHandle() const {
  VULTURE_ASSERT(ValidRenderHandle(layout_handle_), "Descriptor Set must be built first!");
  return layout_handle_;
}

DescriptorSetHandle DescriptorSet::GetHandle() const {
  VULTURE_ASSERT(ValidRenderHandle(layout_handle_), "Descriptor Set must be built first!");
  return set_handle_;
}

DescriptorSet& DescriptorSet::AddBinding(DescriptorType type, ShaderStageFlags shader_stages) {
  VULTURE_ASSERT(!ValidRenderHandle(layout_handle_) && !ValidRenderHandle(set_handle_),
                 "Descriptor Set is already built!");

  auto& binding = layout_info_.bindings_layout_info.emplace_back();
  binding.binding_idx     = layout_info_.bindings_layout_info.size() - 1;
  binding.descriptor_type = type;
  binding.shader_stages   = shader_stages;

  return *this;
}

void DescriptorSet::Build(RenderDevice& device) {
  device_ = &device;

  layout_handle_ = device_->CreateDescriptorSetLayout(layout_info_);
  set_handle_    = device_->CreateDescriptorSet(layout_handle_);
}