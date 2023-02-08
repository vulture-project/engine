#ifndef ARCHETYPE_IMPL
#error Do not include this file directly
#endif

#include <iostream>

namespace fennecs {

namespace detail {

template <typename Component, typename... Components>
BitMask MaskOf() {
  if constexpr (sizeof...(Components) == 0) {
    return ComponentTraits<Component>::Mask();
  } else {
    return ComponentTraits<Component>::Mask() + MaskOf<Components...>();
  }
}

}  // namespace detail

template <typename... Components>
EntityArchetype EntityArchetype::ConsistsOf() {
  return EntityArchetype{detail::MaskOf<Components...>()};
}

template <typename Component>
bool EntityArchetype::Has() const {
  BitMask mask = ComponentTraits<Component>::Mask();
  return (mask_ & mask) == mask;
}

template <typename Component>
EntityArchetype EntityArchetype::Attach() const {
  return EntityArchetype{mask_ | ComponentTraits<Component>::Mask()};
}

template <typename Component>
EntityArchetype EntityArchetype::Detach() const {
  return EntityArchetype{mask_ & (~ComponentTraits<Component>::Mask())};
}

}  // namespace fennecs