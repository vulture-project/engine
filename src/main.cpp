/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file main.cpp
 * @date 2022-04-26
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

#include "sandbox/sandbox_app.hpp"

#include "audio/AudioDevice.h"
#include "audio/AudioSource.h"

#include <stdio.h>
#include <cassert>
#include <filesystem>
#include <iostream>

#include <unistd.h>

using namespace sound;
namespace fs = std::filesystem;

int main() {
  	SandboxApp app{};
  	app.Init();
  	app.Run();
	//ALCdevice* device_ = alcOpenDevice(nullptr);
	//ALCcontext* context_ = alcCreateContext(device_, nullptr);
	//alcMakeContextCurrent(context_);
	//ALuint source_;
	//alGenSources(1, &source_);
	//std::cout << "hello audio!\n";

	//AudioDevice device;
	//device.Open();
	//AudioContext* context = device.CreateContext();
	//context->MakeCurrentPlaying();
	//sound::Source* source = context->CreateSource();
	
	//FILE* f = fopen("5.ogg", "rb");
	//assert(f != nullptr);

	//std::cout << "Current path is " << fs::current_path() << '\n';
	//AudioBuffer* buffer = new AudioBuffer("res/test.wav");
	//sound::Source source;
	//source->SetBuf(buffer);
	//source->Play();
	//alSourcei(source_, AL_BUFFER, buffer->buffer_handle_);
	//alSourcePlay(source_);
	
	//source->SetBuf(buffer);
	//source->SetVolume(1);
	//context->SetVolume(1);
	//context->SetLocation({ 0, 0 ,0 });
	//source->Play();
	
	//sleep(3);

	//sleep(3000);
	//source->ReleaseBuf();
	//alSourcei(source_, AL_BUFFER, 0);
	//delete buffer;
	
	//alDeleteSources(1, &source_);
	//alcDestroyContext(context_);
	//alcCloseDevice(device_);

	//context->DestroySource(source);
	//device.DestroyContext(context);
	//device.Close();

  return 0;
}
