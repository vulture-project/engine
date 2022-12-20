#include <fennecs/entity/handle.hpp>

namespace fennecs {

EntityHandle EntityHandle::Null() {
  return EntityHandle{nullptr, nullptr, nullptr};
}

EntityHandle::EntityHandle(const EntityArchetype* archetype,
                           const EntityLayout* layout,
                           Entity* entity)
    : archetype_{archetype}, layout_{layout}, entity_{entity} {
}

bool EntityHandle::IsNull() const {
  return entity_ == nullptr;
}

bool EntityHandle::IsEqual(const EntityHandle& handle) const {
  return entity_ == handle.entity_;
}

}  // namespace fennecs