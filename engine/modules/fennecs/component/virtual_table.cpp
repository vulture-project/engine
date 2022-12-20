#include <fennecs/component/virtual_table.hpp>

namespace fennecs {

ComponentVirtualTable::ComponentVirtualTable(MoveFunction move,
                                             DestroyFunction destroy)
    : move_{move},
      destroy_{destroy} {
}

void ComponentVirtualTable::Move(Uint8* source, Uint8* destination) {
  move_(source, destination);
}

void ComponentVirtualTable::Destroy(Uint8* place) {
  destroy_(place);
}

}  // namespace fennecs