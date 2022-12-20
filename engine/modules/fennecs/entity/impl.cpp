#include <fennecs/entity/impl.hpp>

namespace fennecs {

Entity::Entity(Uint8* data)
    : data_{data} {
}

Uint8* Entity::Data() {
  return data_;
}

const Uint8* Entity::Data() const {
  return data_;
}

}  // namespace fennecs