#pragma once

#include <vulture/asset/asset_registry.hpp>

#include <vulture/core/core.hpp>

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
#include <vulture/asset/detail/asset_loader_registrar.ipp>
#undef ASSET_LOADER_REGISTRAR_IMPL