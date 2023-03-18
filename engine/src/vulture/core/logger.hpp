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

#include <fmt/color.h>
#include <fmt/core.h>

#include <fstream>
#include <string>
#include <vulture/core/time.hpp>

#define LINE_TO_STR(x) TO_STR(x)
#define TO_STR(x) #x

#define LOG_INFO(module, ...)  Logger::Log(#module, __FILE__ ":" LINE_TO_STR(__LINE__), Logger::kInfo, false, __VA_ARGS__)
#define LOG_WARN(module, ...)  Logger::Log(#module, __FILE__ ":" LINE_TO_STR(__LINE__), Logger::kWarn, false, __VA_ARGS__)
#define LOG_ERROR(module, ...) Logger::Log(#module, __FILE__ ":" LINE_TO_STR(__LINE__), Logger::kError, true, __VA_ARGS__)
#define LOG_DEBUG(module, ...) Logger::Log(#module, __FILE__ ":" LINE_TO_STR(__LINE__), Logger::kDebug, true, __VA_ARGS__)

class Logger {
 public:
  enum LoggingLevel {
    kInfo,
    kWarn,
    kError,
    kDebug
  };

  static const fmt::text_style kInfoStyle;
  static const fmt::text_style kWarnStyle;
  static const fmt::text_style kErrorStyle;
  static const fmt::text_style kDebugStyle;

  static const char* kDefaultLogFileName;
  static const char* kProjectDirectoryName;

  static const int kFilenameAlignment;
  static const int kLevelStringAlignment;

 public:
  static void OpenLogFile(const char* filename = kDefaultLogFileName);
  static void Flush();
  static void Close();

  template <typename... Args>
  static void Log(const char* module, std::string place, LoggingLevel level, bool flush, Args&&... args) {
    size_t project_start = place.find("engine");
    if (project_start == place.npos) {
      project_start = 0;
    }

    fmt::print(log_file_, "[{}] [{}] ", GetCurrentTime(), module);
    fmt::print(log_file_, LevelToTextStyle(level), "{:<{}}", LevelToString(level), kLevelStringAlignment);
    fmt::print(log_file_, "{:<{}} ", place.substr(project_start), kFilenameAlignment);

    fmt::print(log_file_, LevelToTextStyle(level), std::forward<Args>(args)...);
    fmt::print(log_file_, "\n");

    if (flush) {
      Flush();
    }
  }

  static const char* LevelToString(LoggingLevel level);
  static fmt::text_style LevelToTextStyle(LoggingLevel level);

 private:
  static FILE* log_file_;
};

/**
 * Custom-type logging.
 */
#include "glm/glm.hpp"

template <>
struct fmt::formatter<glm::vec3> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const glm::vec3& vector, FormatContext& ctx) {
    return fmt::format_to(ctx.out(), "(x={}, y={}, z={})", vector.x, vector.y, vector.z);
  }
};