/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file mesh.cpp
 * @date 2022-05-14
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

#include "asset/asset_registry.hpp"
#include "renderer/3d/mesh.hpp"
#include "renderer/3d/3d_default_shader_names.hpp"

using namespace vulture;

static constexpr const float kSkyboxVertices[] = {
  -1, -1, -1,
   1, -1, -1,
   1,  1, -1,
  -1,  1, -1,
  -1, -1,  1,
   1, -1,  1,
   1,  1,  1,
  -1,  1,  1
};

static constexpr const uint32_t kSkyboxIndices[] = {
  0, 1, 3,
  3, 1, 2,

  1, 5, 2,
  2, 5, 6,

  5, 4, 6,
  6, 4, 7,

  4, 0, 7,
  7, 0, 3,

  3, 2, 7,
  7, 2, 6,

  4, 5, 0,
  0, 5, 1
};

const float kQuadVertices[] = {
  // positions   // uvs
  -1.0f,  1.0f,  0.0f, 1.0f, // top-left
  -1.0f, -1.0f,  0.0f, 0.0f, // bottom-left
   1.0f, -1.0f,  1.0f, 0.0f, // bottom-right
   1.0f,  1.0f,  1.0f, 1.0f  // top-right
};

const uint32_t kQuadIndices[] = {
  0, 1, 3,
  3, 1, 2
};

SharedPtr<Mesh> vulture::CreateSkyboxMesh(const std::array<std::string, 6>& faces_filenames) {
  SharedPtr<VertexBuffer> vbo{VertexBuffer::Create(kSkyboxVertices, sizeof(kSkyboxVertices))};
  vbo->SetLayout(VertexBufferLayout{{BufferDataType::kFloat3, kAttribNameMSPosition}});

  SharedPtr<IndexBuffer> ibo{IndexBuffer::Create(kSkyboxIndices, sizeof(kSkyboxIndices) / sizeof(kSkyboxIndices[0]))};

  SharedPtr<VertexArray> vao{VertexArray::Create()};
  vao->AddVertexBuffer(vbo);
  vao->SetIndexBuffer(ibo);

  // SharedPtr<Material> material = CreateShared<Material>(Shader::Create("res/shaders/skybox.glsl"));
  SharedPtr<Shader> shader  = AssetRegistry::Instance()->Load<Shader>("res/shaders/skybox.glsl");
  shader->geometry_pass_    = GeometryPass::kForward;
  shader->cull_mode_        = CullMode::kNone;
  shader->depth_compare_op_ = CompareOperation::kLessOrEqual;

  SharedPtr<Material> material = CreateShared<Material>(shader);
  material->AddCubeMap(CubeMap::Create(faces_filenames), "u_skybox");

  return CreateShared<Mesh>(vao, material);
}

SharedPtr<VertexArray> vulture::CreateQuad() {
  SharedPtr<VertexBuffer> vbo{VertexBuffer::Create(kQuadVertices, sizeof(kQuadVertices))};
  vbo->SetLayout(VertexBufferLayout{{BufferDataType::kFloat2, kAttribNameMSPosition},
                                    {BufferDataType::kFloat2, kAttribNameUV}});

  SharedPtr<IndexBuffer> ibo{IndexBuffer::Create(kQuadIndices, sizeof(kQuadIndices) / sizeof(kQuadIndices[0]))};

  SharedPtr<VertexArray> vao{VertexArray::Create()};
  vao->AddVertexBuffer(vbo);
  vao->SetIndexBuffer(ibo);

  AttributeLocationMap location_map;
  location_map[kAttribNameMSPosition] = 0;
  location_map[kAttribNameUV] = 1;
  vao->SetAttributeLocations(location_map);

  return vao;
}