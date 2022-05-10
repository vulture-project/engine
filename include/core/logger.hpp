/**
 * @author Sergey Zelenkin (https://github.com/vssense)
 * @file logger.hpp
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

#include <fstream>
#include <string>

#include "core/time.hpp"
#include "fmt/core.h"

#define LINE_TO_STR(x) TO_STR(x)
#define TO_STR(x) #x

#define LOG_INFO(module, ...)  Logger::Log(#module, __FILE__ ":" LINE_TO_STR(__LINE__), "info", false, __VA_ARGS__)
#define LOG_WARN(module, ...)  Logger::Log(#module, __FILE__ ":" LINE_TO_STR(__LINE__), "warn", false, __VA_ARGS__)
#define LOG_ERROR(module, ...) Logger::Log(#module, __FILE__ ":" LINE_TO_STR(__LINE__), "error", true, __VA_ARGS__)
#define LOG_DEBUG(module, ...) Logger::Log(#module, __FILE__ ":" LINE_TO_STR(__LINE__), "debug", true, __VA_ARGS__)

class Logger {
 public:
  static void OpenLogFile(const char* filename = kDefaultLogFileName);
  static void Flush();
  static void Close();

  template <typename... Args>
  static void Log(const char* module, std::string place, const char* level,
                  bool flush, Args&&... args) {
    size_t project_start = place.find("engine");
    if (project_start == place.npos) {
      project_start = 0;
    }

    fmt::print(log_file_, "[{}] [{}] [{}] {:<{}} ", GetCurrentTime(), module, level,
               place.substr(project_start), kFilenameAlignment);
    fmt::print(log_file_, std::forward<Args>(args)...);
    fmt::print(log_file_, "\n");

    if (flush) {
      Flush();
    }
  }

  constexpr static const char* kDefaultLogFileName = "log/log.txt"; 
  constexpr static const char* kProjectDirectoryName = "engine"; 

  constexpr static const int kFilenameAlignment = 40;

 private:
  static FILE* log_file_;
};
