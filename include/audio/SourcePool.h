#pragma once

#include <list>
#include <unordered_map>
#include <set>
#include <queue>

#include "AudioSource.h"

//owner of all resources
//makes its own context
namespace sound {

	class SourcePool {
	public:
		friend class Source;

		using SourceStorage = std::set<Source*>;

		SourcePool(size_t source_count);
		~SourcePool();

		void Reserve(size_t source_count);
		Source* Acquire();
		void Release(Source* source);

		void StopAllSounds();
		void PauseAllSounds();
		void ResumeAllSounds();

	private:
		SourceStorage source_taken_;
		SourceStorage source_free_;

		void ClearStorage(SourceStorage& storage);
	};

} //namespace sound
