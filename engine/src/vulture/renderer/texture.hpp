/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file texture.hpp
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

#pragma once

#include <vulture/asset/asset.hpp>
#include <vulture/renderer/graphics_api/render_device.hpp>

namespace vulture {

class Texture : public IAsset {
 public:
  Texture(RenderDevice& device, TextureHandle handle);
  Texture(RenderDevice& device, const TextureSpecification& specification);
  ~Texture() override;

  Texture(const Texture& other) = delete;
  Texture& operator=(const Texture& other) = delete;

  Texture(Texture&& other) = delete;
  Texture& operator=(Texture&& other) = delete;

  TextureHandle GetHandle() const;
  const TextureSpecification& GetSpecification() const;

  void Recreate(const TextureSpecification& specification);

 private:
  RenderDevice&        device_;
  TextureHandle        handle_{kInvalidRenderResourceHandle};
  TextureSpecification specification_;
};

}  // namespace vulture