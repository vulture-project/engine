#pragma once

#include <fennecs/core/types.hpp>
#include <fennecs/core/utility.hpp>
#include <fennecs/entity/impl.hpp>
#include <fennecs/entity/layout.hpp>

namespace fennecs {

class EntityAllocator {
 public:
  Entity* Allocate(const EntityLayout& layout);

  void Deallocate(Entity* entity);
};

}  // namespace fennecs