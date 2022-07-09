/**
 * @author Viktor Baranov (github.com/baranov-V-V)
 * @file audio_source.hpp
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

#include <AL/al.h>

#include <glm/vec3.hpp>

#include "audio/audio_buffer.hpp"

namespace vulture {

class AudioContext;

class AudioSource {
 public:
  friend class AudioBuffer;

  AudioSource(AudioContext* context);
  ~AudioSource();

  class Handle {
   public:
    friend class AudioSource;

    Handle(AudioSource* source) : audio_source_(source) { ++audio_source_->handle_count_; };

    ~Handle() {
      if (audio_source_ != nullptr) {
        --audio_source_->handle_count_;
      }
    };

    Handle(const Handle& handle) : audio_source_(handle.audio_source_) { ++audio_source_->handle_count_; };

    Handle(Handle&& handle) : audio_source_(handle.audio_source_) { handle.audio_source_ = nullptr; };

    void SetBuf(AudioBuffer::Handle handle);

    void Play();
    void Stop();
    void Pause();
    void Resume();

    bool IsPlaying();
    bool IsLooping();

    void SetLocation(const glm::vec3& loc);
    void SetDirection(const glm::vec3& dir);
    void SetVelocity(const glm::vec3& vel);

    void SetVolume(float volume);
    void SetLooping(bool is_loop);

    float GetVolume() const;
    float GetProgress();

   private:
    AudioSource* audio_source_;

    void SetProperty(ALenum property, const glm::vec3& argument);
  };

  Handle GetHandle();

 public:
  ALuint al_source_;
  AudioBuffer* buffer_;

  size_t handle_count_;

  AudioContext* belonged_context_;
};

}  // namespace vulture
