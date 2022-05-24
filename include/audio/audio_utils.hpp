/**
 * @author Viktor Baranov (github.com/baranov-V-V)
 * @file audio_utils.hpp
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

#include "audio/audio_buffer.hpp"

namespace vulture {

struct Chunk {
	Chunk(size_t capacity) : capacity(capacity), size(0) {
		data = operator new(capacity);
	}
	
	~Chunk() {
		operator delete(data);
	}

	Chunk(const Chunk&) = delete;
	Chunk(Chunk&& chunk) noexcept : data(chunk.data), capacity(chunk.capacity), size(chunk.size) {
		chunk.data = nullptr;
		chunk.capacity = 0;
		chunk.size = 0;
	};

	void* data;
	size_t capacity;
	size_t size;
};

struct WavHeader {
    char chunk_id[4];
    uint32_t chunk_size;
    char format[4];

    char subchunk1_id[4];
    uint32_t subchunk1_size;
    uint16_t audio_format;
    uint16_t channels_count;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;

    char subchunk2_id[4];
    uint32_t subchunk2_size;
};

RawAudioData ParseOgg(const char* filename);
RawAudioData ParseWav(const char* filename);

} // namespace vulture