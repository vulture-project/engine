/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file sampler.cpp
 * @date 2023-03-19
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

#include <vulture/renderer/sampler.hpp>

using namespace vulture;

Sampler::Sampler(RenderDevice& device, SamplerHandle handle)
    : device_(device), handle_(handle), specification_(device_.GetSamplerSpecification(handle)) {}

Sampler::Sampler(RenderDevice& device, const SamplerSpecification& specification)
    : device_(device), specification_(specification) {
  handle_ = device_.CreateSampler(specification);
}

Sampler::~Sampler() {
  if (ValidRenderHandle(handle_)) {
    device_.DeleteSampler(handle_);
  }
}

SamplerHandle Sampler::GetHandle() const {
  return handle_;
}

const SamplerSpecification& Sampler::GetSpecification() const {
  return specification_;
}

void Sampler::Recreate(const SamplerSpecification& specification) {
  if (ValidRenderHandle(handle_)) {
    device_.DeleteSampler(handle_);
  }

  specification_ = specification;
  handle_ = device_.CreateSampler(specification_);
}