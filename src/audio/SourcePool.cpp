#include "audio/SourcePool.h"

#include <cassert>

namespace sound {

	SourcePool::SourcePool(size_t source_count) {
		Reserve(source_count);
	}

	SourcePool::~SourcePool() {

		if (!source_taken_.empty()) {
			//LOG_ERROR(SourcePool, "On Destruction SourcePool source_taken_ is not empty");
			ClearStorage(source_taken_);
			assert(0);
		}

		ClearStorage(source_free_);
	}

	void SourcePool::Reserve(size_t source_count) {
		for (size_t i = 0; i < source_count; ++i) {
			Source* source = new Source();
			source_free_.insert(source);
		}
	}

	Source* SourcePool::Acquire() {
		if (source_free_.empty()) {
			return nullptr;
		}

		Source* source = *source_free_.begin();
		source_free_.erase(source_free_.begin());
		source_taken_.insert(source);

		assert(source != nullptr);
		//LOG_INFO(source, "source_id: %a", source->source_);

		return source;
	}

	void SourcePool::Release(Source* source) {
		source_taken_.erase(source);
		source_free_.insert(source);
	}

	void SourcePool::StopAllSounds() {
		for (Source* source : source_taken_) {
			
		}
	}

	void SourcePool::PauseAllSounds() {
		for (Source* source : source_taken_) {

		}
	}

	void SourcePool::ResumeAllSounds() {
		for (Source* source : source_taken_) {

		}
	}

	void SourcePool::ClearStorage(SourceStorage& storage) {
		std::list<Source*> holder;
		for (auto it: storage) {
			holder.push_back(it);
		}
		storage.clear();
		while (!holder.empty()) {
			Source* source = holder.front();
			holder.pop_front();
			delete source;
		}
	}

} //sound