/**
 * @author Viktor Baranov (github.com/baranov-V-V)
 * @file audio_buffer.hpp
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

#include <AL/al.h>
#include <AL/alc.h>

#include <cstddef>

class AudioSource;

namespace vulture {

struct RawAudioData {

	RawAudioData() = default;
	
	RawAudioData(size_t alloc_size) : data_size_(alloc_size) {
		data_ = operator new(data_size_);	
	};

	~RawAudioData() {
		operator delete(data_);
	};

	RawAudioData(const RawAudioData&) = delete;
	
	RawAudioData(RawAudioData&& raw_data) {
		data_ = raw_data.data_;
		channels_count_ = raw_data.channels_count_;
		sample_rate_ = raw_data.sample_rate_;
		bits_per_sample = raw_data.bits_per_sample;
		data_size_ = raw_data.data_size_;

		raw_data.data_ = nullptr;
		raw_data.data_size_ = 0;
	};

	void ReplaceMove(RawAudioData&& raw_data) {
		data_ = raw_data.data_;
		channels_count_ = raw_data.channels_count_;
		sample_rate_ = raw_data.sample_rate_;
		bits_per_sample = raw_data.bits_per_sample;
		data_size_ = raw_data.data_size_;

		raw_data.data_ = nullptr;
		raw_data.data_size_ = 0;
	};

	size_t channels_count_;
	size_t sample_rate_;
	size_t bits_per_sample;
	size_t data_size_;
	void* data_;
};


//----------------------------------------TODO---------------------------------------
//rewrite buffer class:
//1)add loading from file + parsing
//2)


class AudioBuffer {
 public:
	friend class AudioSource;
	
	AudioBuffer(RawAudioData&& raw_data);
	~AudioBuffer();

	AudioBuffer(const AudioBuffer&) = delete;
	AudioBuffer(AudioBuffer&& buffer);
	
	class Handle {
	 public:
		friend class AudioBuffer;

		Handle(AudioBuffer* buffer) : buffer_(buffer) {
			++buffer_->handle_count_;
		};

		~Handle() {
			if (buffer_ != nullptr) {
				--buffer_->handle_count_;
			}
		}

		Handle(const Handle& handle) : buffer_(handle.buffer_) {
			++buffer_->handle_count_;
		};

		Handle(Handle&& handle) : buffer_(handle.buffer_) {
      handle.buffer_ = nullptr;
    };

		
		//mb skip this func? yeah need to skip
		AudioBuffer* GetBuffer() {
			return buffer_;
		}

	 private:
		AudioBuffer* buffer_;
	};

	Handle GetHandle() {
		return Handle(this);
	}

 public:
	ALuint al_buffer_handle_;
	void* audio_data_;

	size_t handle_count_{0};
};

} // namespace vulture