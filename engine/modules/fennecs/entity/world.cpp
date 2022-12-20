#include <fennecs/entity/world.hpp>

namespace fennecs {

// 1. Create array where empty entities are placed.
EntityWorld::EntityWorld() {
  entity_registry_.AddArray(EntityArchetype{}, EntityLayout{});
}

// 1. Find array where empty entities are placed.
// 2. Allocate memory for new entity.
// 3. Insert entity to new array.
// 4. Return handle for new entity.
EntityHandle EntityWorld::AddEntity() {
  EntityArray* array = entity_registry_.FindArray(EntityArchetype{});
  Entity* entity = entity_allocator_.Allocate(array->Layout());
  array->Insert(entity);
  return EntityHandle{&array->Archetype(), &array->Layout(), entity};
}

// 1. Extract entity from the array where it is.
// 2. Destroy entity's components.
// 3. Deallocate entity's memory.
void EntityWorld::RemoveEntity(EntityHandle handle) {
  handle.entity_->Unlink();
  Destroy(*handle.archetype_, *handle.layout_, handle.entity_);
  entity_allocator_.Deallocate(handle.entity_);
}

void EntityWorld::Move(const EntityArchetype& archetype,
                       const EntityLayout& layout,
                       Entity* entity,
                       const EntityArchetype& new_archetype,
                       const EntityLayout& new_layout,
                       Entity* new_entity) {
  for (SizeType index = 0; index < ComponentMaxIndex(); ++index) {
    if (archetype.Has(index) && new_archetype.Has(index)) {
      ComponentVirtualTable& virtual_table = component_registry_.VirtualTable(index);
      virtual_table.Move(entity->Data() + layout.Offset(index),
                         new_entity->Data() + new_layout.Offset(index));
    }
  }
}

void EntityWorld::Destroy(const EntityArchetype& archetype,
                          const EntityLayout& layout,
                          Entity* entity) {
  for (SizeType index = 0; index < ComponentMaxIndex(); ++index) {
    if (archetype.Has(index)) {
      ComponentVirtualTable& virtual_table = component_registry_.VirtualTable(index);
      virtual_table.Destroy(entity->Data() + layout.Offset(index));
    }
  }
}

}  // namespace fennecs