#pragma once

#include <fennecs/core/assert.hpp>
#include <fennecs/core/types.hpp>
#include <fennecs/intrusive/node.hpp>

namespace fennecs {

class Entity : public Node<Entity> {
 public:
  Entity(Uint8* data);

  Uint8* Data();

  const Uint8* Data() const;

 private:
  Uint8* data_;
};

}  // namespace fennecs