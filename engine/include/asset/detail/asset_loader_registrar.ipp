#ifndef ASSET_LOADER_REGISTRAR_IMPL
#error Do not include this file directly
#endif

#include <iostream>

namespace vulture {

namespace detail {

template <typename TAssetLoader>
AssetLoaderRegistrar<TAssetLoader>::AssetLoaderRegistrar() {
  AssetRegistry::Instance()->RegisterLoader(CreateShared<TAssetLoader>());
}

}  // namespace detail

}  // namespace vulture
