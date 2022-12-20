#ifndef ENTITY_HANDLE_IMPL
#error Do not include this file ditectly
#endif

namespace fennecs {

template <typename Component>
bool EntityHandle::Has() const {
  return archetype_->template Has<Component>();
}

template <typename Component>
Component& EntityHandle::Get() {
  Uint8* component = entity_->Data() + layout_->template OffsetOf<Component>();
  return *reinterpret_cast<Component*>(component);
}

}  // namespace fennecs