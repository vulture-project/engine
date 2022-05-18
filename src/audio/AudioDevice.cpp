#include "audio/AudioDevice.h"
#include "iostream"

namespace sound {
	AudioDevice::AudioDevice() : device_(nullptr), context_count_(0) {}

	AudioDevice::~AudioDevice() {}

	void AudioDevice::Open(const char* device_name) {
		device_ = alcOpenDevice(device_name); // nullptr = get default device
		
		//if (!p_ALCDevice)
		//	throw("failed to get sound device");

		const ALCchar* name = nullptr;
		if (alcIsExtensionPresent(device_, "ALC_ENUMERATE_ALL_EXT")) {
			name = alcGetString(device_, ALC_ALL_DEVICES_SPECIFIER);
		}
		if (!name || alcGetError(device_) != AL_NO_ERROR) {
			name = alcGetString(device_, ALC_DEVICE_SPECIFIER);
		}

		std::cout << "Opened [" << name << "]\n";
	}

	void AudioDevice::Close() {
		if (context_count_ != 0) {
			const char* device_name = alcGetString(device_, ALC_DEVICE_SPECIFIER);
			//LOG_ERROR(audio_device, "At Close Device: [%a] context_count is not 0: [%a]", device_name, context_count_);
			assert(0);
		}
		alcCloseDevice(device_);
	}

	AudioContext* AudioDevice::CreateContext() {
		AudioContext* audio_context = new AudioContext();
		audio_context->Create(device_);
		++context_count_;
		return audio_context;
	}

	void AudioDevice::DestroyContext(AudioContext* audio_context) {
		audio_context->Destroy();
		--context_count_;
		delete audio_context;
	}

}