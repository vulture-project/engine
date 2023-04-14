#include <vulture/asset/detail/asset_loader_registrar.hpp>
#include <vulture/asset/loaders/glsl_loader.hpp>
#include <vulture/renderer/material_system/shader.hpp>

namespace vulture {

StringView GLSLLoader::Extension() const {
  return StringView{".shader"};
}

SharedPtr<IAsset> GLSLLoader::Load(const String& path) {
  VULTURE_ASSERT(false, "Not implemented!");
  return nullptr;
}

}  // namespace vulture