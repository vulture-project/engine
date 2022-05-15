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

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <cstring>

#include "core/logger.hpp"
#include "resource_loaders/parse_obj.hpp"
#include "renderer/3d/3d_default_shader_names.hpp"

using namespace vulture;

struct PackedVertex {
  glm::vec3 position;
  glm::vec2 uv;
  glm::vec3 normal;

  bool operator<(const PackedVertex& other) const {
    return memcmp((const void*)this, (const void*)&other, sizeof(PackedVertex)) > 0;
  };
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

SharedPtr<Mesh> vulture::ParseMeshWavefront(const std::string& filename) {
  struct Vertex {
    uint32_t index_position;
    uint32_t index_uv;
    uint32_t index_normal;
  };

  struct Face {
    Vertex vertices[3];
  };

  LOG_INFO(Renderer, "Loading mesh from file \"{}\"", filename);

  std::vector<glm::vec3> positions;
  std::vector<glm::vec2> uvs;
  std::vector<glm::vec3> normals;
  std::vector<Face>      faces;

  std::ifstream stream(filename);
  if (stream.fail()) {
    LOG_ERROR(Renderer, "Failed to open file \"{}\"", filename);
    return nullptr;
  }

  static char material_filename[128];
  bool material_specified = false;

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

      faces.push_back(Face{{Vertex{indices[0] - 1, indices[1] - 1, indices[2] - 1},
                            Vertex{indices[3] - 1, indices[4] - 1, indices[5] - 1},
                            Vertex{indices[6] - 1, indices[7] - 1, indices[8] - 1}}});
    }
    else if (std::sscanf(cur_line.c_str(), "mtllib %s", material_filename) == 1) {
      material_specified = true;
    }
  }

  MapVertexToIndex vertexToOutIndex;

  std::vector<PackedVertex> out_vertices;
  std::vector<uint32_t> out_indices;

  for (uint32_t i = 0; i < faces.size(); ++i) {
    for (uint32_t j = 0; j < 3; ++j) {
      PackedVertex packed;
      packed.position = positions[faces[i].vertices[j].index_position];
      packed.uv       = uvs      [faces[i].vertices[j].index_uv      ];
      packed.normal   = normals  [faces[i].vertices[j].index_normal  ];

      uint32_t index = 0;
      bool found = GetSimilarVertexIndex(packed, vertexToOutIndex, &index);

      if (found) {
        out_indices.push_back(index);
      } else {
        out_vertices.push_back(packed);
        out_indices.push_back(out_vertices.size() - 1);
        vertexToOutIndex[packed] = out_vertices.size() - 1;
      }
    }
  }

  SharedPtr<VertexBuffer> vbo{VertexBuffer::Create(out_vertices.data(), out_vertices.size() * sizeof(out_vertices[0]))};
  vbo->SetLayout(VertexBufferLayout{{BufferDataType::kFloat3, kAttribNameMSPosition},
                                    {BufferDataType::kFloat2, kAttribNameUV},
                                    {BufferDataType::kFloat3, kAttribNameMSNormal}});

  SharedPtr<IndexBuffer> ibo{IndexBuffer::Create(out_indices.data(), out_indices.size())};

  SharedPtr<VertexArray> vao{VertexArray::Create()};
  vao->AddVertexBuffer(vbo);
  vao->SetIndexBuffer(ibo);

  LOG_INFO(Renderer,
           "Successfully loaded mesh from file \"{}\" (position={}, uvs={}, normals={}, faces={} | out_vertices={}, "
           "out_indices={})",
           filename, positions.size(), uvs.size(), normals.size(), faces.size(), out_vertices.size(),
           out_indices.size());

  SharedPtr<Material> material = nullptr;

  if (!material_specified) {
    LOG_WARN(Renderer, "Wavefront mesh file \"{}\" doesn't specify material info, using default material!", filename);

    material = CreateShared<Material>(Shader::Create("res/shaders/basic.glsl"));
    material->SetUniform(glm::vec3{1.0}, "{}.{}", kUniformNameMaterial, kStructMemberNameAmbientColor);
    material->SetUniform(glm::vec3{0.8}, "{}.{}", kUniformNameMaterial, kStructMemberNameDiffuseColor);
    material->SetUniform(glm::vec3{0.5}, "{}.{}", kUniformNameMaterial, kStructMemberNameSpecularColor);
    material->SetUniform(225.0f, "{}.{}", kUniformNameMaterial, kStructMemberNameSpecularExponent);
    material->AddTexture(Texture::Create("res/textures/blank.png"), "{}.{}", kUniformNameMaterial,
                         kStructMemberNameDiffuseMap);
  } else {
    material = ParseMaterialWavefront(material_filename);
  }

  return CreateShared<Mesh>(vao, material);
}

SharedPtr<Material> vulture::ParseMaterialWavefront(const std::string& filename) {
  LOG_INFO(Renderer, "Loading material from file \"{}\"", filename);

  std::ifstream stream(filename);
  if (stream.fail()) {
    LOG_ERROR(Renderer, "Failed to open file \"{}\"", filename);
    return nullptr;
  }

  SharedPtr<Material> material{CreateShared<Material>(Shader::Create("res/shaders/basic.glsl"))};

  bool diffuse_map_found = false;

  std::string cur_line;
  while (std::getline(stream, cur_line)) {
    glm::vec3 value3f{0};
    float value1f = 0;

    static char name[128];

    if (std::sscanf(cur_line.c_str(), "Ka %f %f %f", &value3f[0], &value3f[1], &value3f[2]) == 3) {
      material->SetUniform(value3f, "{}.{}", kUniformNameMaterial, kStructMemberNameAmbientColor);
    }
    else if (std::sscanf(cur_line.c_str(), "Kd %f %f %f", &value3f[0], &value3f[1], &value3f[2]) == 3) {
      material->SetUniform(value3f, "{}.{}", kUniformNameMaterial, kStructMemberNameDiffuseColor);
    }
    else if (std::sscanf(cur_line.c_str(), "Ks %f %f %f", &value3f[0], &value3f[1], &value3f[2]) == 3) {
      material->SetUniform(value3f, "{}.{}", kUniformNameMaterial, kStructMemberNameSpecularColor);
    }
    else if (std::sscanf(cur_line.c_str(), "Ns %f", &value1f) == 1) {
      material->SetUniform(value3f, "{}.{}", kUniformNameMaterial, kStructMemberNameSpecularExponent);
    }
    else if (std::sscanf(cur_line.c_str(), "map_Kd %s", name) == 1) {
      material->AddTexture(Texture::Create(name), "{}.{}", kUniformNameMaterial, kStructMemberNameDiffuseMap);
      diffuse_map_found = true;
    }
  }

  if (!diffuse_map_found) {
    material->AddTexture(Texture::Create("res/textures/blank.png"), "{}.{}", kUniformNameMaterial,
                         kStructMemberNameDiffuseMap);
  }

  LOG_INFO(Renderer, "Successfully loaded material from file \"{}\"", filename);

  return material;
}