#pragma once

#include <fennecs/core/types.hpp>
#include <fennecs/component/traits.hpp>

namespace fennecs {

class ComponentVirtualTable {
 public:
  using MoveFunction = void(*)(Uint8*, Uint8*);

  using DestroyFunction = void(*)(Uint8*);

 public:
  template <typename Component>
  static ComponentVirtualTable Of();

 public:
  ComponentVirtualTable(MoveFunction move, DestroyFunction destroy);

  void Move(Uint8* source, Uint8* destination);

  void Destroy(Uint8* place);

 private:
  MoveFunction move_;
  DestroyFunction destroy_;
};

}  // namespace fennecs

#define VIRTUAL_TABLE_IMPL
#include <fennecs/component/virtual_table.ipp>
#undef VIRTUAL_TABLE_IMPL