/**
 * @author Viktor Baranov (github.com/baranov-V-V)
 * @file api.hpp
 * @date 2022-05-19
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

#include <filesystem>

#include <vulture/audio/audio_buffer.hpp>
#include <vulture/audio/audio_source.hpp>
#include <vulture/audio/buffer_manager.hpp>
#include <vulture/core/logger.hpp>

namespace vulture {

void LoadFromFolder(BufferManager* manager, const char* folder_path) {
  std::filesystem::path folder(folder_path);
  for (auto const& dir_entry : std::filesystem::directory_iterator(folder)) {
    std::filesystem::path curr_path(dir_entry);
    if (manager->LoadAudioFile(curr_path.c_str())) {
      LOG_ERROR(audio_loader, "Could Load file {} into BufferManager", curr_path.c_str());
    } else {
      LOG_INFO(audio_loader, "Loaded file {} into BufferManager", curr_path.c_str());
    }
  }
}

}  // namespace vulture