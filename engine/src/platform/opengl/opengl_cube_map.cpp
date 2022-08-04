/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file opengl_cube_map.cpp
 * @date 2022-05-14
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

#include "platform/opengl/opengl_cube_map.hpp"

#include <glad/glad.h>
#include <stb_image/stb_image.h>

#include "core/logger.hpp"

using namespace vulture;

OpenGLCubeMap::OpenGLCubeMap(const std::array<std::string, 6>& faces_filenames) {
  LOG_INFO(Renderer, "Loading OpenGL cube map");

  stbi_set_flip_vertically_on_load(false); // FIXME:

  GL_CALL(glGenTextures(1, &id_));
  GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, id_));

  GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
  GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
  GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
  GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
  GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));

  int32_t width = 0, height = 0, channels = 0;
  uint8_t* data = nullptr;
  for (unsigned int i = 0; i < faces_filenames.size(); i++) {
    data = stbi_load(faces_filenames[i].c_str(), &width, &height, &channels, 0);

    if (data == nullptr) {
      LOG_ERROR(Renderer, "Failed to load OpenGL cube map texture \"{}\"", faces_filenames[i]);
      return;
    }

    GLenum format = channels == 3 ? GL_RGB : GL_RGBA;

    GL_CALL(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, /*level=*/0, format, width, height, /*border=*/0, format,
                         GL_UNSIGNED_BYTE, data));

    stbi_image_free(data);
  }

  LOG_INFO(Renderer, "Successfully loaded OpenGL cube map (id={})", id_);
}

OpenGLCubeMap::~OpenGLCubeMap() { glDeleteTextures(1, &id_); }

uint32_t OpenGLCubeMap::GetID() const { return id_; }

void OpenGLCubeMap::Bind(uint32_t slot) const {
  // glBindTextureUnit(slot, id_); NOTE: for OpenGL version 4.5 and later!
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_CUBE_MAP, id_);
}