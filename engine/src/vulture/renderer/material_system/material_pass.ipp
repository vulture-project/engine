/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file material_pass.ipp
 * @date 2023-03-20
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

template<typename T>
T& MaterialPass::GetProperty(const StringView name) {
  T* result = nullptr;

  for (uint32_t i = 0; i < property_buffers_count_; ++i) {
    PropertyBuffer& property_buffer = property_buffers_[i];
    bool found = false;

    for (const auto& member : property_buffer.reflected_uniform_buffer->members) {
      if (member.name == name) {
        VULTURE_ASSERT(sizeof(T) == member.size, "Invalid property size (name = {0})!", name);
        result = reinterpret_cast<T*>(property_buffer.buffer + member.offset);
        found = true;
        break;
      }
    }

    if (found) {
      break;
    }
  }

  VULTURE_ASSERT(result, "No property with this name found!");

  return *result;
}
