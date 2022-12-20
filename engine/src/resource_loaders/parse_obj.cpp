/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file parse_obj.cpp
 * @date 2022-05-01
 * 
 * The MIT License (MIT)
 * Copyright (c) vulture-project
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

#include "resource_loaders/parse_obj.hpp"

#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <string>

#include "core/logger.hpp"
#include "core/resource_manager.hpp"
#include "renderer/3d/3d_default_shader_names.hpp"

using namespace vulture;

struct PackedVertex {
  glm::vec3 position;
  glm::vec2 uv;
  glm::vec3 normal;
  glm::vec4 tangent{0}; ///< tangent.w = +-1, it shows the handedness of TBN

  bool operator<(const PackedVertex& other) const {
    return memcmp((const void*)this, (const void*)&other, sizeof(PackedVertex)) > 0;
  };
};

struct WavefrontMaterial {
  std::string name;

  glm::vec3 color_ambient;
  glm::vec3 color_diffuse;
  glm::vec3 color_specular;
  float specular_exponent{225.0};
  float transparency{0.0};

  std::optional<std::string> map_ambient{std::nullopt};
  std::optional<std::string> map_diffuse{std::nullopt};
  std::optional<std::string> map_specular{std::nullopt};

  std::optional<std::string> map_normal{std::nullopt};
  float normal_strength{1.0};
};

using MapVertexToIndex = std::map<PackedVertex, uint32_t>;

bool GetSimilarVertexIndex(const PackedVertex& packed, const MapVertexToIndex& map, uint32_t* result) {
  MapVertexToIndex::const_iterator it = map.find(packed);
  bool found = it != map.end();

  if (found) {
    *result = it->second;
  }

  return found;
}

glm::vec3 SafeNormalize(const glm::vec3& vector) {
  if (glm::length(vector) != 0.0) {
    return glm::normalize(vector);
  } else {
    return glm::vec3(0.0);
  }
}

