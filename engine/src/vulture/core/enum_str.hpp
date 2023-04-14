/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file enum_str.hpp
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
#include <cstring>

#define DECLARE_ENUM_TO_STR(Type, ...)                                                                                 \
  enum class Type : uint32_t { __VA_ARGS__, kCount };                                                                  \
                                                                                                                       \
  namespace detail {                                                                                                   \
  static char kBaseEnumToString##Type[] = {#__VA_ARGS__};                                                              \
  static const char* kTokensEnumToString##Type[static_cast<uint32_t>(Type::kCount)];                                   \
                                                                                                                       \
  static int kTmpTokensEnumToString##Type =                                                                            \
      vulture::detail::TokenizeEnumString(const_cast<char*>(kBaseEnumToString##Type), sizeof(kBaseEnumToString##Type), \
                                          kTokensEnumToString##Type, static_cast<uint32_t>(Type::kCount));             \
  } /* namespace detail */                                                                                             \
                                                                                                                       \
  [[maybe_unused]] inline const char* Type##ToStr(Type value) {                                                        \
    return detail::kTokensEnumToString##Type[static_cast<uint32_t>(value)];                                            \
  }                                                                                                                    \
                                                                                                                       \
  [[maybe_unused]] inline Type StrTo##Type(const char* str) {                                                          \
    for (uint32_t i = 0; i < static_cast<uint32_t>(Type::kCount); ++i) {                                               \
      if (std::strcmp(str, detail::kTokensEnumToString##Type[i]) == 0) {                                               \
        return static_cast<Type>(i);                                                                                   \
      }                                                                                                                \
    }                                                                                                                  \
                                                                                                                       \
    return Type::kCount;                                                                                               \
  }

namespace vulture {
namespace detail {

extern int TokenizeEnumString(char* base, int length, const char* tokens[], int size);

}  // namespace detail
}  // namespace vulture