#pragma once

#include <asset/asset.hpp>
#include <asset/asset_loader.hpp>

#include <asset/detail/extension.hpp>

#include <core/core.hpp>

namespace vulture {

class AssetRegistry {
 public:
  static AssetRegistry* Instance();

 public:
  template <typename TAsset>
  SharedPtr<TAsset> Load(const String& path);

  void RegisterLoader(SharedPtr<IAssetLoader> loader);

 private:
  SharedPtr<IAsset> FetchInsertAsset(const String& path, SharedPtr<IAsset> asset);

  SharedPtr<IAsset> TryFindAsset(const String& path);

  void InsertLoader(SharedPtr<IAssetLoader> loader);

  SharedPtr<IAssetLoader> TryFindLoader(StringView extension);

 private:
  static AssetRegistry* instance_;

 private:
  HashMap<String, SharedPtr<IAsset>> assets_;

  HashMap<StringView, SharedPtr<IAssetLoader>> loaders_;
};

}  // namespace vulture

#define ASSET_REGISTRY_IMPL
#include <asset/asset_registry.ipp>
#undef ASSET_REGISTRY_IMPL