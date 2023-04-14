/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file render_graph_blackboard.hpp
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

#include <any>
#include <typeindex>
#include <unordered_map>

namespace vulture {
namespace rg {

class Blackboard {
 public:
  Blackboard() = default;

  template <typename T, typename... Args>
  T& Add(Args&&... args);

  template <typename T>
  T& Get();

  template <typename T>
  const T& Get() const;

  template <typename T>
  bool Has() const;

 private:
  std::unordered_map<std::type_index, std::any> storage_;  // FIXME: (tralf-strues) Come up with another way to store
};

#include <vulture/renderer/render_graph/blackboard.ipp>

}  // namespace rg
}  // namespace vulture
