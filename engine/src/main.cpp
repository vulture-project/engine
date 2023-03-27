/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file main.cpp
 * @date 2023-03-22
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

#include <vulture/asset/asset_registry.hpp>
#include <vulture/asset/loaders/jpg_loader.hpp>
#include <vulture/asset/loaders/obj_loader.hpp>
#include <vulture/asset/loaders/png_loader.hpp>
#include <vulture/asset/loaders/shader_loader.hpp>
#include <vulture/asset/loaders/tga_loader.hpp>
#include <vulture/event_system/event_system.hpp>
#include <vulture/platform/event.hpp>
#include <vulture/renderer/builtin/forward_pass.hpp>
#include <vulture/renderer/builtin/renderer.hpp>
#include <vulture/renderer/light.hpp>
#include <vulture/scene/scene.hpp>
#include <vulture/core/core.hpp>
#include <vector>
#include <fstream>
#include <string>
#include <iostream>

using namespace vulture;

int main() {
  Logger::SetTraceEnabled(true);

  Window window{900, 600, "Vulture"};
  LOG_INFO("Window size = ({}, {}), framebuffer size = ({}, {})", window.GetWidth(), window.GetHeight(),
           window.GetFramebufferWidth(), window.GetFramebufferHeight());

  Dispatcher event_dispatcher;
  InputEventManager::SetWindowAndDispatcher(&window, &event_dispatcher);

  RenderDevice& device = *RenderDevice::Create(RenderDevice::DeviceFamily::kVulkan);
  device.Init(&window, nullptr, nullptr, true);

  /* Register loaders */
  AssetRegistry::Instance()->RegisterLoader(CreateShared<OBJLoader>(device));
  AssetRegistry::Instance()->RegisterLoader(CreateShared<TGALoader>(device));
  AssetRegistry::Instance()->RegisterLoader(CreateShared<JPGLoader>(device));
  AssetRegistry::Instance()->RegisterLoader(CreateShared<PNGLoader>(device));
  AssetRegistry::Instance()->RegisterLoader(CreateShared<ShaderLoader>(device));

  /* Swapchain Spec */
  SwapchainHandle swapchain = device.CreateSwapchain(kTextureUsageBitTransferDst);
  uint32_t swapchain_size{0};
  device.GetSwapchainTextures(swapchain, &swapchain_size, nullptr);

  Vector<TextureHandle> swapchain_textures;
  swapchain_textures.resize(swapchain_size);
  device.GetSwapchainTextures(swapchain, nullptr, swapchain_textures.data());

  const TextureSpecification& swapchain_texture_spec = device.GetTextureSpecification(swapchain_textures[0]);
  uint32_t   surface_width  = swapchain_texture_spec.width;
  uint32_t   surface_height = swapchain_texture_spec.height;
  DataFormat surface_format = swapchain_texture_spec.format;

  TextureSpecification color_output_spec{};
  color_output_spec.format = surface_format;
  color_output_spec.usage  = kTextureUsageBitColorAttachment;
  color_output_spec.width  = surface_width;
  color_output_spec.height = surface_height;
  SharedPtr<Texture> color_output = CreateShared<Texture>(device, color_output_spec);

  /* Command Buffer */
  CommandBuffer& command_buffer = *device.CreateCommandBuffer(CommandBufferType::kGraphics);

  /* Render Graph */
  auto render_graph = CreateUnique<rg::RenderGraph>();
  ColorOutput& color_output_data = render_graph->GetBlackboard().Add<ColorOutput>();
  color_output_data.texture_id =
      render_graph->ImportTexture("color", color_output, color_output_spec, TextureLayout::kTransferSrc);

  render_graph->AddPass<ForwardPass>(ForwardPass::GetName());

  render_graph->Setup();
  render_graph->Compile(device);

  std::ofstream output_file("log/render_graph.dot", std::ios::trunc);
  assert(output_file.is_open());
  render_graph->ExportGraphviz(output_file);
  system("dot -Tpng log/render_graph.dot > log/render_graph.png");

  /* Renderer */
  Renderer renderer{device, std::move(render_graph)};

  /* Scene */
  Scene scene;

  fennecs::EntityHandle camera = scene.CreateEntity("Editor camera");
  camera = scene.GetEntityWorld().Attach<CameraComponent>(camera, PerspectiveCameraSpecs((float)surface_width / (float)surface_height), true);
  camera = scene.GetEntityWorld().Attach<TransformComponent>(camera, glm::vec3(0, 3, 15));
  // camera = scene.GetEntityWorld().Attach<ScriptComponent>(camera, new CameraMovementScript());

  fennecs::EntityHandle sponza = scene.CreateEntity("Sponza");
  sponza = scene.GetEntityWorld().Attach<MeshComponent>(sponza, AssetRegistry::Instance()->Load<Mesh>("meshes/sponza.obj"));
  sponza = scene.GetEntityWorld().Attach<TransformComponent>(sponza);
  sponza.Get<TransformComponent>().transform.scale = glm::vec3(0.025);
  sponza.Get<TransformComponent>().transform.Rotate(M_PI_2, glm::vec3{0, 1, 0});

  fennecs::EntityHandle dir_light = scene.CreateEntity("Directional light");
  dir_light = scene.GetEntityWorld().Attach<DirectionalLightSpecification>(dir_light, glm::vec3{1, 1, 0.5});
  dir_light = scene.GetEntityWorld().Attach<TransformComponent>(dir_light, Transform(glm::vec3(0), glm::vec3(-0.5, 0, 0)));

  /* Render Loop */
  Timer timer;
  float time_start = timer.Elapsed();
  while (true) {
    float timestep = timer.Elapsed() - time_start;
    time_start = timer.Elapsed();

    InputEventManager::TriggerEvents();

    {
      ScopedTimer trace_timer{"scene.OnUpdate()"};
      scene.OnUpdate(timestep);
    }

    uint32_t texture_idx = 0;
    {
      ScopedTimer trace_timer{"FrameBegin()"};
      device.FrameBegin(swapchain, &texture_idx);
    }

    {
      ScopedTimer trace_timer{"scene.Render()"};
      scene.Render(renderer, timer.Elapsed());
    }

    {
      ScopedTimer trace_timer{"layout transition()"};

      command_buffer.Reset();
      command_buffer.Begin();

      command_buffer.TransitionLayout(swapchain_textures[texture_idx], TextureLayout::kUndefined, TextureLayout::kTransferDst);
      command_buffer.CopyTexture(color_output->GetHandle(), swapchain_textures[texture_idx], surface_width, surface_height);
      command_buffer.TransitionLayout(swapchain_textures[texture_idx], TextureLayout::kTransferDst, TextureLayout::kPresentSrc);

      command_buffer.End();
      command_buffer.Submit();
    }

    {
      ScopedTimer trace_timer{"FrameEnd()"};
      device.FrameEnd(swapchain);
    }

    {
      ScopedTimer trace_timer{"Present()"};
      device.Present(swapchain);
    }

    {
      ScopedTimer trace_timer{"window.SetFPSToTitle()"};
      window.SetFPSToTitle(1 / timestep);
    }
  }

  return 0;
}