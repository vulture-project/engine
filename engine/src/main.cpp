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

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <vulture/asset/asset_registry.hpp>
#include <vulture/asset/loaders/fbx_loader.hpp>
#include <vulture/asset/loaders/jpg_loader.hpp>
#include <vulture/asset/loaders/obj_loader.hpp>
#include <vulture/asset/loaders/png_loader.hpp>
#include <vulture/asset/loaders/shader_loader.hpp>
#include <vulture/asset/loaders/tga_loader.hpp>
#include <vulture/core/core.hpp>
#include <vulture/event_system/event_system.hpp>
#include <vulture/platform/event.hpp>
#include <vulture/renderer/builtin/forward_pass.hpp>
#include <vulture/renderer/builtin/renderer.hpp>
#include <vulture/renderer/light.hpp>
#include <vulture/scene/scene.hpp>

#include "camera_movement.hpp"

using namespace vulture;

bool running = true;

void OnQuit(const QuitEvent&) {
  running = false;
}

int main() {
  Logger::SetTraceEnabled(false);

  Window window{1600, 900, "Vulture"};
  LOG_INFO("Window size = ({}, {}), framebuffer size = ({}, {})", window.GetWidth(), window.GetHeight(),
           window.GetFramebufferWidth(), window.GetFramebufferHeight());

  Dispatcher event_dispatcher;
  InputEventManager::SetWindowAndDispatcher(&window, &event_dispatcher);
  event_dispatcher.GetSink<QuitEvent>().Connect<&OnQuit>();

  RenderDevice& device = *RenderDevice::Create(RenderDevice::DeviceFamily::kVulkan);
  device.Init(&window, nullptr, nullptr, true);

  /* Register loaders */
  AssetRegistry::Instance()->RegisterLoader(CreateShared<OBJLoader>(device));
  AssetRegistry::Instance()->RegisterLoader(CreateShared<FBXLoader>(device));
  AssetRegistry::Instance()->RegisterLoader(CreateShared<TGALoader>(device));
  AssetRegistry::Instance()->RegisterLoader(CreateShared<JPGLoader>(device));
  AssetRegistry::Instance()->RegisterLoader(CreateShared<PNGLoader>(device));
  AssetRegistry::Instance()->RegisterLoader(CreateShared<ShaderLoader>(device));

  /* Swapchain Spec */
  SwapchainHandle swapchain = device.CreateSwapchain(kTextureUsageBitTransferDst);
  uint32_t swapchain_size{0};
  device.GetSwapchainTextures(swapchain, &swapchain_size, nullptr);

  LOG_INFO("Swapchain size is {0}", swapchain_size);

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
  Array<CommandBuffer*, kFramesInFlight> command_buffers{nullptr};
  for (auto& command_buffer : command_buffers) {
    command_buffer = device.CreateCommandBuffer(CommandBufferType::kGraphics);
  }

  /* Sync primitives for frames in flight */
  Array<FenceHandle, kFramesInFlight> fences_in_flight{kInvalidRenderResourceHandle};
  for (auto& fence : fences_in_flight) {
    fence = device.CreateFence();
  }

  Array<SemaphoreHandle, kFramesInFlight> semaphores_render_finished{kInvalidRenderResourceHandle};
  for (auto& semaphore : semaphores_render_finished) {
    semaphore = device.CreateSemaphore();
  }

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
  camera = scene.GetEntityWorld().Attach<ScriptComponent>(camera, new CameraMovementScript());

  fennecs::EntityHandle sponza = scene.CreateEntity("Sponza");
  sponza = scene.GetEntityWorld().Attach<MeshComponent>(sponza, AssetRegistry::Instance()->Load<Mesh>("meshes/sponza.obj"));
  sponza = scene.GetEntityWorld().Attach<TransformComponent>(sponza);
  sponza.Get<TransformComponent>().transform.scale = glm::vec3(0.02);
  sponza.Get<TransformComponent>().transform.Rotate(M_PI_2, glm::vec3{0, 1, 0});

  // fennecs::EntityHandle dir_light = scene.CreateEntity("Directional light");
  // dir_light = scene.GetEntityWorld().Attach<DirectionalLightSpecification>(dir_light, glm::vec3{0.5, 0.5, 0.25}, 0);
  // dir_light = scene.GetEntityWorld().Attach<TransformComponent>(dir_light, Transform(glm::vec3(0), glm::vec3(-0.5, 0, 0)));

  fennecs::EntityHandle point_light1 = scene.CreateEntity("Point light1");
  point_light1 = scene.GetEntityWorld().Attach<PointLightSpecification>(point_light1, glm::vec3{1.0, 0.5, 0.1}, 2, 4);
  point_light1 = scene.GetEntityWorld().Attach<TransformComponent>(point_light1, Transform(glm::vec3(-8.9, 2.5, 24)));

  fennecs::EntityHandle point_light2 = scene.CreateEntity("Point light2");
  point_light2 = scene.GetEntityWorld().Attach<PointLightSpecification>(point_light2, glm::vec3{1.0, 0.5, 0.1}, 2, 4);
  point_light2 = scene.GetEntityWorld().Attach<TransformComponent>(point_light2, Transform(glm::vec3(-8.9, 2.5, -22)));

  fennecs::EntityHandle point_light3 = scene.CreateEntity("Point light3");
  point_light3 = scene.GetEntityWorld().Attach<PointLightSpecification>(point_light3, glm::vec3{1.0, 0.5, 0.1}, 2, 4);
  point_light3 = scene.GetEntityWorld().Attach<TransformComponent>(point_light3, Transform(glm::vec3(8.5, 2.5, 24)));

  fennecs::EntityHandle point_light4 = scene.CreateEntity("Point light4");
  point_light4 = scene.GetEntityWorld().Attach<PointLightSpecification>(point_light4, glm::vec3{1.0, 0.5, 0.1}, 2, 4);
  point_light4 = scene.GetEntityWorld().Attach<TransformComponent>(point_light4, Transform(glm::vec3(8.5, 2.5, -22)));

  scene.OnStart(event_dispatcher);

  /* Render Loop */
  Timer timer;
  float time_start = timer.Elapsed();
  while (running) {
    float timestep = timer.Elapsed() - time_start;
    time_start = timer.Elapsed();

    InputEventManager::TriggerEvents();

    scene.OnUpdate(timestep);

    device.FrameBegin();

    uint32_t texture_idx = 0;
    device.AcquireNextTexture(swapchain, &texture_idx);

    uint32_t current_frame = device.CurrentFrame();
    // LOG_DEBUG("Current frame = {0}", current_frame);
    {
      ScopedTimer trace_timer{"WaitForFences()"};
      device.WaitForFences(1, &fences_in_flight[current_frame]);
      device.ResetFence(fences_in_flight[current_frame]);
    }

    CommandBuffer& command_buffer = *command_buffers[current_frame];
    {
      ScopedTimer trace_timer{"command_buffer.Reset() and Begin()"};
      command_buffer.Reset();
      command_buffer.Begin();
    }

    {
      ScopedTimer trace_timer{"scene.Render()"};
      scene.Render(renderer, command_buffer, current_frame, timer.Elapsed());
    }

    {
      ScopedTimer trace_timer{"TransitionLayout(swapchain_texture -> kTransferDst)"};
      command_buffer.TransitionLayout(swapchain_textures[texture_idx], TextureLayout::kUndefined, TextureLayout::kTransferDst);
    }

    {
      ScopedTimer trace_timer{"CopyTexture()"};
      command_buffer.CopyTexture(color_output->GetHandle(), swapchain_textures[texture_idx], surface_width, surface_height);
    }

    {
      ScopedTimer trace_timer{"TransitionLayout(swapchain_texture -> kPresentSrc)"};
      command_buffer.TransitionLayout(swapchain_textures[texture_idx], TextureLayout::kTransferDst, TextureLayout::kPresentSrc);
    }

    {
      ScopedTimer trace_timer{"command_buffer.End() and Submit()"};
      command_buffer.End();
      command_buffer.Submit(fences_in_flight[current_frame], semaphores_render_finished[current_frame]);
      // command_buffer.Submit(fences_in_flight[current_frame]);
    }

    device.FrameEnd();

    {
      ScopedTimer trace_timer{"Present()"};
      device.Present(swapchain, semaphores_render_finished[current_frame]);
      // device.Present(swapchain, kInvalidRenderResourceHandle);
    }

    float frame_time = timer.Elapsed() - time_start;
    {
      ScopedTimer trace_timer{"window.SetFPSToTitle()"};
      window.SetFPSToTitle(1 / frame_time);
    }

    // LOG_DEBUG("Frame: {0} FPS: {1}", current_frame, 1 / frame_time);
  }

  device.WaitIdle();

  return 0;
}