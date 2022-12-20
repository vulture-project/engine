#include <fennecs/entity/layout.hpp>

namespace fennecs {

EntityLayout EntityLayout::Empty() {
  return EntityLayout{};
}

SizeType EntityLayout::Stride() const {
  return stride_;
}

SizeType EntityLayout::Offset(SizeType index) const {
  FENNECS_ASSERT(offsets_.count(index) == 1,
         "Attempt to get offset of component that does not contain in entity");

  return offsets_.at(index);
}

}  // namespace fennecs