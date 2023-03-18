#include <vulture/asset/detail/asset_loader_registrar.hpp>
#include <vulture/asset/loaders/glsl_loader.hpp>
#include <vulture/renderer/shader.hpp>

namespace vulture {

StringView GLSLLoader::Extension() const {
  return StringView{".glsl"};
}

SharedPtr<IAsset> GLSLLoader::Load(const String& path) {
  return Shader::Create(path);
}

}  // namespace vulture