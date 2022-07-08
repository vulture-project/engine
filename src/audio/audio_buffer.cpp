/**
 * @author Viktor Baranov (github.com/baranov-V-V)
 * @file audio_buffer.cpp
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

#include <climits>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cstring>
#include <exception>

#include <AL/alext.h>
#include <AL/al.h>

#include "audio/audio_utils.hpp"
#include "audio/audio_buffer.hpp"

#include "core/logger.hpp"

namespace vulture {

AudioBuffer::AudioBuffer(RawAudioData&& raw_data) : al_buffer_handle_(0), audio_data_(raw_data.data_) {
	if (alcGetCurrentContext() == nullptr) {
		throw std::runtime_error("Can not generate buffer if no current context is set");
	}
	
	alGenBuffers(1, &al_buffer_handle_);
	
	ALenum error = alGetError();
	if (error != AL_NO_ERROR) {
		LOG_ERROR(AudioBuffer, "Can not generate al_buffer, error: {0:#X}", error);
		LOG_ERROR(AudioBuffer, "Maybe if error is 0xA004 there is no current context to generate buffers in");
		throw std::runtime_error("Can not generate buffer");
	}

	ALenum format = AL_NONE;
	if (raw_data.channels_count_ == 1) {
		format = AL_FORMAT_MONO16;
	}
	else if (raw_data.channels_count_ == 2) {
		format = AL_FORMAT_STEREO16;
	}
	else if (raw_data.channels_count_ == 3) {
		format = AL_FORMAT_BFORMAT2D_16;
	}
	else if (raw_data.channels_count_ == 4) {
		format = AL_FORMAT_BFORMAT3D_16;
	} else {
		LOG_ERROR(AudioBuffer, "Wrong hannels count in raw_data: {}", raw_data.channels_count_);
	}

	LOG_INFO(AudioBuffer, "Buffer {} channels {}", al_buffer_handle_, raw_data.channels_count_);

	alGetError();
	alBufferData(al_buffer_handle_, format, raw_data.data_, raw_data.data_size_, raw_data.sample_rate_);

	error = alGetError();
	if (error != AL_NO_ERROR) {
		LOG_ERROR(AudioBuffer, "Can not fill buffer with data, error: {0:#X}", error);
		throw std::runtime_error("Can not fill buffer");
	}

	audio_data_ = raw_data.data_;
	raw_data.data_ = nullptr;
}

AudioBuffer::AudioBuffer(AudioBuffer&& buffer) {
	audio_data_ = buffer.audio_data_;
	al_buffer_handle_ = buffer.al_buffer_handle_;
	
	buffer.al_buffer_handle_ = 0;
	buffer.audio_data_ = nullptr;
}

AudioBuffer::~AudioBuffer() {
	if (al_buffer_handle_ != 0) {
		if (handle_count_ != 0) {
			LOG_ERROR(AudioBuffer, "Deleting buffer with al_handle : {} but handle_count is not 0: {}", handle_count_);
		}

		operator delete(audio_data_);
		alDeleteBuffers(1, &al_buffer_handle_);
	}
}

} // namespace vulture
