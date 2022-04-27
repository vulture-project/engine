#pragma once

#include "TypeIdGenerator.h"
#include "stdafx.h"
#include "Logger.h"

/*
namespace ECS {

	class IComponent {
	public:
		IComponent();
		virtual ~IComponent();

		using ComponentTypeId = utility::TypeIdGenerator<IComponent>::TypeId;

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
	const typename IComponent::ComponentTypeId Component<T>::TypeId = utility::TypeIdGenerator<IComponent>::Get<T>();

	template<typename T>
	inline Component<T>::Component() {

	}

	template<typename T>
	inline Component<T>::~Component() {

	}

	template<typename T>
	inline const typename IComponent::ComponentTypeId& Component<T>::GetComponentTypeId() const {
		return type_id_;
	}

}	//namespace ECS
*/

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


/*
	//-------------------------------------------------------------------------------------------------------------------

	class BasicRenderComponent : public Component<BasicRenderComponent> {
	public:
		BasicRenderComponent() {
			LOG_DEBUG(components, "CreatedRenderComponent: %a", this);
		};

		virtual ~BasicRenderComponent() {
			LOG_DEBUG(components, "DestroyedRenderComponent: %a", this);
		};

		void Render(sf::RenderTarget& target, sf::Vector2f pos) {
			sf::RectangleShape rectangle;
		
			rectangle.setSize(sf::Vector2f(30, 30));
			rectangle.setOutlineColor(sf::Color::Red);
			rectangle.setOutlineThickness(3);
			rectangle.setPosition(pos);

			target.draw(rectangle);
		};

	private:
		
	};

	class PosMoveComponent : public Component<PosMoveComponent> {
	public:
		PosMoveComponent(sf::Vector2f pos) : pos_(pos), speed_(0, 0) {
			LOG_DEBUG(components, "CreatedPosMoveComponent: %a", this);
		};

		virtual ~PosMoveComponent() {
			LOG_DEBUG(components, "DestroyedPosMoveComponent: %a", this);
		};

		void SetPos(const sf::Vector2f& pos) {
			pos_ = pos;
		};

		sf::Vector2f GetPos() {
			return pos_;
		}

		void Update() {
			speed_ = { 0.0f, 0.0f };

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
				speed_.y = 1.0f;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
				speed_.y = -1.0f;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
				speed_.x = +1.0f;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
				speed_.x = -1.0f;
			}
			
			pos_.x += speed_.x;
			pos_.y += speed_.y;
		}

	private:
		sf::Vector2f pos_;
		sf::Vector2f speed_;
	};

	using Vec3f = sf::Vector3f;

	class Position : public Component<Position> {
	public:
		Position() {}
		Position(const Vec3f& abs_pos, const Vec3f& rel_pos) : abs_pos_(abs_pos), rel_pos_(rel_pos) {}

		//SetRelPos()
		//SetRelPos()

	private:
		Vec3f rel_pos_;
		Vec3f abs_pos_;
	};

	class MeshComponent : public Component<MeshComponent> {
	public:
		using Mesh = void;

		MeshComponent() {}
		MeshComponent(Mesh* mesh) : mesh_(mesh) {}
		virtual ~MeshComponent() {};

		Mesh* GetMesh() {
			return mesh_;
		}

	private:
		Mesh* mesh_;
	};
*/
}	//namespace ecs
