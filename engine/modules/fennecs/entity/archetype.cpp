#include <fennecs/entity/archetype.hpp>

namespace fennecs {

bool EntityArchetype::Is(const EntityArchetype& archetype) const {
  return (mask_ & archetype.mask_) == archetype.mask_;
}

bool EntityArchetype::Equals(const EntityArchetype& archetype) const {
  return mask_ == archetype.mask_;
}

bool EntityArchetype::Has(SizeType index) const {
  BitMask mask = ComponentMaskByIndex(index);
  return (mask_ & mask) == mask;
}

EntityArchetype::EntityArchetype(BitMask mask)
    : mask_{mask} {
}

}  // namespace fennecs