#include "audio/AudioSource.h"


namespace sound {

	Source::Source() {
		alGenSources(1, &source_);
		//LOG_INFO(source, "source_id: %a", source_);
	}

	Source::Source(ALuint source_handle) {
		source_ = source_handle;
	}

	Source::~Source() {
		alDeleteSources(1, &source_);
	}

	void Source::SetBuf(AudioBuffer* buffer) {
		buffer_ = buffer;
		alSourcei(source_, AL_BUFFER, buffer_->buffer_handle_);
	}

	void Source::ReleaseBuf() {
		alSourcei(source_, AL_BUFFER, 0);
	}

	AudioBuffer* Source::GetBuf() {
		return buffer_;
	}

	void Source::Play() {
		alSourcePlay(source_);
	}

	void Source::Pause() {
		alSourcePause(source_);
	}

	void Source::Stop() {
		alSourceStop(source_);
	}

	void Source::Resume() {
		if (!IsPlaying()) {
			alSourcePlay(source_);
		}
	}

	bool Source::IsPlaying() {
		ALint playState;
		alGetSourcei(source_, AL_SOURCE_STATE, &playState);
		return (playState == AL_PLAYING);
	}

	bool Source::IsLooping() {
		ALint is_loop;
		alGetSourcei(source_, AL_LOOPING, &is_loop);
		return is_loop;
	}

	void Source::SetVolume(float volume) {
		alSourcef(source_, AL_GAIN, volume);
	}

	void Source::SetLooping(bool is_loop) {
		alSourcei(source_, AL_LOOPING, (ALint)is_loop);
	}

	float Source::GetVolume() const {
		ALfloat volume = 0;
		alGetSourcef(source_, AL_GAIN, &volume);
		return volume;
	}

	float Source::GetProgress() {
		float seconds_passed = 0;
		float seconds_total = 0;
		alGetSourcef(source_, AL_SEC_OFFSET, &seconds_passed);

		ALint size_in_bytes;
		ALint channels;
		ALint bits;

		alGetBufferi(buffer_->buffer_handle_, AL_SIZE, &size_in_bytes);
		alGetBufferi(buffer_->buffer_handle_, AL_CHANNELS, &channels);
		alGetBufferi(buffer_->buffer_handle_, AL_BITS, &bits);

		ALint length_in_samples = size_in_bytes * 8 / (channels * bits);

		ALint frequency;

		alGetBufferi(buffer_->buffer_handle_, AL_FREQUENCY, &frequency);

		seconds_total = (float)length_in_samples / (float)frequency;

		return seconds_passed / seconds_total;
	}

}
