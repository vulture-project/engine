#pragma once

#include <vulture/core/core.hpp>
#include <vulture/renderer/3d/mesh.hpp>

namespace vulture {

namespace detail {

SharedPtr<Mesh> ParseMeshWavefront(const std::string& filename);

using MapMaterials = std::map<std::string, SharedPtr<Material>>;

bool ParseMaterialsWavefront(const std::string& filename, MapMaterials& materials);

}  // namespace detail

}  // namespace vulture