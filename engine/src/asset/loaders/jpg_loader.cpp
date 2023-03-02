#include <asset/detail/asset_loader_registrar.hpp>

#include <asset/loaders/jpg_loader.hpp>

#include <renderer/texture.hpp>

namespace vulture {

StringView JPGLoader::Extension() const {
  return StringView{".jpg"};
}

SharedPtr<IAsset> JPGLoader::Load(const String& path) {
  return Texture::Create(path);
}

}  // namespace vulture