SharedPtr<Mesh> vulture::ParseMeshWavefront(const std::string& filename) {
  struct Vertex {
    uint32_t index_position;
    uint32_t index_uv;
    uint32_t index_normal;
  };

  struct Face {
    SharedPtr<Material> material{nullptr};
    Vertex vertices[3];
  };

  LOG_INFO(Renderer, "Loading mesh from file \"{}\"", filename);

  SharedPtr<Material> default_material =
      CreateShared<Material>(ResourceManager::LoadShader("res/shaders/basic_normal_mapped.glsl"));
  default_material->SetUniform(glm::vec3{1.0}, "{}.{}", kUniformNameMaterial, kStructMemberNameAmbientColor);
  default_material->SetUniform(glm::vec3{0.8}, "{}.{}", kUniformNameMaterial, kStructMemberNameDiffuseColor);
  default_material->SetUniform(glm::vec3{0.5}, "{}.{}", kUniformNameMaterial, kStructMemberNameSpecularColor);
  default_material->SetUniform(225.0f, "{}.{}", kUniformNameMaterial, kStructMemberNameSpecularExponent);
  default_material->SetUniform(1, "{}.{}", kUniformNameMaterial, kStructMemberNameNormalStrength);
  default_material->SetUniform(false, "{}.{}", kUniformNameMaterial, kStructMemberNameUseNormalMap);
  default_material->AddTexture(ResourceManager::LoadTexture("res/textures/blank.png"), "{}.{}", kUniformNameMaterial,
                               kStructMemberNameDiffuseMap);

  std::vector<glm::vec3> positions;
  std::vector<glm::vec2> uvs;
  std::vector<glm::vec3> normals;
  std::vector<Face>      faces;

  std::ifstream stream(filename);
  if (stream.fail()) {
    LOG_ERROR(Renderer, "Failed to open file \"{}\"", filename);
    return nullptr;
  }

  static char name[128];

  bool material_specified = false;
  MapMaterials materials;
  SharedPtr<Material> cur_material{nullptr};

  std::string cur_line;
  while (std::getline(stream, cur_line)) {
    float values[3];

    if (std::sscanf(cur_line.c_str(), "v %f %f %f", &values[0], &values[1], &values[2]) == 3) {
      positions.emplace_back(values[0], values[1], values[2]);
    }
    else if (std::sscanf(cur_line.c_str(), "vt %f %f", &values[0], &values[1]) == 2) {
      uvs.emplace_back(values[0], values[1]);
    }
    else if (std::sscanf(cur_line.c_str(), "vn %f %f %f", &values[0], &values[1], &values[2]) == 3) {
      normals.emplace_back(values[0], values[1], values[2]);
    }
    else if (cur_line[0] == 'f' && cur_line[1] == ' ') {
      uint32_t indices[9];
      std::sscanf(cur_line.c_str(), "f %u/%u/%u %u/%u/%u %u/%u/%u", &indices[0], &indices[1], &indices[2],
                                                                    &indices[3], &indices[4], &indices[5],
                                                                    &indices[6], &indices[7], &indices[8]);

      faces.push_back(Face{cur_material, {Vertex{indices[0] - 1, indices[1] - 1, indices[2] - 1},
                                          Vertex{indices[3] - 1, indices[4] - 1, indices[5] - 1},
                                          Vertex{indices[6] - 1, indices[7] - 1, indices[8] - 1}}});
    }
    else if (std::sscanf(cur_line.c_str(), "usemtl %127s", name) == 1) {
      if (material_specified && materials.find(name) != materials.end()) {
        cur_material = materials[name];
      } else {
        LOG_WARN(Renderer, "No material \"{}\" found!", name);
        cur_material = default_material;
      }
    }
    else if (std::sscanf(cur_line.c_str(), "mtllib %127s", name) == 1) {
      if (ParseMaterialsWavefront(name, materials)) {
        material_specified = true;
      }
    }
  }

  MapVertexToIndex vertexToOutIndex;

  std::vector<PackedVertex> out_vertices;
  std::vector<std::vector<uint32_t>> out_indices{1};
  std::vector<SharedPtr<Material>> out_materials{faces[0].material};
  cur_material = out_materials[0];

  for (uint32_t i = 0; i < faces.size(); ++i) {
    for (uint32_t j = 0; j < 3; ++j) {
      if (cur_material != faces[i].material) {
        out_materials.push_back(faces[i].material);
        cur_material = faces[i].material;
        out_indices.push_back({});
      }

      PackedVertex packed;
      packed.position = positions[faces[i].vertices[j].index_position];
      packed.uv       = uvs      [faces[i].vertices[j].index_uv      ];
      packed.normal   = normals  [faces[i].vertices[j].index_normal  ];

      uint32_t index = 0;
      bool found = GetSimilarVertexIndex(packed, vertexToOutIndex, &index);

      if (found) {
        out_indices[out_indices.size() - 1].push_back(index);
      } else {
        out_vertices.push_back(packed);
        out_indices[out_indices.size() - 1].push_back(out_vertices.size() - 1);
        vertexToOutIndex[packed] = out_vertices.size() - 1;
      }
    }
  }

  /* Calculating tangents */
  std::vector<glm::vec3> out_bitangents{out_vertices.size()}; // Used just for correction, not loaded to a vbo!

  for (uint32_t submesh_idx = 0; submesh_idx < out_indices.size(); ++submesh_idx) {
    for (uint32_t i = 0; i < out_indices[submesh_idx].size(); i += 3) {
      PackedVertex* vertices[3] = {&out_vertices[out_indices[submesh_idx][i + 0]],
                                   &out_vertices[out_indices[submesh_idx][i + 1]],
                                   &out_vertices[out_indices[submesh_idx][i + 2]]};
      
      glm::vec3 ms_edge0 = vertices[1]->position - vertices[0]->position;
      glm::vec3 ms_edge1 = vertices[2]->position - vertices[1]->position;

      glm::vec2 uv_edge0 = vertices[1]->uv - vertices[0]->uv;
      glm::vec2 uv_edge1 = vertices[2]->uv - vertices[1]->uv;

      float det = (uv_edge0.x * uv_edge1.y - uv_edge1.x * uv_edge0.y);
      float inverse_det = 0;

      if (det == 0.0f) {
        uv_edge0 = glm::vec2(0, 1);
        uv_edge1 = glm::vec2(1, 0);
        inverse_det = 1;
      } else {
        inverse_det = 1 / det;
      }

      glm::vec3 tangent;
      tangent.x = inverse_det * (uv_edge1.y * ms_edge0.x - uv_edge0.y * ms_edge1.x);
      tangent.y = inverse_det * (uv_edge1.y * ms_edge0.y - uv_edge0.y * ms_edge1.y);
      tangent.z = inverse_det * (uv_edge1.y * ms_edge0.z - uv_edge0.y * ms_edge1.z);

      glm::vec3 bitangent;
      bitangent.x = inverse_det * (-uv_edge1.x * ms_edge0.x + uv_edge0.x * ms_edge1.x);
      bitangent.y = inverse_det * (-uv_edge1.x * ms_edge0.y + uv_edge0.x * ms_edge1.y);
      bitangent.z = inverse_det * (-uv_edge1.x * ms_edge0.z + uv_edge0.x * ms_edge1.z);

      // Averaging tangents and bitangents between adjacent faces
      vertices[0]->tangent += glm::vec4(tangent, 0);
      vertices[1]->tangent += glm::vec4(tangent, 0);
      vertices[2]->tangent += glm::vec4(tangent, 0);

      out_bitangents[out_indices[submesh_idx][i + 0]] += bitangent;
      out_bitangents[out_indices[submesh_idx][i + 1]] += bitangent;
      out_bitangents[out_indices[submesh_idx][i + 2]] += bitangent;
    }
  }

  for (uint32_t i = 0; i < out_vertices.size(); ++i) {
    glm::vec3 normal    = SafeNormalize(out_vertices[i].normal);
    glm::vec3 tangent   = SafeNormalize(out_vertices[i].tangent);
    glm::vec3 bitangent = SafeNormalize(out_bitangents[i]);

    bool tangent_invalid = glm::isnan(tangent.x) || glm::isnan(tangent.y) ||
                           glm::isnan(tangent.z) || glm::isinf(tangent.x) ||
                           glm::isinf(tangent.y) || glm::isinf(tangent.z);

    bool bitangent_invalid = glm::isnan(bitangent.x) || glm::isnan(bitangent.y) ||
                             glm::isnan(bitangent.z) || glm::isinf(bitangent.x) ||
                             glm::isinf(bitangent.y) || glm::isinf(bitangent.z);

    if (tangent_invalid) {
      tangent = glm::cross(normal, bitangent);
    } else if (bitangent_invalid) {
      bitangent = glm::cross(normal, glm::vec3(tangent));
    }

    tangent   = SafeNormalize(tangent);
    bitangent = SafeNormalize(bitangent);

    float handedness =(glm::dot(glm::cross(normal, tangent), bitangent) < 0.0f) ? -1.0f : 1.0f;

    // Gram-Schmidt proccess to make TBN orthonormal
    tangent = SafeNormalize(tangent - glm::dot(normal, tangent) * normal);

    out_vertices[i].normal  = normal;
    out_vertices[i].tangent = glm::vec4(tangent, handedness);

    assert(!glm::isnan(tangent.x));
    assert(!glm::isnan(tangent.y));
    assert(!glm::isnan(tangent.z));
    assert(!glm::isinf(tangent.x));
    assert(!glm::isinf(tangent.y));
    assert(!glm::isinf(tangent.z));
  }

  /* Creating Mesh object */
  SharedPtr<Mesh> mesh = CreateShared<Mesh>();

  SharedPtr<VertexBuffer> vbo{VertexBuffer::Create(out_vertices.data(), out_vertices.size() * sizeof(out_vertices[0]))};
  vbo->SetLayout(VertexBufferLayout{{BufferDataType::kFloat3, kAttribNameMSPosition},
                                    {BufferDataType::kFloat2, kAttribNameUV},
                                    {BufferDataType::kFloat3, kAttribNameMSNormal},
                                    {BufferDataType::kFloat4, kAttribNameMSTangent}});

  for (uint32_t submesh_idx = 0; submesh_idx < out_indices.size(); ++submesh_idx) {
    SharedPtr<IndexBuffer> ibo{IndexBuffer::Create(out_indices[submesh_idx].data(), out_indices[submesh_idx].size())};

    SharedPtr<VertexArray> vao{VertexArray::Create()};
    vao->AddVertexBuffer(vbo);
    vao->SetIndexBuffer(ibo);

    mesh->GetSubmeshes().emplace_back(vao, out_materials[submesh_idx]);
  }

  LOG_INFO(Renderer,
           "Successfully loaded mesh from file \"{}\" (submeshes={}, positions={}, uvs={}, normals={}, faces={}, "
           "out_vertices={})",
           filename, mesh->GetSubmeshes().size(), positions.size(), uvs.size(), normals.size(), faces.size(),
           out_vertices.size());

  return mesh;
}

