#include "ECS/entity.h"


namespace ecs {
	Entity::EntityId EntityIdGenerator::id_counter = 0;

	const Entity::EntityId& EntityIdGenerator::Get() {
		++id_counter;
		return id_counter;
	}

	Entity::Entity() : id_(EntityIdGenerator::Get()) {

	};
}