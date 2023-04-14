#include <vulture/asset/detail/asset_loader_registrar.hpp>
#include <vulture/asset/detail/texture_loader.hpp>
#include <vulture/asset/loaders/png_loader.hpp>
#include <vulture/renderer/texture.hpp>

namespace vulture {

PNGLoader::PNGLoader(RenderDevice& device) : device_(device) {}

StringView PNGLoader::Extension() const {
  return StringView{".png"};
}

SharedPtr<IAsset> PNGLoader::Load(const String& path) {
  return detail::LoadTexture(device_, path);
}

}  // namespace vulture