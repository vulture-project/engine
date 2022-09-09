/**
 * @author Viktor Baranov (github.com/baranov-V-V)
 * @file source_pool.hpp
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

#include <list>
#include <unordered_map>
#include <map>
#include <optional>
#include <queue>
#include <string>

#include "audio/audio_source.hpp"

//owner of all resources
//makes its own context
namespace vulture {

class AudioContext;

class SourcePool {
 public:
  friend class AudioSource;

  using SourceStorage = std::map<std::string, AudioSource*>;

  SourcePool() = default;
  ~SourcePool();

  SourcePool(const SourcePool&) = delete;
  SourcePool(SourcePool&& pool);
	
	bool CreateSource(AudioContext* context, const char* name);
	std::optional<AudioSource::Handle> GetSource(const char* name);

	bool Destroy(const char* name);
  
  class Iterator {
	 private:
	 	using SourceStorageIterator = SourceStorage::iterator;

	 public:
    using value_type = AudioSource::Handle;
		using pointer = AudioSource::Handle*;

    Iterator(SourceStorageIterator iterator) : iterator_(iterator) {};
    
    value_type operator*() {
      return iterator_->second->GetHandle();
    }
    const value_type operator*() const {
      return iterator_->second->GetHandle();;
    }

    Iterator& operator++() {
        ++iterator_;
        return *this;
    }
    Iterator operator++(int) {
        Iterator tmp(*this);
        ++iterator_;
        return tmp;
    }

    bool operator!=(const Iterator &rhs) const {
        return iterator_ != rhs.iterator_;
    }
    bool operator==(const Iterator &rhs) const {
        return iterator_ == rhs.iterator_;
    }

   private:
		SourceStorageIterator iterator_;
	};

	Iterator Begin() {
		return Iterator(sources_.begin());
	}

	Iterator End() {
		return Iterator(sources_.end());
	}

 private:
  SourceStorage sources_;

  void ClearStorage(SourceStorage& storage);
};

} // namespace vulture
