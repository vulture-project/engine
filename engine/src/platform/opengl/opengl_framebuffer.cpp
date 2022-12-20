/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file opengl_framebuffer.cpp
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

#include "platform/opengl/opengl_framebuffer.hpp"

#include <glad/glad.h>

using namespace vulture;

GLenum ConvertToOpenGLInternalFormat(FramebufferAttachmentFormat format) {
  switch (format) {
    case FramebufferAttachmentFormat::kRGBA8:           { return GL_RGBA8; }
    case FramebufferAttachmentFormat::kRGBA32F:         { return GL_RGBA16F; }
    case FramebufferAttachmentFormat::kDepth24Stencil8: { return GL_DEPTH24_STENCIL8; }
    default:                                            { assert(!"Invalid framebuffer attachment format"); }
  }

  return 0;
}

GLenum ConvertToOpenGLFormat(FramebufferAttachmentFormat format) {
  switch (format) {
    case FramebufferAttachmentFormat::kRGBA8:           { return GL_RGBA; }
    case FramebufferAttachmentFormat::kRGBA32F:         { return GL_RGBA; }
    case FramebufferAttachmentFormat::kDepth24Stencil8: { return GL_DEPTH_STENCIL; }
    default:                                            { assert(!"Invalid framebuffer attachment format"); }
  }

  return 0;
}

GLenum ConvertToOpenGLType(FramebufferAttachmentFormat format) {
  switch (format) {
    case FramebufferAttachmentFormat::kRGBA8:           { return GL_UNSIGNED_BYTE; }
    case FramebufferAttachmentFormat::kRGBA32F:         { return GL_FLOAT; }
    case FramebufferAttachmentFormat::kDepth24Stencil8: { return GL_UNSIGNED_INT_24_8; }
    default:                                            { assert(!"Invalid framebuffer attachment format"); }
  }

  return 0;
}

bool IsDepthAttachment(FramebufferAttachmentFormat format) {
  return format == FramebufferAttachmentFormat::kDepth24Stencil8;
}

bool IsColorAttachment(FramebufferAttachmentFormat format) {
  return !IsDepthAttachment(format);
}

OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpec& spec) : specification_(spec) {
  ReinitializeAttachments();
}

OpenGLFramebuffer::~OpenGLFramebuffer() {
  DeleteAttachments();

  GL_CALL(glDeleteFramebuffers(1, &id_));
  id_ = 0;
}

void OpenGLFramebuffer::Bind() const {
  GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, id_));
}

void OpenGLFramebuffer::Unbind() const {
  GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

uint32_t OpenGLFramebuffer::GetColorAttachmentId(uint32_t idx) {
  return GetColorAttachmentOpenGLHandle(idx);
}

void OpenGLFramebuffer::BindColorAttachmentAsTexture(uint32_t idx, uint32_t slot) {
  uint32_t id = GetColorAttachmentOpenGLHandle(idx);

  GL_CALL(glActiveTexture(GL_TEXTURE0 + slot));
  GL_CALL(glBindTexture(GL_TEXTURE_2D, id));
}

void OpenGLFramebuffer::BlitDepthAttachment(Framebuffer* target) {
  OpenGLFramebuffer* opengl_target = dynamic_cast<OpenGLFramebuffer*>(target);

  GL_CALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, id_));
  GL_CALL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, opengl_target->id_));

  GL_CALL(glBlitFramebuffer(0, 0, specification_.width, specification_.height, 0, 0, specification_.width,
                            specification_.height, GL_DEPTH_BUFFER_BIT, GL_NEAREST));

  GL_CALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, 0));
  GL_CALL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
}

FramebufferSpec OpenGLFramebuffer::GetFramebufferSpec() const {
  return specification_;
}

void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height) {
  specification_.width  = width;
  specification_.height = height;
  ReinitializeAttachments();
}

void OpenGLFramebuffer::DeleteAttachments() {
  for (uint32_t i = 0; i < attachments_id_.size(); ++i) {
    if (specification_.attachments[i].can_be_sampled) {
      GL_CALL(glDeleteTextures(1, &attachments_id_[i]));
    } else {
      GL_CALL(glDeleteRenderbuffers(1, &attachments_id_[i]));
    }

    attachments_id_[i] = 0;
  }

  attachments_id_.clear();
}

void OpenGLFramebuffer::ReinitializeAttachments() {
  if (id_ != 0) {
    DeleteAttachments();
    GL_CALL(glDeleteFramebuffers(1, &id_));
  }

  GL_CALL(glGenFramebuffers(1, &id_));
  Bind();

  uint32_t color_attachments = 0;

  for (const auto& attachment : specification_.attachments) {
    uint32_t id{0};
    GLenum format = ConvertToOpenGLFormat(attachment.format);
    GLenum internal_format = ConvertToOpenGLInternalFormat(attachment.format);
    GLenum type = ConvertToOpenGLType(attachment.format);

    if (attachment.can_be_sampled) {
      GL_CALL(glGenTextures(1, &id));
      GL_CALL(glBindTexture(GL_TEXTURE_2D, id));

      GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
      GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
      GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
      GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
      GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

      GL_CALL(glTexImage2D(GL_TEXTURE_2D, /*level=*/0, internal_format, specification_.width, specification_.height,
                           /*border=*/0, format, type, /*pixels=*/nullptr));

      GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));

      if (IsColorAttachment(attachment.format)) {
        GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + color_attachments++, GL_TEXTURE_2D, id,
                                       /*level=*/0));
      } else if (IsDepthAttachment(attachment.format)) {
        GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, id,
                                       /*level=*/0));
      }
    } else {
      GL_CALL(glGenRenderbuffers(1, &id));

      GL_CALL(glBindRenderbuffer(GL_TEXTURE_2D, id));
      GL_CALL(glRenderbufferStorage(GL_RENDERBUFFER, internal_format, specification_.width, specification_.height));
      GL_CALL(glBindRenderbuffer(GL_TEXTURE_2D, 0));

      if (IsColorAttachment(attachment.format)) {
        GL_CALL(
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + color_attachments++, GL_RENDERBUFFER, id));
      } else if (IsDepthAttachment(attachment.format)) {
        GL_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, id));
      }
    }

    attachments_id_.push_back(id);
  }

  std::vector<GLenum> draw_buffers;
  draw_buffers.resize(color_attachments);
  for (uint32_t i = 0; i < color_attachments; ++i) {
    draw_buffers[i] = GL_COLOR_ATTACHMENT0 + i;
  }

  GL_CALL(glDrawBuffers(draw_buffers.size(), draw_buffers.data()));

  Unbind();
}

uint32_t OpenGLFramebuffer::GetColorAttachmentOpenGLHandle(uint32_t idx) {
  uint32_t color_attachments = 0;
  uint32_t i = 0;
  for (; i < attachments_id_.size() && color_attachments < idx; ++i) {
    if (IsColorAttachment(specification_.attachments[i].format)) {
      ++color_attachments;
    }
  }

  if (color_attachments != idx) {
    LOG_ERROR(Renderer, "Invalid color attachment idx = {}, there are {} color attachments", idx, color_attachments);
    return 0;
  }

  return attachments_id_[i];
}
