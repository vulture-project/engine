#pragma once

#include <fennecs/core/types.hpp>

#include <new>
#include <utility>

namespace fennecs {

constexpr SizeType Align(SizeType value, SizeType alignment);

}  // namespace fennecs

#define UTILITY_IMPL
#include <fennecs/core/utility.ipp>
#undef UTILITY_IMPL