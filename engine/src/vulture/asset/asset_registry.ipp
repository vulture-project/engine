#ifndef ASSET_REGISTRY_IMPL
#error Do not include this file directly
#endif

namespace vulture {

template <typename TAsset>
SharedPtr<TAsset> AssetRegistry::Load(const String& path) {
  if (auto asset = TryFindAsset(path)) {
    return std::static_pointer_cast<TAsset>(asset);
  }

  if (auto loader = TryFindLoader(detail::Extension(path))) {
    return std::static_pointer_cast<TAsset>(FetchInsertAsset(path, loader->Load(assets_folder_ / path)));
  }

  LOG_ERROR("Unable to load {}", path);
  return nullptr;
}

}  // namespace vulture