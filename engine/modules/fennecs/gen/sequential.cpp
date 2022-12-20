#include <fennecs/gen/sequential.hpp>

namespace fennecs {

SizeType SequentialGenerator::Next() {
  return kCurrent++;
}

SizeType SequentialGenerator::kCurrent {

};

}  // namespace fennecs