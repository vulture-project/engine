#pragma once

#include <fennecs/core/types.hpp>
#include <fennecs/core/utility.hpp>
#include <fennecs/containers/bit_mask.hpp>
#include <fennecs/gen/sequential.hpp>

namespace fennecs {

/**
 * @brief Returns bit that corresponds to component specified by given index.
 */
BitMask ComponentMaskByIndex(SizeType index);

/**
 * @brief Returns maximal index of component.
 */
SizeType ComponentMaxIndex();

/**
 * @brief Component traits.
 */
template <typename Component>
class ComponentTraits {
 public:
  /**
   * @brief Returns bit mask that contains unique component bit.
   */
  static BitMask Mask();

  /**
   * @brief Returns unique component index.
   */
  static Uint64 Index();

  /**
   * @brief
   */
  template <typename C = Component, std::enable_if_t<std::is_move_constructible_v<C>, int> = 0>
  static void Move(Uint8* source, Uint8* destination);

  template <typename C = Component, std::enable_if_t<!std::is_move_constructible_v<C> && std::is_copy_constructible_v<C>, int> = 0>
  static void Move(Uint8* source, Uint8* destination);

  /**
   * @brief
   */
  static void Destroy(Uint8* place);

 private:
  static SizeType kIndex;
};

}  // namespace fennecs

#define COMPONENT_TRAITS_IMPL
#include <fennecs/component/traits.ipp>
#undef COMPONENT_TRAITS_IMPL