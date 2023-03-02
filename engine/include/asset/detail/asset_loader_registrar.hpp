#pragma once

#include <asset/asset_registry.hpp>

#include <core/core.hpp>

namespace vulture {

namespace detail {

template <typename TAssetLoader>
class AssetLoaderRegistrar {
 public:
  AssetLoaderRegistrar();
};

}  // namespace detail

}  // namespace vulture

#define ASSET_LOADER_REGISTRAR_IMPL
#include <asset/detail/asset_loader_registrar.ipp>
#undef ASSET_LOADER_REGISTRAR_IMPL