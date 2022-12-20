/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file framebuffer.hpp
 * @date 2022-07-28
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

#include <vector>

#include "core/core.hpp"

namespace vulture {

enum class FramebufferAttachmentFormat {
  kInvalid,

  // Color attachments
  kRGBA8,
  kRGBA32F,

  // Depth and stencil attachments
  kDepth24Stencil8,

  kTotal
};

struct FramebufferAttachmentSpec {
  // TODO: (tralf_strues) add filtering and wrap specs
  FramebufferAttachmentFormat format{FramebufferAttachmentFormat::kInvalid};
  bool can_be_sampled{true};

  FramebufferAttachmentSpec() = default;
  FramebufferAttachmentSpec(FramebufferAttachmentFormat format, bool can_be_sampled = true)
      : format(format), can_be_sampled(can_be_sampled) {}
};

struct FramebufferSpec {
  uint32_t width{0};
  uint32_t height{0};
  std::vector<FramebufferAttachmentSpec> attachments;
};

class Framebuffer {
 public:
  static Framebuffer* Create(const FramebufferSpec& spec);

 public:
  virtual ~Framebuffer() = default;

  virtual void Bind() const = 0;
  virtual void Unbind() const = 0;

  virtual uint32_t GetColorAttachmentId(uint32_t idx = 0) = 0;
  virtual void BindColorAttachmentAsTexture(uint32_t idx, uint32_t slot) = 0;

  virtual void BlitDepthAttachment(Framebuffer* target) = 0;

  virtual FramebufferSpec GetFramebufferSpec() const = 0;

  virtual void Resize(uint32_t width, uint32_t height) = 0;
};

}  // namespace vulture