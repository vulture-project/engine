/**
 * @author Viktor Baranov (github.com/baranov-V-V)
 * @file audio_context.hpp
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

#include <AL/alc.h>
#include <glm/vec3.hpp>
#include <optional>

#include "audio/source_pool.hpp"
#include "audio/audio_source.hpp"

using Vec3f = glm::vec3;

namespace vulture {

class AudioListener;
class AudioDevice;

class AudioContext {
 public:
	friend class AudioListener;

	AudioContext(AudioDevice* device);
	~AudioContext();

	AudioContext(const AudioContext&) = delete;

	AudioContext(AudioContext&& context);

	bool MakeCurrent();
	bool IsCurrent();

	AudioListener GetNewListener();

	bool CreateSource(const char* name);
	std::optional<AudioSource::Handle> GetSource(const char* name);
	
	bool DestroySource(const char* name);

	//using iterator of pool
	void StopAllSounds();
  void PauseAllSounds();
  void ResumeAllSounds();

 private:
	SourcePool pool_;
	ALCcontext* context_;

	AudioDevice* device_owner_;

	AudioListener* current_listener_;
};

} // namespace vulture
