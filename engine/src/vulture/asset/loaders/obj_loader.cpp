#include <vulture/asset/detail/asset_loader_registrar.hpp>
#include <vulture/asset/loaders/detail/parse_obj.hpp>
#include <vulture/asset/loaders/obj_loader.hpp>

namespace vulture {

StringView OBJLoader::Extension() const {
  return StringView{".obj"};
}

SharedPtr<IAsset> OBJLoader::Load(const String& path) {
  return detail::ParseMeshWavefront(path);
}

}  // namespace vulture