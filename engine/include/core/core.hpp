/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file core.hpp
 * @date 2022-04-27
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

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <list>
#include <memory>
#include <vector>
#include <string_view>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>

#include "platform/window.hpp"
#include "core/logger.hpp"

#define ASSERT(x, message)    \
  if (!(x)) {                 \
    LOG_ERROR(Core, message); \
    assert(false); \
  }

#define GL_CALL(x)     \
  OpenglClearErrors(); \
  x;                   \
  ASSERT(OpenglLogCall(), "OpenGL Error!");

namespace vulture {

void OpenglClearErrors();

bool OpenglLogCall();

template <typename T>
using SharedPtr = std::shared_ptr<T>;

template <typename T>
using WeakPtr = std::weak_ptr<T>;

template <typename T, typename... Args>
constexpr SharedPtr<T> CreateShared(Args&&... args) {
  return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename T>
using ScopePtr = std::unique_ptr<T>;

template <typename T, typename... Args>
constexpr ScopePtr<T> CreateScope(Args&&... args) {
  return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T>
using Vector = std::vector<T>;

template<typename T>
using List = std::list<T>;

template<typename T>
using UniquePtr = std::unique_ptr<T>;

using String = std::string;

using StringView = std::string_view;

template<typename K, typename V>
using HashMap = std::unordered_map<K, V>;

}  // namespace vulture