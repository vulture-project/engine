#include <vulture/asset/detail/asset_loader_registrar.hpp>
#include <vulture/asset/loaders/jpeg_loader.hpp>
#include <vulture/renderer/texture.hpp>

namespace vulture {

StringView JPEGLoader::Extension() const {
  return StringView{".jpeg"};
}

SharedPtr<IAsset> JPEGLoader::Load(const String& path) {
  VULTURE_ASSERT(false, "Not implemented!");
  return nullptr;
}

}  // namespace vulture