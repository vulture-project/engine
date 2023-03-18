#pragma once

#include <vulture/asset/asset.hpp>
#include <vulture/core/core.hpp>

namespace vulture {

class IAssetLoader {
 public:
  virtual ~IAssetLoader() = default;

  virtual StringView Extension() const = 0;

  virtual SharedPtr<IAsset> Load(const String& path) = 0;
};

}  // namespace vulture