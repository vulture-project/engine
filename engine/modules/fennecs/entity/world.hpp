#pragma once

#include <fennecs/core/utility.hpp>
#include <fennecs/component/registry.hpp>
#include <fennecs/entity/allocator.hpp>
#include <fennecs/entity/archetype.hpp>
#include <fennecs/entity/array.hpp>
#include <fennecs/entity/handle.hpp>
#include <fennecs/entity/impl.hpp>
#include <fennecs/entity/layout.hpp>
#include <fennecs/entity/registry.hpp>
#include <fennecs/entity/stream.hpp>

namespace fennecs {

/**
 * @brief
 */
class EntityWorld {
 public:
  /**
   * @brief Creates empty entity world.
   */
  EntityWorld();

  /**
   * @brief Adds new entity to world.
   */
  EntityHandle AddEntity();

  /**
   * @brief Removes existing entity from world.
   */
  void RemoveEntity(EntityHandle);

  /**
   * @brief Attaches additional component to existing entity.
   */
  template <typename Component, typename... ArgTypes>
  EntityHandle Attach(EntityHandle handle, ArgTypes&& ... args);

  /**
   * @brief Detaches component from existing entity.
   */
  template <typename Component>
  EntityHandle Detach(EntityHandle handle);

  /**
   * @brief Queries stream of entities of given archetype.
   */
   template <typename... Components>
   EntityStream Query();

 private:
  template <typename Component>
  EntityArray* FindArrayOnAttach(EntityHandle handle);

  template <typename Component>
  EntityArray* FindArrayOnDetach(EntityHandle handle);

  void Move(const EntityArchetype& archetype,
            const EntityLayout& layout,
            Entity* entity,
            const EntityArchetype& new_archetype,
            const EntityLayout& new_layout,
            Entity* new_entity);

  void Destroy(const EntityArchetype& archetype,
               const EntityLayout& layout,
               Entity* entity);

 private:
  ComponentRegistry component_registry_;
  EntityAllocator entity_allocator_;
  EntityRegistry entity_registry_;
};

}  // namespace ecs

#define WORLD_IMPL
#include <fennecs/entity/world.ipp>
#undef WORLD_IMPL