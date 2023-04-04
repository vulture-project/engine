/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file skybox_loader.cpp
 * @date 2023-04-04
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

#include <stb_image/stb_image.h>
#include <yaml-cpp/yaml.h>

#include <vulture/asset/detail/asset_loader_registrar.hpp>
#include <vulture/asset/detail/texture_loader.hpp>
#include <vulture/asset/loaders/skybox_loader.hpp>
#include <vulture/renderer/geometry/mesh.hpp>

namespace vulture {

SkyboxLoader::SkyboxLoader(RenderDevice& device) : device_(device) {}

StringView SkyboxLoader::Extension() const {
  return StringView{".skybox"};
}

bool ParseSkyboxSide(const YAML::Node& root, Array<String, 6>& filenames, const StringView side_str,
                     CubeMapFaceLayer side, const StringView path) {
  YAML::Node side_node = root[side_str.data()];
  if (!side_node) {
    LOG_ERROR("Skybox file {0} does not contain \"{1}\" section!", path, side_str);
    return false;
  }

  filenames[static_cast<uint32_t>(side)] = side_node.as<String>();
  return true;
}

stbi_uc* LoadTexturePixels(const String& path, int32_t& width, int32_t& height, int32_t& channels) {
  stbi_set_flip_vertically_on_load(false);
  stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
  if (pixels == nullptr) {
    LOG_ERROR("Texture file \"{0}\" not found!", path);
    return nullptr;
  }

  return pixels;
}

SharedPtr<IAsset> SkyboxLoader::Load(const String& path) {
  AssetRegistry* asset_registry = AssetRegistry::Instance();

  SharedPtr<Shader> shader = asset_registry->Load<Shader>(".vulture/shaders/BuiltIn.Skybox.shader");
  LOG_DEBUG("Shader reflection ({}):", ".vulture/shaders/BuiltIn.Skybox.shader");
  shader->GetReflection().PrintData();  // FIXME: Debug only

  SharedPtr<Material> material = CreateShared<Material>(device_);
  material->AddShader(shader);

  SharedPtr<Mesh> mesh = CreateShared<Mesh>(device_, Geometry::CreateCube(), material);

  YAML::Node root = YAML::LoadFile(path.data());

  Array<String, 6> filenames;

  if (!ParseSkyboxSide(root, filenames, "Right", CubeMapFaceLayer::kRightPositiveX, path)) {
    return nullptr;
  }

  if (!ParseSkyboxSide(root, filenames, "Left", CubeMapFaceLayer::kLeftNegativeX, path)) {
    return nullptr;
  }

  if (!ParseSkyboxSide(root, filenames, "Top", CubeMapFaceLayer::kTopPositiveY, path)) {
    return nullptr;
  }

  if (!ParseSkyboxSide(root, filenames, "Bottom", CubeMapFaceLayer::kBottomNegativeY, path)) {
    return nullptr;
  }

  if (!ParseSkyboxSide(root, filenames, "Front", CubeMapFaceLayer::kFrontPositiveZ, path)) {
    return nullptr;
  }

  if (!ParseSkyboxSide(root, filenames, "Back", CubeMapFaceLayer::kBackNegativeZ, path)) {
    return nullptr;
  }

  int32_t width    = 0;
  int32_t height   = 0;
  int32_t channels = 0;

  Array<stbi_uc*, 6> side_pixels;

  for (uint32_t i = 0; i < 6; ++i) {
    side_pixels[i] = LoadTexturePixels(filenames[i], width, height, channels);
    if (side_pixels[i] == nullptr) {
      return nullptr;
    }
  }

  TextureSpecification tex_specification{};
  tex_specification.format     = DataFormat::kR8G8B8A8_UNORM;
  tex_specification.usage      = kTextureUsageBitSampled;
  tex_specification.type       = TextureType::kTextureCube;
  tex_specification.width      = static_cast<uint32_t>(width);
  tex_specification.height     = static_cast<uint32_t>(height);
  TextureHandle handle = device_.CreateTexture(tex_specification);

  void* map_data = nullptr;

  uint32_t layer_size  = width * height * 4;
  uint32_t buffer_size = layer_size * 6;
  BufferHandle staging_buffer = device_.CreateBuffer(buffer_size, kBufferUsageBitTransferSrc, true, &map_data);

  device_.InvalidateBufferMemory(staging_buffer, 0, buffer_size);
  for (uint32_t i = 0; i < 6; ++i) {
    std::memcpy(static_cast<stbi_uc*>(map_data) + i * layer_size, side_pixels[i], layer_size);
  }
  device_.FlushBufferMemory(staging_buffer, 0, buffer_size);

  CommandBuffer* command_buffer = device_.CreateCommandBuffer(CommandBufferType::kGraphics, true);
  command_buffer->Begin();

  command_buffer->TransitionLayout(handle, TextureLayout::kUndefined, TextureLayout::kTransferDst);
  command_buffer->CopyBufferToTexture(staging_buffer, handle, width, height, 0, 6);
  command_buffer->GenerateMipmaps(handle, TextureLayout::kShaderReadOnly);

  command_buffer->End();
  command_buffer->Submit();

  for (uint32_t i = 0; i < 6; ++i) {
    stbi_image_free(side_pixels[i]);
  }

  SharedPtr<Texture> texture = CreateShared<Texture>(device_, handle);
  SharedPtr<Sampler> sampler = CreateShared<Sampler>(device_, SamplerSpecification{});

  MaterialPass& material_pass = material->GetMaterialPass(shader->GetTargetPassId());
  auto& property = material_pass.GetTextureSampler("uSkybox");
  property.texture = texture;
  property.sampler = sampler;

  material->WriteMaterialPassDescriptors();

  return mesh;
}

}  // namespace vulture