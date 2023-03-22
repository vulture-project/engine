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

#include <vulture/renderer/graphics_api/data_format.hpp>

namespace vulture {

/* Texture specification */
enum class TextureType : uint32_t {
  kInvalid,
  kTexture2D,
  kTextureCube
};

enum TextureUsageBit : uint32_t {
  kTextureUsageBitNone            = 0x0000'0000,
  kTextureUsageBitTransferSrc     = 0x0000'0001,
  kTextureUsageBitTransferDst     = 0x0000'0002,
  kTextureUsageBitSampled         = 0x0000'0004,
  kTextureUsageBitColorAttachment = 0x0000'0010,
  kTextureUsageBitDepthAttachment = 0x0000'0020,
  kTextureUsageBitInputAttachment = 0x0000'0080,
};

using TextureUsageFlags = uint32_t;

// TODO: Get rid of?

DECLARE_ENUM_TO_STR(TextureLayout,
                    kUndefined,
                    kGeneral,
                    kPresentSrc,

                    kColorAttachment,
                    kDepthStencilAttachment,
                    kDepthStencilReadOnly,
                    kShaderReadOnly,

                    kTransferSrc,
                    kTransferDst);

struct TextureSpecification {
  DataFormat        format       {DataFormat::kInvalid};
  TextureType       type         {TextureType::kInvalid};
  TextureUsageFlags usage        {kTextureUsageBitNone};
  bool              cpu_readable {false};  ///< Allows CPU to read texels from it (may affect performance).

  uint32_t          width        {0};
  uint32_t          height       {0};
  uint32_t          mip_levels   {1};
  uint32_t          samples      {1};
};

/* Texture sampler */
enum class SamplerFilter : uint32_t {
  kNearest,
  kLinear
};

enum class SamplerAddressMode : uint32_t {
  kRepeat,
  kMirroredRepeat,
  kClampToEdge,
  kClampToBorder,
};

enum class SamplerMipmapMode : uint32_t {
  kNearest,
  kLinear
};

enum class SamplerBorderColor : uint32_t {
  kFloatTransparentBlack,
  kIntTransparentBlack,
  
  kFloatOpaqueBlack,
  kIntOpaqueBlack,

  kFloatOpaqueWhite,
  kIntOpaqueWhite
};

struct SamplerSpecification {
  SamplerFilter      min_filter         {SamplerFilter::kLinear};
  SamplerFilter      mag_filter         {SamplerFilter::kLinear};

  SamplerAddressMode address_mode_u     {SamplerAddressMode::kRepeat};
  SamplerAddressMode address_mode_v     {SamplerAddressMode::kRepeat};
  SamplerAddressMode address_mode_w     {SamplerAddressMode::kRepeat};
  SamplerBorderColor border_color       {SamplerBorderColor::kIntOpaqueBlack};

  bool               anisotropy_enabled {false};
  float              max_anisotropy     {0.0f};

  SamplerMipmapMode  mipmap_mode        {SamplerMipmapMode::kLinear};
  float              min_lod            {0.0f};
  float              max_lod            {0.0f};
  float              lod_bias           {0.0f};

  /* TODO: compare operation */
  /* TODO: unnormalized coordinates */
};

}  // namespace vulture