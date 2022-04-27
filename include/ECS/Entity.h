#pragma once

#include <unordered_map>
#include <cassert>

//#include "stdafx.h"
#include "TypeIdGenerator.h"
#include "Component.h"

/*
namespace ECS {

	class IEntity {
	public:
		IEntity();
		virtual ~IEntity();

		bool operator==(const IEntity& rhs) {
			return id_ == rhs.id_;
		}
		bool operator<(const IEntity& rhs) {
			return id_ < rhs.id_;
		}

		using EntityId = size_t;
		using EntityTypeId = utility::TypeIdGenerator<IEntity>::TypeId;

		virtual const EntityTypeId& GetEntityTypeId() const = 0;
		const EntityId& GetEntityId();

	private:
		EntityId id_;
	};

	template <typename T>
	class Entity : public IEntity {
	public:
		Entity();
		virtual ~Entity();

		virtual const EntityTypeId& GetEntityTypeId() const override;

	private:
		static const EntityTypeId type_id_;
	};

	//to make custom entities u need to make derived classes from Entyty<\"You class name\">

	class EntityIdGenerator {
	public:
		EntityIdGenerator();
		~EntityIdGenerator();

		static const IEntity::EntityId& Get();

	private:
		static IEntity::EntityId id_counter;
	};


	template <typename T>
	const IEntity::EntityTypeId Entity<T>::type_id_ = utility::TypeIdGenerator<IEntity>::Get<T>();

	template<typename T>
	inline Entity<T>::Entity() {

	}

	template<typename T>
	inline Entity<T>::~Entity() {

	}

	template<typename T>
	inline const IEntity::EntityTypeId& Entity<T>::GetEntityTypeId() const {
		return type_id_;
	}

}	//namespace ECS

*/

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

	//-----------------------------------------------------------------------------

	/*
	class MovingBox : public Entity {
	public:
		MovingBox(PosMoveComponent* move, BasicRenderComponent* render_comp) {
			AddComponent(move);
			AddComponent(render_comp);
		};
	};
	*/

} //namespace ecs