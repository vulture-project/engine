#pragma once

#include <asset/asset.hpp>
#include <asset/asset_loader.hpp>

#include <core/core.hpp>

namespace vulture {

class JPGLoader : public IAssetLoader {
 public:
  StringView Extension() const override;

  SharedPtr<IAsset> Load(const String& path) override;
};

}  // namespace vulture