bool vulture::ParseMaterialsWavefront(const std::string& filename, MapMaterials& materials_map) {
  LOG_INFO(Renderer, "Loading materials from file \"{}\"", filename);

  std::ifstream stream(filename);
  if (stream.fail()) {
    LOG_ERROR(Renderer, "Failed to open file \"{}\"", filename);
    return false;
  }

  std::vector<WavefrontMaterial> materials;
  uint32_t materials_count = 0;

  std::string cur_line;
  while (std::getline(stream, cur_line)) {
    glm::vec3 value3f{0};
    float value1f = 0;
    static char name[128];

    if (std::sscanf(cur_line.c_str(), "newmtl %127s", name) == 1) {
      materials.push_back(WavefrontMaterial{});
      ++materials_count;
      materials[materials_count - 1].name = name;
    }
    else if (std::sscanf(cur_line.c_str(), "Ka %f %f %f", &value3f[0], &value3f[1], &value3f[2]) == 3) {
      materials[materials_count - 1].color_ambient = value3f;
    }
    else if (std::sscanf(cur_line.c_str(), "Kd %f %f %f", &value3f[0], &value3f[1], &value3f[2]) == 3) {
      materials[materials_count - 1].color_diffuse = value3f;
    }
    else if (std::sscanf(cur_line.c_str(), "Ks %f %f %f", &value3f[0], &value3f[1], &value3f[2]) == 3) {
      materials[materials_count - 1].color_specular = value3f;
    }
    else if (std::sscanf(cur_line.c_str(), "Ns %f", &value1f) == 1) {
      materials[materials_count - 1].specular_exponent = value1f;
    }
    else if (std::sscanf(cur_line.c_str(), "map_Kd %127s", name) == 1) {
      materials[materials_count - 1].map_diffuse = name;
    }
    else if (std::sscanf(cur_line.c_str(), "map_Bump -bm %f %127s", &value1f, name) == 2) {
      materials[materials_count - 1].map_normal = name;
      materials[materials_count - 1].normal_strength = value1f;
    }
  }

  for (uint32_t i = 0; i < materials_count; ++i) {
    SharedPtr<Shader> shader = ResourceManager::LoadShader("res/shaders/deferred_geometry.glsl");
    SharedPtr<Material> material = CreateShared<Material>(shader);

    shader->enable_blending_ = false;

    if (materials[i].map_normal.has_value()) {
      material->AddTexture(ResourceManager::LoadTexture(materials[i].map_normal.value()), "{}.{}", kUniformNameMaterial,
                           kStructMemberNameNormalMap);
      material->SetUniform(true, "{}.{}", kUniformNameMaterial, kStructMemberNameUseNormalMap);
      material->SetUniform(materials[i].normal_strength, "{}.{}", kUniformNameMaterial,
                           kStructMemberNameNormalStrength);
    } else {
      material->SetUniform(false, "{}.{}", kUniformNameMaterial, kStructMemberNameUseNormalMap);
    }

    if (materials[i].map_diffuse.has_value()) {
      material->AddTexture(ResourceManager::LoadTexture(*materials[i].map_diffuse), "{}.{}", kUniformNameMaterial,
                           kStructMemberNameDiffuseMap);
    } else {
      material->AddTexture(ResourceManager::LoadTexture("res/textures/blank.png"), "{}.{}", kUniformNameMaterial,
                           kStructMemberNameDiffuseMap);
    }

    material->SetUniform(materials[i].color_ambient, "{}.{}", kUniformNameMaterial, kStructMemberNameAmbientColor);
    material->SetUniform(materials[i].color_diffuse, "{}.{}", kUniformNameMaterial, kStructMemberNameDiffuseColor);
    material->SetUniform(materials[i].color_specular.r, "{}.{}", kUniformNameMaterial, "specular");
    material->SetUniform(materials[i].specular_exponent, "{}.{}", kUniformNameMaterial,
                         kStructMemberNameSpecularExponent);

    materials_map[materials[i].name] = material;
  }

  LOG_INFO(Renderer, "Successfully loaded materials from file \"{}\"", filename);

  return true;
}