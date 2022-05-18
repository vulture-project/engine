#pragma once

#include "AudioBuffer.h"
#include <AL/al.h>

namespace sound {

	//only via SourcePool
	class Source {
	public:
		friend class AudioBuffer;

		Source();
		~Source();

		//void* VGet() = 0;
		//void VGetResource() = 0;
		//bool VOnRestore() = 0;

		void SetBuf(AudioBuffer* buffer);
		void ReleaseBuf();
		AudioBuffer* GetBuf();

		void Play();
		void Stop();
		void Pause();
		void Resume();

		bool IsPlaying();
		bool IsLooping();

		void SetVolume(float volume);
		void SetLooping(bool is_loop);
		
		float GetVolume() const;
		float GetProgress();

	public:
		Source(ALuint source_handle);

		ALuint source_;
		AudioBuffer* buffer_;
	};

} //namespace audio
