/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file texture_loader.cpp
 * @date 2023-03-22
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

#include <stb_image/stb_image.h>
#include <vulture/asset/detail/texture_loader.hpp>

using namespace vulture;

SharedPtr<Texture> detail::LoadTexture(RenderDevice& device, const String& path) {
  int32_t tex_width    = 0;
  int32_t tex_height   = 0;
  int32_t tex_channels = 0;

  stbi_set_flip_vertically_on_load(true);
  stbi_uc* pixels = stbi_load(path.c_str(), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
  if (pixels == nullptr) {
    LOG_ERROR("Texture file \"{}\" not found!", path);
    return nullptr;
  }

  uint32_t tex_mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(tex_width, tex_height)))) + 1;

  TextureSpecification tex_specification{};
  tex_specification.format     = DataFormat::kR8G8B8A8_UNORM;
  tex_specification.usage      = kTextureUsageBitSampled;
  tex_specification.type       = TextureType::kTexture2D;
  tex_specification.width      = static_cast<uint32_t>(tex_width);
  tex_specification.height     = static_cast<uint32_t>(tex_height);
  tex_specification.mip_levels = tex_mip_levels;

  TextureHandle handle = device.CreateTexture(tex_specification);

  /* Copy pixels to texture */
  void* map_data = nullptr;

  uint32_t buffer_size = tex_width * tex_height * 4;
  BufferHandle staging_buffer = device.CreateBuffer(buffer_size, kBufferUsageBitTransferSrc, true, &map_data);

  device.InvalidateBufferMemory(staging_buffer, 0, buffer_size);
  std::memcpy(map_data, pixels, buffer_size);
  device.FlushBufferMemory(staging_buffer, 0, buffer_size);

  CommandBuffer* command_buffer = device.CreateCommandBuffer(CommandBufferType::kGraphics, true);
  command_buffer->Begin();

  command_buffer->TransitionLayout(handle, TextureLayout::kUndefined, TextureLayout::kTransferDst);
  command_buffer->CopyBufferToTexture(staging_buffer, handle, tex_width, tex_height);
  command_buffer->GenerateMipmaps(handle, TextureLayout::kShaderReadOnly);

  command_buffer->End();
  command_buffer->Submit();

  stbi_image_free(pixels);

  return CreateShared<Texture>(device, handle);
}