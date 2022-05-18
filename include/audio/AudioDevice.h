#pragma once

#include <set>

#include <AL/alc.h>
#include <AL/al.h>
#include "audio/AudioContext.h"

namespace sound {

class AudioDevice {
public:
	friend class AudioContext;

	AudioDevice();
	~AudioDevice();

	void Open(const char* device_name = nullptr);
	void Close();

	AudioContext* CreateContext();
	void DestroyContext(AudioContext*);

private:
	ALCdevice* device_;
	int context_count_;
};

} //namespace sound
