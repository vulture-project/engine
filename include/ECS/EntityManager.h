#pragma once

#include <unordered_map>
#include <utility>
#include <memory>
#include <cassert>

#include "ECS/Entity.h"
#include "ECS/TypeIdGenerator.h"

namespace ecs {

	class EntityManager {
	public:
		using EntityStorage = std::unordered_map<Entity::EntityId, Entity*>;

		EntityManager() {

		};

		~EntityManager() {
			while (!entities_.empty()) {
				auto it = entities_.begin();
				delete it->second;
				entities_.erase(it->first);
			}
		};

		class Iterator {
		public:
			using value_type = Entity*;
			using difference_type = std::ptrdiff_t;
			using pointer = Entity**;
			using reference = Entity*&;
			using iterator_category = std::forward_iterator_tag;

			Iterator() {}
			Iterator(EntityStorage::iterator storage_it) : curr_it_(storage_it) {}

			Iterator& operator=(const Iterator& rhs) {
				curr_it_ = rhs.curr_it_;
			}

			reference operator*() {
				return curr_it_->second;
			}
			const reference operator*() const {
				return curr_it_->second;
			}

			reference operator->() {
				return curr_it_->second;
			}
			const reference operator->() const {
				return curr_it_->second;
			}

			Iterator& operator++() {
				++curr_it_;
				return *this;
			}
			Iterator operator++(int) {
				Iterator tmp(*this);
				++curr_it_;
				return tmp;
			}

			bool operator==(const Iterator& rhs) const {
				return curr_it_ == rhs.curr_it_;
			}

			bool operator!=(const Iterator& rhs) const {
				return curr_it_ != rhs.curr_it_;
			}
			

		private:
			std::unordered_map<Entity::EntityId, Entity*>::iterator curr_it_;
		};

		Iterator Begin() {
			return Iterator(entities_.begin());
		}

		Iterator End() {
			return Iterator(entities_.end());
		}

		/*
		template <typename T, typename... Args>
		Entity::EntityId CreateEntity(Args... args);
		*/

		void AddEntity(Entity* entity);

		Entity* GetEntity(Entity::EntityId id) {
			auto it = entities_.find(id);

			if (it != entities_.end()) {
				return it->second;
			}

			return nullptr;
		};

		template <typename T>
		T* GetEntity(Entity id);

		void DestroyEntity(Entity::EntityId id);

	private:
		EntityStorage entities_;
	};

	inline void EntityManager::AddEntity(Entity* entity) {
		assert(entities_.find(entity->GetEntityId()) == entities_.end());

		entities_.insert({ entity->GetEntityId(), entity });
	}

	template<typename T>
	inline T* EntityManager::GetEntity(Entity id) {
		auto it = entities_.find(id);

		if (it != entities_.end()) {
			return reinterpret_cast<T*>(it->second);
		}

		return nullptr;
	}

	inline void EntityManager::DestroyEntity(Entity::EntityId id) {
		auto it = entities_.find(id);

		assert(it != entities_.end());

		delete it->second;
		entities_.erase(id);
	}
}
