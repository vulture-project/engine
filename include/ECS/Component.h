#pragma once

#include "ECS/TypeIdGenerator.h"


namespace ecs {

	class IComponent {
	public:
		IComponent() {};
		virtual ~IComponent() {};

		using ComponentTypeId = ecs::utility::TypeIdGenerator<IComponent>::TypeId;

		virtual const ComponentTypeId& GetComponentTypeId() const = 0;

	private:

	};

	template <typename T>
	class Component : public IComponent {
	public:
		Component();
		virtual ~Component();

		virtual const ComponentTypeId& GetComponentTypeId() const override;

		const static ComponentTypeId TypeId;
	};

	//to make custom entities u need to make derived classes from Entyty<\"You class name\">

	template <typename T>
	const typename IComponent::ComponentTypeId Component<T>::TypeId = ecs::utility::TypeIdGenerator<IComponent>::Get<T>();

	template<typename T>
	inline Component<T>::Component() {

	}

	template<typename T>
	inline Component<T>::~Component() {

	}

	template<typename T>
	inline const typename IComponent::ComponentTypeId& Component<T>::GetComponentTypeId() const {
		return TypeId;
	}

}	//namespace ecs
