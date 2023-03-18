#include <vulture/asset/detail/asset_loader_registrar.hpp>
#include <vulture/asset/loaders/jpeg_loader.hpp>
#include <vulture/renderer/texture.hpp>

namespace vulture {

StringView JPEGLoader::Extension() const {
  return StringView{".jpeg"};
}

SharedPtr<IAsset> JPEGLoader::Load(const String& path) {
  return Texture::Create(path);
}

}  // namespace vulture