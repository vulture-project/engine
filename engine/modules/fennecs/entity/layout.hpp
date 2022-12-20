#pragma once

#include <fennecs/core/assert.hpp>
#include <fennecs/core/types.hpp>
#include <fennecs/component/traits.hpp>

#include <unordered_map>

namespace fennecs {

/**
 * @brief Layout describes how component placed in memory.
 */
class EntityLayout {
 public:
  /**
   * @brief Creates layout for empty entity.
   */
  static EntityLayout Empty();

 public:
  /**
   * @brief Returns stride between two entities in array.
   */
  [[nodiscard]] SizeType Stride() const;

  /**
   * @brief Returns offset of given component specified by its index.
   */
  [[nodiscard]] SizeType Offset(SizeType index) const;

  /**
   * @brief Returns offset of given component specified by its type.
   */
  template <typename Component>
  [[nodiscard]] SizeType OffsetOf() const;

  /**
   * @brief Create layout copy but with addition component.
   */
  template <typename Component>
  [[nodiscard]] EntityLayout Attach() const;

  /**
   * @brief Create layout copy but without component.
   */
  template <typename Component>
  [[nodiscard]] EntityLayout Detach() const;

 private:
  std::unordered_map<SizeType, SizeType> offsets_;
  SizeType stride_;
};

}  // namespace fennecs

#define ENTITY_LAYOUT_IMPL
#include <fennecs/entity/layout.ipp>
#undef ENTITY_LAYOUT_IMPL