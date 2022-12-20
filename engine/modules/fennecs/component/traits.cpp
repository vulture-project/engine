#include <fennecs/component/traits.hpp>

namespace fennecs {

BitMask ComponentMaskByIndex(SizeType index) {
  return 1ull << index;
}

SizeType ComponentMaxIndex() {
  return 64ull;
}

}  // namespace fennecs