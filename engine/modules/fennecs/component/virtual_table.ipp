#ifndef VIRTUAL_TABLE_IMPL
#error Do not include this file directly
#endif

namespace fennecs {

template <typename Component>
ComponentVirtualTable ComponentVirtualTable::Of() {
  return ComponentVirtualTable{
    ComponentTraits<Component>::Move,
    ComponentTraits<Component>::Destroy
  };
}

}  // namespace fennecs