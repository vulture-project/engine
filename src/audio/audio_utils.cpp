/**
 * @author Viktor Baranov (github.com/baranov-V-V)
 * @file audio_utils.cpp
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

#include <fstream>
#include <vector>
#include <cstdio>
#include <exception>
#include <cstdio>

#include "audio/audio_utils.hpp"
#include "core/logger.hpp"

#include <vorbis/vorbisfile.h>
#include <vorbis/codec.h>

namespace vulture {

RawAudioData ParseWav(const char* filename) {
	RawAudioData raw_data;

	std::FILE* audio_file = fopen(filename, "rb");
	if (audio_file == nullptr) {
		fclose(audio_file);
		throw std::runtime_error("Can not open audio file " + std::string(filename));
	}

	WavHeader wav_header = {};
	size_t read_num = std::fread(&wav_header, 1, sizeof(WavHeader), audio_file);
	if (read_num != sizeof(WavHeader)) {
		fclose(audio_file);
		throw std::runtime_error("Can not read Wave header from file " + std::string(filename));
	}
	
	//todo check for curruptions
	raw_data.bits_per_sample = wav_header.bits_per_sample;
	raw_data.channels_count_ = wav_header.channels_count;
	raw_data.sample_rate_	 	 = wav_header.sample_rate;
	raw_data.data_size_ 	   = wav_header.subchunk2_size;

	LOG_DEBUG(AudioUtils, "raw_data size: {}", raw_data.data_size_);

	raw_data.data_ 				   = operator new(raw_data.data_size_);

	size_t bytes_read = std::fread(raw_data.data_, 1, raw_data.data_size_, audio_file);
	if (bytes_read != raw_data.data_size_) {
		fclose(audio_file);
		throw std::runtime_error("Can not read enough bytes from audio file " + std::string(filename));
	}

	return raw_data;
}

RawAudioData ParseOgg(const char* filename) {
	RawAudioData raw_data;

	std::FILE* audio_file = fopen(filename, "rb");
	if (audio_file == nullptr) {
		throw std::runtime_error("Can not open audio file " + std::string(filename));
	}

	OggVorbis_File vf = {};

	if(ov_open_callbacks(audio_file, &vf, NULL, 0, OV_CALLBACKS_NOCLOSE) < 0) {
		ov_clear(&vf);
		throw std::runtime_error("File does not appear to be an Ogg bitstream");
  }
	
	vorbis_info* vi = ov_info(&vf, -1);

	ogg_int64_t seeked_size = ov_pcm_total(&vf, -1) * 2;
	if (seeked_size == OV_EINVAL) {
		ov_clear(&vf);
		throw std::runtime_error("Can not seek length of file" + std::string(filename));
	}

	raw_data.data_size_ = static_cast<size_t>(ov_pcm_total(&vf, -1) * 2); //num of samples each 16bit (2 bytes)
	raw_data.data_ = operator new(raw_data.data_size_);

	raw_data.channels_count_ = vi->channels;
	raw_data.sample_rate_ 	 = vi->rate;
	raw_data.bits_per_sample = vi->bitrate_nominal; //mb wrong
	
	std::vector<Chunk> chunked_audio;

	//global constant
	const size_t decode_chunk_capacity = 4096;
	size_t decoded_data_size = 0;

	bool eof = false;
  int current_section = 0;
		
	while(!eof) {
		Chunk chunk(decode_chunk_capacity);

		long read_count = ov_read(&vf, static_cast<char*>(chunk.data), decode_chunk_capacity, 0, 2, 1, &current_section);
		
		if (read_count == 0) {
			eof = true;
		} else if (read_count < 0) {
			ov_clear(&vf);
			throw std::runtime_error("Corrupted bitstream section in ogg file: " + std::string(filename));
		} else {
			chunk.size = read_count;
			decoded_data_size += read_count;
			chunked_audio.push_back(std::move(chunk));
		}

  }
	
  char* curr_fill_pos = static_cast<char*>(raw_data.data_);

	for (size_t i = 0; i < chunked_audio.size(); ++i) {
		std::memcpy(curr_fill_pos, chunked_audio[i].data, chunked_audio[i].size);	
    curr_fill_pos += chunked_audio[i].size;
	}

	ov_clear(&vf);

	return raw_data;
}

} // namespace vulture