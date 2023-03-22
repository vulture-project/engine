#pragma once

#include <assimp/scene.h>

#include <vulture/asset/asset.hpp>
#include <vulture/asset/asset_loader.hpp>
#include <vulture/core/core.hpp>
#include <vulture/renderer/geometry/mesh.hpp>
#include <vulture/renderer/material_system/material.hpp>

namespace vulture {

class OBJLoader : public IAssetLoader {
 public:
  OBJLoader(RenderDevice& device);

  StringView Extension() const override;

  SharedPtr<IAsset> Load(const String& path) override;

 private:
  void LoadMaterials(const aiScene* scene, Vector<SharedPtr<Material>>& materials);
 
 private:
  RenderDevice& device_;
};

}  // namespace vulture