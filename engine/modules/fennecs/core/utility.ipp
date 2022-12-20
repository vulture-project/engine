#ifndef UTILITY_IMPL
#error Do not include this file directly
#endif

namespace fennecs {

constexpr SizeType Align(SizeType value, SizeType alignment) {
  return (value + alignment - SizeType{1}) / alignment;
}

}  // namespace fennecs