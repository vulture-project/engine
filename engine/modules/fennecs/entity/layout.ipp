#ifndef ENTITY_LAYOUT_IMPL
#error Do not include this file directly
#endif

namespace fennecs {

template <typename Component>
SizeType EntityLayout::OffsetOf() const {
  FENNECS_ASSERT(offsets_.count(ComponentTraits<Component>::Index()) == 1,
         "Attempt ot get offset of component that does not contain in entity");

  return offsets_.at(ComponentTraits<Component>::Index());
}

template <typename Component>
EntityLayout EntityLayout::Attach() const {
  FENNECS_ASSERT(offsets_.count(ComponentTraits<Component>::Index()) != 1,
         "Attempt to attach already attached component");

  EntityLayout layout{*this};
  layout.offsets_.emplace(ComponentTraits<Component>::Index(), stride_);
  layout.stride_ += sizeof(Component);
  return layout;
}

template <typename Component>
EntityLayout EntityLayout::Detach() const {
  FENNECS_ASSERT(false, "TODO: Implement it.");

  return EntityLayout{};
}

}  // namespace fennecs