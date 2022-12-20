#ifndef COMPONENT_TRAITS_IMPL
#error Do not include this file directly
#endif

#include <iostream>

namespace fennecs {

template <typename Component>
BitMask ComponentTraits<Component>::Mask() {
  return 1 << kIndex;
}

template <typename Component>
Uint64 ComponentTraits<Component>::Index() {
  return kIndex;
}

template <typename Component>
template <typename C, std::enable_if_t<std::is_move_constructible_v<C>, int>>
void ComponentTraits<Component>::Move(Uint8* source, Uint8* destination) {
  new(destination) Component(::std::move(*reinterpret_cast<Component*>(source)));
}

template <typename Component>
template <typename C, std::enable_if_t<!std::is_move_constructible_v<C> && std::is_copy_constructible_v<C>, int>>
void ComponentTraits<Component>::Move(Uint8* source, Uint8* destination) {
  new(destination) Component(*reinterpret_cast<Component*>(source));
}

template <typename Component>
void ComponentTraits<Component>::Destroy(Uint8* place) {
  reinterpret_cast<Component*>(place)->~Component();
}

template <typename Component>
SizeType ComponentTraits<Component>::kIndex{SequentialGenerator::Next()};

}  // namespace fennecs