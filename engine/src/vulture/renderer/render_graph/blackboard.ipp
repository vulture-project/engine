/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file render_graph_blackboard.ipp
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

template <typename T, typename... Args>
T& Blackboard::Add(Args&&... args) {
  return storage_[typeid(T)].emplace<T>(T{std::forward<Args>(args)...});
}

template <typename T>
T& Blackboard::Get() {
  assert(Has<T>());
  return std::any_cast<T&>(storage_.at(typeid(T)));
}

template <typename T>
const T& Blackboard::Get() const {
  assert(Has<T>());
  return std::any_cast<const T&>(storage_.at(typeid(T)));
}

template <typename T>
bool Blackboard::Has() const {
  return storage_.find(typeid(T)) != storage_.cend();
}
