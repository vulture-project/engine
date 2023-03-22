/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file per_renderpass_data.hpp
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

#include <string>
#include <unordered_map>

namespace vulture {

using RenderPassId = uint64_t;

constexpr RenderPassId GeneratePassIdFromString(const std::string& pass_name) {
  auto hash_fn = std::hash<std::string>{};
  return hash_fn(pass_name);
}

template <typename T>
class PerRenderPassData {
 public:
  T& operator[](RenderPassId pass_id) {
    auto it = data_.find(pass_id);
    assert(it != data_.end());

    return it->second;
  };

  void Emplace(RenderPassId pass_id, T&& value) {
    data_.emplace(pass_id, std::forward<T>(value));
  }

  bool Contains(RenderPassId pass_id) {
    return data_.find(pass_id) != data_.end();
  };

  bool Contains(RenderPassId pass_id) const {
    return data_.find(pass_id) != data_.cend();
  };

 private:
  std::unordered_map<RenderPassId, T> data_;
};

}  // namespace vulture