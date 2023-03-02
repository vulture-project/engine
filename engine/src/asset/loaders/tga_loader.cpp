#include <asset/detail/asset_loader_registrar.hpp>

#include <asset/loaders/tga_loader.hpp>

#include <renderer/texture.hpp>

namespace vulture {

StringView TGALoader::Extension() const {
  return StringView{".tga"};
}

SharedPtr<IAsset> TGALoader::Load(const String& path) {
  return Texture::Create(path);
}

}  // namespace vulture