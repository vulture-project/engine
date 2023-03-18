#pragma once

#include <vulture/asset/asset.hpp>
#include <vulture/asset/asset_loader.hpp>
#include <vulture/core/core.hpp>

namespace vulture {

class OBJLoader : public IAssetLoader {
 public:
  StringView Extension() const override;

  SharedPtr<IAsset> Load(const String& path) override;
};

}  // namespace vulture