#include <vulture/asset/detail/asset_loader_registrar.hpp>
#include <vulture/asset/loaders/png_loader.hpp>
#include <vulture/renderer/texture.hpp>

namespace vulture {

StringView PNGLoader::Extension() const {
  return StringView{".png"};
}

SharedPtr<IAsset> PNGLoader::Load(const String& path) {
  return Texture::Create(path);
}

}  // namespace vulture