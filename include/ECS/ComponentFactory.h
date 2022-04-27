#pragma once

#include "ECS/Component.h"
#include "ECS/Entity.h"

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

