/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file opengl_texture.cpp
 * @date 2022-04-28
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

#include "renderer/opengl_texture.hpp"

#include <glad/glad.h>
#include <stb_image/stb_image.h>

OpenGLTexture::OpenGLTexture(const std::string& filename) {
  int32_t width, height, channels;
  uint8_t* data = stbi_load(filename.c_str(), &width, &height, &channels, /*desired_channels=*/0);
  assert(data);

  glGenTextures(1, &id_);
  glBindTexture(GL_TEXTURE_2D, id_);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, /*level=*/0, GL_RGB, width, height, /*border=*/0, GL_RGB, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  stbi_image_free(data);
}

OpenGLTexture::~OpenGLTexture() { glDeleteTextures(1, &id_); }

uint32_t OpenGLTexture::GetWidth() const { return width_; }
uint32_t OpenGLTexture::GetHeight() const { return height_; }
uint32_t OpenGLTexture::GetID() const { return id_; }

void OpenGLTexture::Bind(uint32_t slot) const {
  glBindTextureUnit(slot, id_);
}