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
#include <fmt/ostream.h>

#include <cassert>
#include <fstream>
#include <string>
#include <vulture/core/enum_str.hpp>
#include <vulture/core/time.hpp>

#define LINE_TO_STR(x) TO_STR(x)
#define TO_STR(x) #x

#define LOG_INFO(...)         vulture::Logger::Log(__FILE__ ":" LINE_TO_STR(__LINE__), vulture::LogLevel::kInfo, false, __VA_ARGS__)
#define LOG_WARN(...)         vulture::Logger::Log(__FILE__ ":" LINE_TO_STR(__LINE__), vulture::LogLevel::kWarn, false, __VA_ARGS__)
#define LOG_TRACE_START(...)  vulture::Logger::Log(__FILE__ ":" LINE_TO_STR(__LINE__), vulture::LogLevel::kTraceStart, true, __VA_ARGS__)
#define LOG_TRACE_FINISH(...) vulture::Logger::Log(__FILE__ ":" LINE_TO_STR(__LINE__), vulture::LogLevel::kTraceFinish, true, __VA_ARGS__)
#define LOG_ERROR(...)        vulture::Logger::Log(__FILE__ ":" LINE_TO_STR(__LINE__), vulture::LogLevel::kError, true, __VA_ARGS__)
#define LOG_DEBUG(...)        vulture::Logger::Log(__FILE__ ":" LINE_TO_STR(__LINE__), vulture::LogLevel::kDebug, true, __VA_ARGS__)

namespace vulture {

DECLARE_ENUM_TO_STR(LogLevel, kInfo, kWarn, kTraceStart, kTraceFinish, kError, kDebug);

class Logger {
 public:
  static const fmt::text_style kInfoStyle;
  static const fmt::text_style kWarnStyle;
  static const fmt::text_style kTraceStartStyle;
  static const fmt::text_style kTraceFinishStyle;
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
  static void SetTraceEnabled(bool enabled);

  template <typename... Args>
  static void Log(std::string place, LogLevel level, bool flush, Args&&... args) {
    if (!trace_enabled_ && (level == LogLevel::kTraceStart || level == LogLevel::kTraceFinish)) {
      return;
    }

    size_t project_start = place.find("engine");
    if (project_start == place.npos) {
      project_start = 0;
    }

    if (level == LogLevel::kTraceStart) {
      fmt::print(log_file_, "{: >{}}", "", 2 * (cur_tracer_depth_++));
    } else if (level == LogLevel::kTraceFinish) {
      assert(cur_tracer_depth_ > 0);

      fmt::print(log_file_, "{: >{}}", "", 2 * (--cur_tracer_depth_));
    }

    // std::string current_time = GetCurrentTimeStr();
    // fmt::print(log_file_, "[{}] ", current_time);
    // fmt::print(log_file_, LevelToTextStyle(level), "{:<s{}}", LogLevelToStr(level), kLevelStringAlignment);
    fmt::print(log_file_, LevelToTextStyle(level), "{} ", LevelToTextLabel(level));
    // fmt::print(log_file_, "{:<{}} ", place.substr(project_start), kFilenameAlignment);

    if (level == LogLevel::kError || level == LogLevel::kDebug) {
      fmt::print(log_file_, fmt::emphasis::underline, "{}", place.substr(project_start));
      fmt::print(log_file_, " ");
    }

    fmt::print(log_file_, LevelToTextStyle(level), std::forward<Args>(args)...);
    fmt::print(log_file_, "\n");

    if (flush) {
      Flush();
    }
  }

  static const char*     LevelToTextLabel(LogLevel level);
  static fmt::text_style LevelToTextStyle(LogLevel level);

 private:
  static FILE* log_file_;
  static uint32_t cur_tracer_depth_;
  static bool trace_enabled_;
};

}  // namespace vulture

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