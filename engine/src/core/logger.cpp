/**
 * @author Sergey Zelenkin (https://github.com/vssense)
 * @file logger.cpp
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

#include "core/logger.hpp"
#include <cassert>

FILE* Logger::log_file_ = stdout;

// Constants
const fmt::text_style Logger::kInfoStyle  = fmt::emphasis::faint;
const fmt::text_style Logger::kWarnStyle  = fmt::emphasis::bold | fg(fmt::color::purple);
const fmt::text_style Logger::kErrorStyle = fmt::emphasis::bold | fg(fmt::color::red);
const fmt::text_style Logger::kDebugStyle = fmt::emphasis::bold;

const char* Logger::kDefaultLogFileName = "log/log.txt"; 
const char* Logger::kProjectDirectoryName = "engine"; 

const int Logger::kFilenameAlignment = 0;
const int Logger::kLevelStringAlignment = 7;

void Logger::OpenLogFile(const char* filename) {
  log_file_ = fopen(filename, "a");

  if (log_file_ == nullptr) {
    printf("Can't open log file: %s\n", filename);
    fflush(stdout);

    assert(!"Can't open log_file");
  }
}

void Logger::Flush() {
  fflush(log_file_);
}

void Logger::Close() {
  if (log_file_ != stdout) {
    fclose(log_file_);
    log_file_ = stdout;
  }
}

const char* Logger::LevelToString(LoggingLevel level) {
  switch (level) {
    case kInfo:  { return "[info]"; }
    case kWarn:  { return "[warn]"; }
    case kError: { return "[error]"; }
    case kDebug: { return "[debug]"; }
    default:     { return "unknown"; }
  }
}

fmt::text_style Logger::LevelToTextStyle(LoggingLevel level) {
  switch (level) {
    case kInfo:  { return kInfoStyle; }
    case kWarn:  { return kWarnStyle; }
    case kError: { return kErrorStyle; }
    case kDebug: { return kDebugStyle; }
    default:     { return fmt::emphasis::italic | fg(fmt::color::purple); }
  }
}
