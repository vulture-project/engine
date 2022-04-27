#pragma once

#include <unordered_map>
#include <functional>

#include "Entity.h"
#include "EntityManager.h"
#include "TestEntities.h"

//for creating entities in runtime
/*
namespace ECS {

	class EntityFactory {
	public:
		EntityFactory(EntityManager* manager);
		~EntityFactory();

		using construct_p = void (EntityFactory::*)();

		//insert functions here;
		void CreateEntityI();
		void CreateEntityII();
		void CreateEntityII(int a);

		void CreateEntityByName(const std::string& func_name);

		template <typename T, typename... Args>
		void CreateEntity(Args... args);

	private:
		EntityManager* manager_;
		std::unordered_map<std::string, construct_p> construct_functions_;
	};

	template<typename T, typename ...Args>
	inline void EntityFactory::CreateEntity(Args ...args) {
		manager_->CreateEntity<T>(std::forward<Args>(args)...);
	}

} //namespace ECS
*/
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
			LOG_DEBUG(entity_factory, "invoking from entity factory  %a", func_name);

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

