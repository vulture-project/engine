#include <fennecs/gen/sequential.hpp>

#include <iostream>

namespace fennecs {

SizeType SequentialGenerator::Next() {
  return kCurrent++;
}

SizeType SequentialGenerator::kCurrent{0};

}  // namespace fennecs