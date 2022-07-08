/**
 * @author Viktor Baranov (github.com/baranov-V-V)
 * @file audio_listener.hpp
 * @date 2022-05-23
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

#include <glm/vec3.hpp>

using Vec3f = glm::vec3;

namespace vulture {

class AudioContext;

class AudioListener {
 public:

  AudioListener(AudioContext* context);

  AudioListener(const AudioListener&) = delete;
  AudioListener(AudioListener&& listener);

  bool IsCurrent();
  void MakeCurrent();

  void GetLocation(Vec3f& loc);
  void GetOrientation(Vec3f& at, Vec3f& up);
  float GetVolume();

  void SetLocation(const Vec3f& location);
  void SetOrientation(const Vec3f& at, const Vec3f& up);
  void SetVolume(const float& val);

private:	
  AudioContext* context_;
  Vec3f pos_;
  Vec3f at_;
  Vec3f up_;
  float volume_;
  bool is_current_;
};

} // namespace vulture