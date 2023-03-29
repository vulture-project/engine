#include <assimp/postprocess.h>

#include <assimp/Importer.hpp>
#include <vulture/asset/detail/asset_loader_registrar.hpp>
#include <vulture/asset/detail/mesh_loader.hpp>
#include <vulture/asset/loaders/obj_loader.hpp>

namespace vulture {

OBJLoader::OBJLoader(RenderDevice& device) : device_(device) {}

StringView OBJLoader::Extension() const {
  return StringView{".obj"};
}

SharedPtr<IAsset> OBJLoader::Load(const String& path) {
  return detail::LoadMesh(device_, path);
}

}  // namespace vulture