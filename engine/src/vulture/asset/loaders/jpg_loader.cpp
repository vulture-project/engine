#include <vulture/asset/detail/asset_loader_registrar.hpp>
#include <vulture/asset/detail/texture_loader.hpp>
#include <vulture/asset/loaders/jpg_loader.hpp>
#include <vulture/renderer/texture.hpp>

namespace vulture {

JPGLoader::JPGLoader(RenderDevice& device) : device_(device) {}

StringView JPGLoader::Extension() const {
  return StringView{".jpg"};
}

SharedPtr<IAsset> JPGLoader::Load(const String& path) {
  return detail::LoadTexture(device_, path);
}

}  // namespace vulture