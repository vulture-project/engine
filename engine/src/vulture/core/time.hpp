/**
 * @author Sergey Zelenkin (https://github.com/vssense)
 * @file time.hpp
 * @date 2022-05-08
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

#include <chrono>
#include <string>
#include <vulture/core/types.hpp>

namespace vulture {

class Timer {
 public:
  Timer() { Reset(); }

  inline void Reset() { start_ = std::chrono::high_resolution_clock::now(); }

  inline float Elapsed() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start_)
               .count() * 0.001f * 0.001f * 0.001f;
  }

  inline float ElapsedMs() { return Elapsed() * 1000.0f; }

 private:
  std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

class ScopedTimer {
 public:
  ScopedTimer(const String& name);
  ~ScopedTimer();

 private:
  String name_;
  Timer  timer_;
};

/**
 * @brief Get current time in format hh:mm:ss
 * 
 * @return String current time
 */
String GetCurrentTimeStr();

}  // namespace vulture
