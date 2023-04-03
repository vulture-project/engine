/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file mesh_loader.cpp
 * @date 2023-03-29
 * 
 * The MIT License (MIT)
 * Copyright (c) 2022 Nikita Mochalov
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <assimp/postprocess.h>

#include <assimp/Importer.hpp>
#include <vulture/asset/asset_registry.hpp>
#include <vulture/asset/detail/mesh_loader.hpp>

namespace vulture {
namespace detail {

void LoadMaterials(RenderDevice& device, const aiScene* scene, Vector<SharedPtr<Material>>& materials);

SharedPtr<Mesh> LoadMesh(RenderDevice& device, const String& path) {
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate |
                                                 aiProcess_SortByPType |
                                                 aiProcess_GenSmoothNormals |
                                                 aiProcess_CalcTangentSpace |
                                                 aiProcess_GenUVCoords);
  if (scene == nullptr) {
    LOG_ERROR("No file \"{}\" found", path);
    return nullptr;
  }

  if (scene->mRootNode == nullptr) {
    LOG_ERROR("File \"{}\" does not contain a root node!", path);
    return nullptr;
  }

  if (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
    LOG_ERROR("Scene in \"{}\" is incomplete!", path);
    return nullptr;
  }

  if (scene->mNumMeshes == 0) {
    LOG_ERROR("No meshes found in \"{}\"", path);
    return nullptr;
  }

  Vector<SharedPtr<Material>> materials;
  LoadMaterials(device, scene, materials);

  SharedPtr<Mesh> result_mesh = CreateShared<Mesh>();

  for (uint32_t mesh_idx = 0; mesh_idx < scene->mNumMeshes; ++mesh_idx) {
    aiMesh* mesh = scene->mMeshes[mesh_idx];

    uint32_t vertex_count = mesh->mNumVertices;
    uint32_t index_count  = 3 * mesh->mNumFaces;

    Submesh& submesh = result_mesh->GetSubmeshes().emplace_back(vertex_count, index_count);

    Vector<Vertex3D>& vertices = submesh.GetGeometry().GetVertices();
    Vector<uint32_t>& indices  = submesh.GetGeometry().GetIndices();

    for(uint32_t vertex_idx = 0; vertex_idx < vertex_count; ++vertex_idx) {
      Vertex3D& vertex = vertices[vertex_idx];
      
      vertex.position = glm::vec3{mesh->mVertices[vertex_idx].x,
                                  mesh->mVertices[vertex_idx].y,
                                  mesh->mVertices[vertex_idx].z};

      if (mesh->mTextureCoords[0]) {
        vertex.tex_coords = glm::vec2{mesh->mTextureCoords[0][vertex_idx].x, mesh->mTextureCoords[0][vertex_idx].y};
      }

      vertex.normal = glm::vec3{mesh->mNormals[vertex_idx].x,
                                mesh->mNormals[vertex_idx].y,
                                mesh->mNormals[vertex_idx].z};

      vertex.tangent = glm::vec3{mesh->mTangents[vertex_idx].x,
                                 mesh->mTangents[vertex_idx].y,
                                 mesh->mTangents[vertex_idx].z};

      vertex.bitangent = glm::vec3{mesh->mBitangents[vertex_idx].x,
                                   mesh->mBitangents[vertex_idx].y,
                                   mesh->mBitangents[vertex_idx].z};
    }

    for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
      aiFace face = mesh->mFaces[i];
      VULTURE_ASSERT(face.mNumIndices == 3, "Only triangle meshes are supported at the moment!");

      for (uint32_t j = 0; j < face.mNumIndices; ++j) {
        indices[3 * i + j] = face.mIndices[j];
      }
    }

    submesh.UpdateDeviceBuffers(device);
    submesh.SetMaterial(materials[mesh->mMaterialIndex]);
    

    // result_mesh->vertex_buffer = device->CreateStaticVertexBuffer<Vertex3D>(vertices.size());
    // device->LoadBufferData<Vertex3D>(result_mesh->vertex_buffer, 0, vertices.size(), vertices.data());

    // result_mesh->indices_count = indices.size();
    // result_mesh->index_buffer  = device->CreateStaticIndexBuffer(indices.size());
    // device->LoadBufferData<uint32_t>(result_mesh->index_buffer, 0, indices.size(), indices.data());
  }

