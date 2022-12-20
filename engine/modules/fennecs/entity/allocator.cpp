#include <fennecs/entity/allocator.hpp>

#include <iostream>

namespace fennecs {

Entity* EntityAllocator::Allocate(const EntityLayout& layout) {
  SizeType entity_size = sizeof(Entity);
  Uint8* memory = new Uint8[entity_size + layout.Stride()];
  return new(memory) Entity(memory + entity_size);
}

void EntityAllocator::Deallocate(Entity* entity) {
  delete[] reinterpret_cast<Uint8*>(entity);
}

}  // namespace fennecs