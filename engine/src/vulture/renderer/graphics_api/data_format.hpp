/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file data_format.hpp
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

#include <cstdint>
#include <vulture/core/core.hpp>

namespace vulture {

/**
 * @brief Specifies data format
 * 
 * Naming explained:
 * 1. Letters U/S before the type specify whether the type is unsigned/signed
 * 2. Suffix NORM means the type is normalized, i.e in range [0, 1]
 * 3. Suffix SRGB means the type has sRGB nonlinear encoding
 */
DECLARE_ENUM_TO_STR(DataFormat,
                    kInvalid,

                    /* One-component */
                    kR32_UINT,
                    kR32_SINT,
                    kR32_SFLOAT,

                    kD16_UNORM,
                    kD32_SFLOAT,

                    /* Two-component */
                    kR32G32_UINT,
                    kR32G32_SINT,
                    kR32G32_SFLOAT,

                    kD24_UNORM_S8_UINT,

                    /* Three-component */
                    kR8G8B8_UNORM,
                    kR8G8B8_SRGB,

                    kR32G32B32_SFLOAT,

                    /* Four-component */
                    kR8G8B8A8_UNORM,
                    kR8G8B8A8_SRGB,
                    kB8G8R8A8_SRGB,

                    kR32G32B32A32_SFLOAT);

/**
 * @param format
 * @return Size of the data format in bytes. 
 */
inline uint32_t GetDataFormatSize(DataFormat format) {
  switch (format) {
    /* One-component */
    case (DataFormat::kR32_UINT):            { return 4; }
    case (DataFormat::kR32_SINT):            { return 4; }
    case (DataFormat::kR32_SFLOAT):          { return 4; }

    case (DataFormat::kD16_UNORM):           { return 2; }
    case (DataFormat::kD32_SFLOAT):          { return 4; }

    /* Two-component */
    case (DataFormat::kR32G32_UINT):         { return 8; }
    case (DataFormat::kR32G32_SINT):         { return 8; }
    case (DataFormat::kR32G32_SFLOAT):       { return 8; }

    case (DataFormat::kD24_UNORM_S8_UINT):   { return 4; }

    /* Three-component */
    case (DataFormat::kR8G8B8_UNORM):        { return 3; }
    case (DataFormat::kR8G8B8_SRGB):         { return 3; }
    case (DataFormat::kR32G32B32_SFLOAT):    { return 12; }

    /* Four-component */
    case (DataFormat::kR8G8B8A8_UNORM):      { return 4; }
    case (DataFormat::kR8G8B8A8_SRGB):       { return 4; }
    case (DataFormat::kR32G32B32A32_SFLOAT): { return 16; }

    default: { return 0; }
  }
}

inline bool IsDepthContainingDataFormat(DataFormat format) {
  return format == DataFormat::kD16_UNORM ||
         format == DataFormat::kD32_SFLOAT ||
         format == DataFormat::kD24_UNORM_S8_UINT;
}

inline bool IsDepthStencilDataFormat(DataFormat format) {
  return format == DataFormat::kD24_UNORM_S8_UINT;
}

}  // namespace vulture