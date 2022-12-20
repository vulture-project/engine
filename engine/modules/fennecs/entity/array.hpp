#pragma once

#include <fennecs/entity/archetype.hpp>
#include <fennecs/entity/handle.hpp>
#include <fennecs/entity/impl.hpp>
#include <fennecs/entity/layout.hpp>
#include <fennecs/intrusive/list.hpp>
#include <fennecs/intrusive/node.hpp>

namespace fennecs {

class EntityArray : public Node<EntityArray> {
 public:
  EntityArray(const EntityArchetype& archetype, const EntityLayout& layout);

  [[nodiscard]] const EntityArchetype& Archetype() const;

  [[nodiscard]] const EntityLayout& Layout() const;

  [[nodiscard]] bool IsEmpty() const;

  [[nodiscard]] Node<Entity>* Head();

  [[nodiscard]] Node<Entity>* Tail();

  void Insert(Entity* node);

 private:
  EntityArchetype archetype_;
  EntityLayout layout_;
  IntrusiveList<Entity> entities_;
};

}  // namespace fennecs