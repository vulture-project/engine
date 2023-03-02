#include <asset/detail/asset_loader_registrar.hpp>

#include <asset/loaders/jpeg_loader.hpp>

#include <renderer/texture.hpp>

namespace vulture {

StringView JPEGLoader::Extension() const {
  return StringView{".jpeg"};
}

SharedPtr<IAsset> JPEGLoader::Load(const String& path) {
  return Texture::Create(path);
}

}  // namespace vulture