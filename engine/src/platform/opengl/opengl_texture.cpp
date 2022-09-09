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

#include "platform/opengl/opengl_texture.hpp"

#include <glad/glad.h>
#include <stb_image/stb_image.h>

#include "core/logger.hpp"

using namespace vulture;

OpenGLTexture::OpenGLTexture(const std::string& filename) {
  LOG_INFO(Renderer, "Loading OpenGL texture \"{}\"", filename);

  stbi_set_flip_vertically_on_load(true);  // FIXME:

  int32_t width, height, channels;
  uint8_t* data = stbi_load(filename.c_str(), &width, &height, &channels, /*desired_channels=*/0);

  if (data == nullptr) {
    LOG_ERROR(Renderer, "Failed to load OpenGL texture \"{}\"", filename);
    return;
  }

  LOG_DEBUG(Renderer, "Loaded image (width = {}, height = {}, channels = {})", width, height, channels);

  GLenum internal_format = 0;
  GLenum data_format = 0;

  switch (channels) {
    case 4: {
      internal_format = GL_RGBA8;
      data_format = GL_RGBA;
      break;
    }

    case 3: {
      internal_format = GL_RGB8;
      data_format = GL_RGB;
      break;
    }

    default: {
      LOG_ERROR(Renderer, "Invalid number of image channels, {}", channels);
    }
  }

  width_ = width;
  height_ = height;

  GL_CALL(glGenTextures(1, &id_));
  GL_CALL(glBindTexture(GL_TEXTURE_2D, id_));

  GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
  GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
  GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
  GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

  GL_CALL(glTexImage2D(GL_TEXTURE_2D, /*level=*/0, internal_format, width, height, /*border=*/0, data_format,
                       GL_UNSIGNED_BYTE, data));

  GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));

  stbi_image_free(data);

  LOG_INFO(Renderer, "Successfully loaded OpenGL texture \"{}\" (id={})", filename, id_);
}

OpenGLTexture::~OpenGLTexture() { /*GL_CALL(glDeleteTextures(1, &id_));*/
}

uint32_t OpenGLTexture::GetWidth() const { return width_; }
uint32_t OpenGLTexture::GetHeight() const { return height_; }
uint32_t OpenGLTexture::GetID() const { return id_; }

void OpenGLTexture::Bind(uint32_t slot) const {
  // glBindTextureUnit(slot, id_); NOTE: for OpenGL version 4.5 and later!
  GL_CALL(glActiveTexture(GL_TEXTURE0 + slot));
  GL_CALL(glBindTexture(GL_TEXTURE_2D, id_));
}