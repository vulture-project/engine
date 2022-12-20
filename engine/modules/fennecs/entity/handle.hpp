#pragma once

#include <fennecs/entity/archetype.hpp>
#include <fennecs/entity/layout.hpp>
#include <fennecs/entity/impl.hpp>

namespace fennecs {

class EntityStream;
class EntityWorld;

class EntityHandle {
 friend class EntityStream;
 friend class EntityWorld;

 public:
  static EntityHandle Null();

 public:
  template <typename Component>
  [[nodiscard]] bool Has() const;

  template <typename Component>
  [[nodiscard]] Component& Get();

  [[nodiscard]] bool IsNull() const;

  [[nodiscard]] bool IsEqual(const EntityHandle& handle) const;

 private:
  EntityHandle(const EntityArchetype* archetype,
               const EntityLayout* layout,
               Entity* entity);

 private:
  const EntityArchetype* archetype_;
  const EntityLayout* layout_;
  Entity* entity_;
};

}  // namespace ecs

#define ENTITY_HANDLE_IMPL
#include <fennecs/entity/handle.ipp>
#undef ENTITY_HANDLE_IMPL