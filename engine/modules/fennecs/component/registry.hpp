#pragma once

#include <fennecs/core/assert.hpp>
#include <fennecs/core/types.hpp>
#include <fennecs/component/traits.hpp>
#include <fennecs/component/virtual_table.hpp>
#include <fennecs/containers/bit_mask.hpp>
#include <fennecs/core/types.hpp>

#include <unordered_map>

namespace fennecs {

class ComponentRegistry {
 public:
  ComponentVirtualTable& VirtualTable(SizeType index);

  template <typename Component>
  void RegisterVirtualTableOf();

  template <typename Component>
  [[nodiscard]] bool HasVirtualTableOf();

 private:
  std::unordered_map<SizeType, ComponentVirtualTable> virtual_tables_;
};

}  // namespace fennecs

#define REGISTRY_IMPL
#include <fennecs/component/registry.ipp>
#undef REGISTRY_IMPL