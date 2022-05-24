/**
 * @author Viktor Baranov (github.com/baranov-V-V)
 * @file source_pool.cpp
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

#include <cassert>
#include <utility>

#include "audio/source_pool.hpp"

#include "core/logger.hpp"

namespace vulture {

SourcePool::~SourcePool() {
	ClearStorage(sources_);
}

SourcePool::SourcePool(SourcePool&& pool) :
	sources_(std::move(pool.sources_)) {
}

bool SourcePool::CreateSource(const char* name) {
	std::string source_name(name);

	AudioSource* source = new AudioSource();
	auto result_it = sources_.emplace(std::make_pair(std::move(source_name), source));
	
	if (result_it.second == false) {
		delete source;
		LOG_DEBUG(source_pool, "Could not create insert source in pool with name {}", name);
		return false;
	}

	return true;
}

std::optional<AudioSource::Handle> SourcePool::GetSource(const char* name) {
	auto it = sources_.find(std::string(name));

	if (it == sources_.end()) {
		return std::nullopt;
	}

	return std::optional<AudioSource::Handle>(it->second);	
}

bool SourcePool::Destroy(const char* name) {
	size_t count = sources_.erase(std::string(name));
	return count != 0;
}

void SourcePool::ClearStorage(SourceStorage& storage) {
	while (!storage.empty()) {
		AudioSource* source = storage.begin()->second;
		storage.erase(storage.begin());
		delete source;
	}
}

} // namespace vulture