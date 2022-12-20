#ifndef ARCHETYPE_IMPL
#error Do not include this file directly
#endif

#include <iostream>

namespace fennecs {

namespace detail {

template <typename... Components>
struct Mask;

template <typename Component>
struct Mask<Component> {
  static const BitMask kValue;
};

template <typename Component>
const BitMask Mask<Component>::kValue = ComponentTraits<Component>::Mask();

template <typename Component, typename... Components>
struct Mask<Component, Components...> {
  static const BitMask kValue;
};

template <typename Component, typename... Components>
const BitMask Mask<Component, Components...>::kValue = ComponentTraits<Component>::Mask() | Mask<Components...>::kValue;

}  // namespace detail

template <typename... Components>
EntityArchetype EntityArchetype::ConsistsOf() {
  return EntityArchetype{detail::Mask<Components...>::kValue};
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