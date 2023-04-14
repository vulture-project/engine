#pragma once

#include <vulture/asset/asset.hpp>
#include <vulture/asset/asset_loader.hpp>
#include <vulture/core/core.hpp>

namespace vulture {

class RenderDevice;

class JPGLoader : public IAssetLoader {
 public:
  JPGLoader(RenderDevice& device);

  StringView Extension() const override;

  SharedPtr<IAsset> Load(const String& path) override;

 private:
  RenderDevice& device_;
};

}  // namespace vulture