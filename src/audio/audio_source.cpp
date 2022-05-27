/**
 * @author Viktor Baranov (github.com/baranov-V-V)
 * @file audio_source.cpp
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

#include "audio/audio_source.hpp"

#include "core/logger.hpp"

namespace vulture {

AudioSource::AudioSource() : handle_count_(0) {
	alGenSources(1, &al_source_);
	LOG_INFO(AudioSource, "source_id generated: {}", al_source_);
}

AudioSource::~AudioSource() {
	alSourcei(al_source_, AL_BUFFER, 0);
	alDeleteSources(1, &al_source_);
}

//need to cover this into friend functions
void AudioSource::Handle::SetBuf(AudioBuffer::Handle handle) {
	audio_source_->buffer_ = handle.GetBuffer();
	LOG_DEBUG(AudioSource, "buf no. {}", audio_source_->buffer_->al_buffer_handle_);
	alSourcei(audio_source_->al_source_, AL_BUFFER, audio_source_->buffer_->al_buffer_handle_);
}

AudioSource::Handle AudioSource::GetHandle() {
	return Handle(this);
}

void AudioSource::Handle::Play() {
	//LOG_DEBUG(audio_source, "started playing. {}", audio_source_->al_source_);
	//LOG_DEBUG(audio_source, "with buf no. {}", audio_source_->buffer_->al_buffer_handle_);
	alSourcePlay(audio_source_->al_source_);
}

void AudioSource::Handle::Pause() {
	alSourcePause(audio_source_->al_source_);
}

void AudioSource::Handle::Stop() {
	alSourceStop(audio_source_->al_source_);
}

void AudioSource::Handle::Resume() {
	if (!IsPlaying()) {
		alSourcePlay(audio_source_->al_source_);
	}
}

bool AudioSource::Handle::IsPlaying() {
	ALint playState;
	alGetSourcei(audio_source_->al_source_, AL_SOURCE_STATE, &playState);
	return (playState == AL_PLAYING);
}

bool AudioSource::Handle::IsLooping() {
	ALint is_loop;
	alGetSourcei(audio_source_->al_source_, AL_LOOPING, &is_loop);
	return is_loop;
}

void AudioSource::Handle::SetVolume(float volume) {
	alSourcef(audio_source_->al_source_, AL_GAIN, volume);
}

void AudioSource::Handle::SetLooping(bool is_loop) {
	alSourcei(audio_source_->al_source_, AL_LOOPING, (ALint)is_loop);
}

float AudioSource::Handle::GetVolume() const {
	ALfloat volume = 0;
	alGetSourcef(audio_source_->al_source_, AL_GAIN, &volume);
	return volume;
}

float AudioSource::Handle::GetProgress() {
	float seconds_passed = 0;
	float seconds_total = 0;
	alGetSourcef(audio_source_->al_source_, AL_SEC_OFFSET, &seconds_passed);

	ALint size_in_bytes;
	ALint channels;
	ALint bits;

	alGetBufferi(audio_source_->buffer_->al_buffer_handle_, AL_SIZE, &size_in_bytes);
	alGetBufferi(audio_source_->buffer_->al_buffer_handle_, AL_CHANNELS, &channels);
	alGetBufferi(audio_source_->buffer_->al_buffer_handle_, AL_BITS, &bits);

	ALint length_in_samples = size_in_bytes * 8 / (channels * bits);

	ALint frequency;

	alGetBufferi(audio_source_->buffer_->al_buffer_handle_, AL_FREQUENCY, &frequency);

	seconds_total = (float)length_in_samples / (float)frequency;

	return seconds_passed / seconds_total;
}

} // namespace vulture
