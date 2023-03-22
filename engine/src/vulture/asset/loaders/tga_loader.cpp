#include <vulture/asset/detail/asset_loader_registrar.hpp>
#include <vulture/asset/detail/texture_loader.hpp>
#include <vulture/asset/loaders/tga_loader.hpp>
#include <vulture/renderer/texture.hpp>

namespace vulture {

TGALoader::TGALoader(RenderDevice& device) : device_(device) {}

StringView TGALoader::Extension() const {
  return StringView{".tga"};
}

SharedPtr<IAsset> TGALoader::Load(const String& path) {
  return detail::LoadTexture(device_, path);
}

}  // namespace vulture