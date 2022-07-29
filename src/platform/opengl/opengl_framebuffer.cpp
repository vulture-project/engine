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

GLenum ConvertToOpenGLFormat(FramebufferAttachmentFormat format) {
  switch (format) {
    case FramebufferAttachmentFormat::kRGBA8:           { return GL_RGBA8; }
    case FramebufferAttachmentFormat::kDepth24Stencil8: { return GL_DEPTH24_STENCIL8; }
    default:                                            { assert(!"Invalid framebuffer attachment format"); }
  }

  return 0;
}

bool IsColorAttachment(FramebufferAttachmentFormat format) {
  return format == FramebufferAttachmentFormat::kRGBA8;
}

bool IsDepthAttachment(FramebufferAttachmentFormat format) {
  return format == FramebufferAttachmentFormat::kDepth24Stencil8;
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

  uint32_t color_attachments = 0;

  for (const auto& attachment : specification_.attachments) {
    uint32_t attachment_id{0};
    GLenum attachment_format = ConvertToOpenGLFormat(attachment.format);

    if (attachment.can_be_sampled) {
      GL_CALL(glGenTextures(1, &attachment_id));
      GL_CALL(glBindTexture(GL_TEXTURE_2D, attachment_id));

      GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
      GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
      GL_CALL(glTexImage2D(GL_TEXTURE_2D, /*level=*/0, attachment_format, specification_.width, specification_.height,
                           /*border=*/0, attachment_format, GL_UNSIGNED_BYTE, /*pixels=*/nullptr));

      GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));

      if (IsColorAttachment(attachment.format)) {
        GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + color_attachments, GL_TEXTURE_2D,
                                       attachment_id,
                                       /*level=*/0));
      } else if (IsDepthAttachment(attachment.format)) {
        GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, attachment_id,
                                       /*level=*/0));
      }
    } else {
      GL_CALL(glGenRenderbuffers(1, &attachment_id));
      GL_CALL(glBindRenderbuffer(GL_TEXTURE_2D, attachment_id));

      GL_CALL(glRenderbufferStorage(GL_RENDERBUFFER, attachment_format, specification_.width, specification_.height));

      GL_CALL(glBindRenderbuffer(GL_TEXTURE_2D, 0));

      if (IsColorAttachment(attachment.format)) {
        GL_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + color_attachments, GL_RENDERBUFFER,
                                          attachment_id));
      } else if (IsDepthAttachment(attachment.format)) {
        GL_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, attachment_id));
      }
    }

    attachments_id_.push_back(attachment_id);
  }
}