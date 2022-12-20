#pragma once

#include <fennecs/entity/array.hpp>
#include <fennecs/entity/handle.hpp>

#include <list>

namespace fennecs {

class EntityWorld;

/**
 * @brief Stream of entities.
 */
class EntityStream {
  friend class EntityWorld;

 public:
  /**
   * @brief Returns next handle to entity or null if all entities have been
   * returned.
   */
  EntityHandle Next();

 private:
  EntityStream(const EntityArchetype& archetype, Node<EntityArray>* head, Node<EntityArray>* tail);

 private:
  EntityArchetype archetype_;
  Node<EntityArray>* curr_array_node_;
  Node<EntityArray>* tail_array_node_;
  Node<Entity>* curr_entity_node_;
  Node<Entity>* tail_entity_node_;
};

}  // namespace ecs