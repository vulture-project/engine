#include <asset/detail/asset_loader_registrar.hpp>

#include <asset/loaders/obj_loader.hpp>

#include <asset/loaders/detail/parse_obj.hpp>

namespace vulture {

StringView OBJLoader::Extension() const {
  return StringView{".obj"};
}

SharedPtr<IAsset> OBJLoader::Load(const String& path) {
  return detail::ParseMeshWavefront(path);
}

}  // namespace vulture