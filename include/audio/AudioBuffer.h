#pragma once

#include <AL/al.h>
#include <AL/alc.h>

class Source;

namespace sound {

	class AudioBuffer {
	public:
		friend class Source;

		AudioBuffer(const char* filename);
		virtual ~AudioBuffer();
	
	public:
		ALuint buffer_handle_;
	};

} //namespace sound