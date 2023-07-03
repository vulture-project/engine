/**
 * @author Sergey Zelenkin (https://github.com/vssense)
 * @file time.cpp
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

#include <vulture/core/logger.hpp>
#include <vulture/core/time.hpp>

using namespace vulture;

ScopedTimer::ScopedTimer(const String& name) : name_(name) {
  LOG_TRACE_START("{}", name_);
}

ScopedTimer::~ScopedTimer() {
  LOG_TRACE_FINISH("{} - {:.{}f}ms", name_, timer_.ElapsedMs(), 2);
}

const size_t kHhMmSsStart = 11;
const size_t kHhMmSsLength = 8;

String vulture::GetCurrentTimeStr() {
  time_t rawtime = std::time(nullptr);
  String time_str = asctime(localtime(&rawtime));
  time_str.pop_back();   // remove '\n'

  return time_str.substr(kHhMmSsStart, kHhMmSsLength);
}
