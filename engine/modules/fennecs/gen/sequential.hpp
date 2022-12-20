#pragma once

#include <fennecs/core/types.hpp>

namespace fennecs {

class SequentialGenerator {
 public:
  static SizeType Next();

 private:
  static SizeType kCurrent;
};

}  // namespace fennecs