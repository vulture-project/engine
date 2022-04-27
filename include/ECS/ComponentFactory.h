#pragma once

#include "Component.h"
#include "Entity.h"

/*
namespace ECS {

	class ComponentFactory {
	public:
		ComponentFactory(ComponentManager* manager);
		~ComponentFactory();

		using construct_p = void (ComponentFactory::*)(IEntity::EntityId);
	
		void CreateComponentI(IEntity::EntityId id);
		void CreateComponentII(IEntity::EntityId id);
		void CreateComponentIII(IEntity::EntityId id);

		void CreateComponentByName(IEntity::EntityId id, const std::string& func_name);

		template <typename T, typename... Args>
		void CreateComponent(IEntity::EntityId id, Args... args);

	private:
		ComponentManager* manager_;
		std::unordered_map<std::string, construct_p> construct_functions_;
	};

	template<typename T, typename ...Args>
	inline void ComponentFactory::CreateComponent(IEntity::EntityId id, Args ...args) {
		manager_->CreateComponent<T>(id, std::forward<Args>(args)...);
	}

} //namespace ECS
*/

namespace ecs {

	class ComponentFactory {
	public:
		ComponentFactory() {};
		~ComponentFactory() {};

		using construct_p = void (ComponentFactory::*)();

		//insert functions here for runtime;
		void CreateEntityI() {};
		void CreateEntityII() {};

		void CreateComponentByName(const std::string& func_name) {
			LOG_DEBUG(entity_factory, "invoking from entity factory  %a", func_name);

			assert(construct_functions_.find(func_name) != construct_functions_.end());

			std::invoke(construct_functions_[func_name], this);
		};

		template <typename T, typename... Args>
		Entity* CreateEntity(Args... args);

	private:
		std::unordered_map<std::string, construct_p> construct_functions_;
	};

	template<typename T, typename ...Args>
	inline Entity* ComponentFactory::CreateEntity(Args ...args) {
		return new T(std::forward<Args>(args)...);
	}

} //namespace ecs

