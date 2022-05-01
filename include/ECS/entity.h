#pragma once

#include <unordered_map>
#include <cassert>

#include "ECS/type_id_generator.h"
#include "ECS/component.h"

namespace ecs {

	class Entity {
	public:
		Entity();
		virtual ~Entity() {
			while (!components_.empty()) {
				auto it = components_.begin();
				delete it->second;
				components_.erase(it->first);
			}
		};

		using EntityId = size_t;
		using ComponentStorage = std::unordered_map<IComponent::ComponentTypeId, IComponent*>;


		const EntityId& GetEntityId() {
			return id_;
		};

		void AddComponent(IComponent* component) {
			IComponent::ComponentTypeId type_id = component->GetComponentTypeId();
			
			assert(components_.find(type_id) == components_.end());
			
			components_.insert({ type_id, component });
		};

		template <typename T>
		T* GetComponent() {
			auto it = components_.find(T::TypeId);
			if (it != components_.end()) {
				return reinterpret_cast<T*>(it->second);
			}
			return nullptr;
		};

		template <typename T>
		const T* GetComponent() const {
			auto it = components_.find(T::TypeId);
			if (it != components_.end()) {
				return reinterpret_cast<const T*>(it->second);
			}
			return nullptr;
		};

		template <typename T>
		void DestroyComponent() {
			IComponent::ComponentTypeId type_id = T::TypeId;

			auto it = components_.find(type_id);

			assert(it != components_.end());

			delete it->second;
			components_.erase(type_id);
		};

	private:
		EntityId id_;
		ComponentStorage components_;
	};

	class EntityIdGenerator {
	public:
		EntityIdGenerator();
		~EntityIdGenerator();

		static const Entity::EntityId& Get();

	private:
		static Entity::EntityId id_counter;
	};

} //namespace ecs