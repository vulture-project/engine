#pragma once

#include <fennecs/core/types.hpp>
#include <fennecs/entity/array.hpp>
#include <fennecs/intrusive/list.hpp>

namespace fennecs {

class EntityRegistry {
 public:
  [[nodiscard]] Node<EntityArray>* Head();

  [[nodiscard]] Node<EntityArray>* Tail();

  EntityArray* AddArray(const EntityArchetype& archetype, const EntityLayout& layout);

  EntityArray* FindArray(const EntityArchetype& archetype);

 private:
  IntrusiveList<EntityArray> arrays_;
};

}  // namespace fennecs