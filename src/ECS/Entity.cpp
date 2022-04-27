#include "Entity.h"

/*
namespace ECS {

	IEntity::EntityId EntityIdGenerator::id_counter = 0;

	IEntity::IEntity() : id_(EntityIdGenerator::Get()) {

	}

	IEntity::~IEntity() {

	}

	const IEntity::EntityId& IEntity::GetEntityId() {
		return id_;
	}


	EntityIdGenerator::EntityIdGenerator() {

	}

	EntityIdGenerator::~EntityIdGenerator() {

	}

	const IEntity::EntityId& EntityIdGenerator::Get() {
		++id_counter;
		return id_counter;
	}

}
*/

namespace ecs {
	Entity::EntityId EntityIdGenerator::id_counter = 0;

	const Entity::EntityId& EntityIdGenerator::Get() {
		++id_counter;
		return id_counter;
	}

	Entity::Entity() : id_(EntityIdGenerator::Get()) {

	};
}