#pragma once

#include <cassert>
#include <cstdint>
#include <vulture/core/core.hpp>

namespace vulture {

DECLARE_ENUM_TO_STR(ShaderDataType,
                    kInvalid,

                    kBoolean,
                    kInt,
                    kUInt,
                    kFloat,

                    kVec2,
                    kVec3,
                    kVec4,

                    kIVec2,
                    kIVec3,
                    kIVec4,

                    kMat2,
                    kMat3,
                    kMat4);

}  // namespace vulture