  return result_mesh;
}

void LoadMaterials(RenderDevice& device, const aiScene* scene, Vector<SharedPtr<Material>>& materials) {
  materials.clear();

  AssetRegistry* asset_registry = AssetRegistry::Instance();

  SharedPtr<Sampler> default_sampler        = CreateShared<Sampler>(device, SamplerSpecification{});
  SharedPtr<Texture> default_texture        = asset_registry->Load<Texture>(".vulture/textures/blank.png");
  SharedPtr<Texture> default_texture_normal = asset_registry->Load<Texture>(".vulture/textures/blank_normal.png");

  SharedPtr<Shader> forward_shader = asset_registry->Load<Shader>(".vulture/shaders/BuiltIn.Forward.shader");

  LOG_DEBUG("Shader reflection ({}):", ".vulture/shaders/BuiltIn.Forward.shader");
  forward_shader->GetReflection().PrintData();  // FIXME: Debug only

  for (uint32_t material_idx = 0; material_idx < scene->mNumMaterials; ++material_idx) {
    SharedPtr<Material> material = CreateShared<Material>(device);
    material->AddShader(forward_shader);

    MaterialPass& material_pass = material->GetMaterialPass(forward_shader->GetTargetPassId());

    aiMaterial* assimp_material = scene->mMaterials[material_idx];
    
    /* Color values */
    aiColor4D ambient_color{0, 0, 0, 1};
    aiGetMaterialColor(assimp_material, AI_MATKEY_COLOR_AMBIENT, &ambient_color);
    material_pass.GetProperty<glm::vec3>("ambientColor") = {ambient_color.r, ambient_color.g, ambient_color.b};

    aiColor4D diffuse_color{0, 0, 0, 1};
    aiGetMaterialColor(assimp_material, AI_MATKEY_COLOR_DIFFUSE, &diffuse_color);
    material_pass.GetProperty<glm::vec3>("diffuseColor") = {diffuse_color.r, diffuse_color.g, diffuse_color.b};

    aiColor4D specular_color{0, 0, 0, 1};
    aiGetMaterialColor(assimp_material, AI_MATKEY_COLOR_SPECULAR, &specular_color);
    material_pass.GetProperty<glm::vec3>("specularColor") = {specular_color.r, specular_color.g, specular_color.b};

    /* Scalar values */
    float specular_exponent{1};
    aiGetMaterialFloat(assimp_material, AI_MATKEY_SPECULAR_FACTOR, &specular_exponent);
    material_pass.GetProperty<float>("specularExponent") = specular_exponent;

    float normal_strength{1.0};
    aiGetMaterialFloat(assimp_material, AI_MATKEY_BUMPSCALING, &normal_strength);
    material_pass.GetProperty<float>("normalStrength") = normal_strength;

    /* Texture maps */
    auto& diffuse_map = material_pass.GetTextureSampler("uDiffuseMap");
    diffuse_map.sampler = default_sampler;

    bool diffuse_map_found = false;
    aiString diffuse_map_path;
    if (assimp_material->GetTexture(aiTextureType_DIFFUSE, 0, &diffuse_map_path) == aiReturn_SUCCESS) {
      diffuse_map.texture = asset_registry->Load<Texture>(diffuse_map_path.C_Str());

      if (diffuse_map.texture) {
        diffuse_map_found = true;
      }
    }

    if (!diffuse_map_found) {
      diffuse_map.texture = default_texture;
    }

    auto& normal_map = material_pass.GetTextureSampler("uNormalMap");
    normal_map.sampler = default_sampler;

    bool normal_map_found = false;
    material_pass.GetProperty<uint32_t>("useNormalMap") = 1;  // FIXME: (tralf-strues) not always needed!
    aiString normal_map_path;
    if (assimp_material->GetTexture(aiTextureType_NORMALS, 0, &normal_map_path) == aiReturn_SUCCESS) {
      normal_map.texture = asset_registry->Load<Texture>(normal_map_path.C_Str());

      if (normal_map.texture) {
        normal_map_found = true;
      }
    }

    if (!normal_map_found) {
      normal_map.texture = default_texture_normal;
    }

    material->WriteMaterialPassDescriptors();
    materials.push_back(material);
  }
}

}  // namespace detail
}  // namespace vulture
