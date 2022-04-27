#pragma once

#include <unordered_map>
#include <functional>

#include "ECS/Entity.h"
#include "ECS/EntityManager.h"

namespace ecs {

	class EntityFactory {
	public:
		EntityFactory();
		~EntityFactory() {};

		using construct_p = Entity* (EntityFactory::*)();

		//insert functions here for runtime;
		Entity* CreateEntityI() {};
		Entity* CreateEntityII() {};

		Entity* CreateEntityByName(const std::string& func_name) {

			assert(construct_functions_.find(func_name) != construct_functions_.end());

			return std::invoke(construct_functions_[func_name], this);
		};

		template <typename T, typename... Args>
		Entity* CreateEntity(Args... args);

	private:
		std::unordered_map<std::string, construct_p> construct_functions_;
	};

	template<typename T, typename ...Args>
	inline Entity* EntityFactory::CreateEntity(Args ...args) {
		return new T(std::forward<Args>(args)...);
	}

} //namespace ECS

