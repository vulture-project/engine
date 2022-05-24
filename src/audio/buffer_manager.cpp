/**
 * @author Viktor Baranov (github.com/baranov-V-V)
 * @file buffer_manager.cpp
 * @date 2022-05-21
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

#include <filesystem>
#include <utility>
#include <exception>

#include "audio/buffer_manager.hpp"
#include "audio/audio_utils.hpp"
#include "core/logger.hpp"

namespace vulture {
    
BufferManager::~BufferManager() {
	while (!buffers_.empty()) {
		AudioBuffer* buffer = buffers_.begin()->second;
		buffers_.erase(buffers_.begin());
		delete buffer;
	}
}

bool BufferManager::LoadAudioFile(const char* path, const char* name) {
	std::filesystem::path file_path(path);
	
	RawAudioData raw_data;
	
	try {
		if (file_path.extension() == ".wav") {
			raw_data.ReplaceMove(std::move(ParseWav(path)));
		} else if (file_path.extension() == ".ogg") {
			raw_data.ReplaceMove(std::move(ParseOgg(path)));
		}
	} catch(std::runtime_error& e) {
		LOG_ERROR(buffer_manager, "Could not load sound {} into raw_data, caught exeption: {}", path, e.what());
	}
	
	std::string buf_name;
	if (name == nullptr) {
		buf_name = file_path.stem();
	} else {
		buf_name = name;
	}
	
	AudioBuffer* buffer = nullptr;
	
	try {
		buffer = new AudioBuffer(std::move(raw_data));
	} catch(std::runtime_error& e) {
		LOG_ERROR(buffer_manager, "{}", e.what());
		return false;
	}

	auto result_it = buffers_.emplace(std::make_pair(std::move(buf_name), buffer));

	if (result_it.second == false) {
		delete buffer;
		LOG_ERROR(buffer_manager, "Could not insert buffer {} in storage", buf_name);
		return false;
	}

	return true;
}

std::optional<AudioBuffer::Handle> BufferManager::GetBuffer(const char* name) {
	auto it = buffers_.find(std::string(name));

	if (it == buffers_.end()) {
		return std::nullopt;
	}

	return std::optional<AudioBuffer::Handle>(it->second);
}

bool BufferManager::DeleteBuffer(const char* name) {
	size_t count = buffers_.erase(std::string(name));
	return count != 0;
}

} // namespace vulture
