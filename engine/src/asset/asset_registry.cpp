#include <asset/asset_registry.hpp>

namespace vulture {

AssetRegistry* AssetRegistry::Instance() {
  if (instance_ == nullptr) {
    instance_ = new AssetRegistry();
  }
  return instance_;
}

AssetRegistry* AssetRegistry::instance_{nullptr};

void AssetRegistry::RegisterLoader(SharedPtr<IAssetLoader> loader) {
  InsertLoader(loader);
}

SharedPtr<IAsset> AssetRegistry::FetchInsertAsset(const String& path, SharedPtr<IAsset> asset) {
  assets_.emplace(path, asset);
  return asset;
}

SharedPtr<IAsset> AssetRegistry::TryFindAsset(const String& path) {  
  if (auto iter = assets_.find(path); iter != assets_.end()) {
    return iter->second;
  }
  return SharedPtr<IAsset>{};
}

void AssetRegistry::InsertLoader(SharedPtr<IAssetLoader> loader) {
  auto result = loaders_.emplace(loader->Extension(), loader);
}

SharedPtr<IAssetLoader> AssetRegistry::TryFindLoader(StringView extension) {
  if (auto iter = loaders_.find(extension); iter != loaders_.end()) {
    return iter->second;
  }
  return SharedPtr<IAssetLoader>{};
}

}  // namespace vulture