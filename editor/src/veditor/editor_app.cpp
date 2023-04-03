/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file editor_app.cpp
 * @date 2022-08-02
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

#include <veditor/camera_movement.hpp>
#include <veditor/editor_app.hpp>
#include <vulture/asset/asset_registry.hpp>
#include <vulture/asset/loaders/fbx_loader.hpp>
#include <vulture/asset/loaders/jpg_loader.hpp>
#include <vulture/asset/loaders/obj_loader.hpp>
#include <vulture/asset/loaders/png_loader.hpp>
#include <vulture/asset/loaders/shader_loader.hpp>
#include <vulture/asset/loaders/tga_loader.hpp>
#include <vulture/renderer/builtin/forward_pass.hpp>

using namespace vulture;

/************************************************************************************************
 * INIT
 ************************************************************************************************/
EditorApp::EditorApp() : window_(1600, 900), device_(*RenderDevice::Create(RenderDevice::DeviceFamily::kVulkan)) {}

EditorApp::~EditorApp()
{
  // OnCloseImGui();
  // preview_panel_.OnClose();
}

int EditorApp::Init() {
  Logger::SetTraceEnabled(false);

  InputEventManager::SetWindowAndDispatcher(&window_, &event_dispatcher_);
  event_dispatcher_.GetSink<QuitEvent>().Connect<&EditorApp::OnQuit>(*this);

  device_.Init(&window_, nullptr, nullptr, true);

  /* Register loaders FIXME: (tralf-strues) move somewhere else */
  AssetRegistry::Instance()->RegisterLoader(CreateShared<OBJLoader>(device_));
  AssetRegistry::Instance()->RegisterLoader(CreateShared<FBXLoader>(device_));
  AssetRegistry::Instance()->RegisterLoader(CreateShared<TGALoader>(device_));
  AssetRegistry::Instance()->RegisterLoader(CreateShared<JPGLoader>(device_));
  AssetRegistry::Instance()->RegisterLoader(CreateShared<PNGLoader>(device_));
  AssetRegistry::Instance()->RegisterLoader(CreateShared<ShaderLoader>(device_));

  CreateSwapchain();
  CreateFrameData();

  imgui_implementation_ = ImGuiImplementation::Create(window_, device_, swapchain_);
  OnUpdateImGuiStyle();
  imgui_implementation_->UpdateFonts("assets/.vulture/fonts/Roboto/Roboto-Medium.ttf", 16.0f);
  
  /* Panels init */
  preview_panel_ = CreateUnique<PreviewPanel>(device_, *imgui_implementation_, event_dispatcher_);
  preview_panel_->OnInit();
  entities_panel_ = CreateUnique<EntitiesPanel>();
  inspector_panel_ = CreateUnique<InspectorPanel>();

  CreateRenderer();

  return 0;
}

void EditorApp::CreateSwapchain() {
  swapchain_ = device_.CreateSwapchain(kTextureUsageBitTransferDst);
  uint32_t swapchain_size{0};
  device_.GetSwapchainTextures(swapchain_, &swapchain_size, nullptr);

  LOG_INFO("Swapchain size is {0}", swapchain_size);

  swapchain_textures_.resize(swapchain_size);
  device_.GetSwapchainTextures(swapchain_, nullptr, swapchain_textures_.data());
}

void EditorApp::DestroySwapchain() {
  if (ValidRenderHandle(swapchain_)) {
    device_.DeleteSwapchain(swapchain_);
  }
}

void EditorApp::CreateFrameData() {
  for (auto& frame : frames_) {
    frame.command_buffer           = device_.CreateCommandBuffer(CommandBufferType::kGraphics);
    frame.fence_render_finished     = device_.CreateFence();
    frame.semaphore_render_finished = device_.CreateSemaphore();
  }
}

void EditorApp::DestroyFrameData() {
  for (auto& frame : frames_) {
    if (frame.command_buffer) {
      delete frame.command_buffer;
    }

    if (ValidRenderHandle(frame.fence_render_finished)) {
      device_.DeleteFence(frame.fence_render_finished);
    }

    if (ValidRenderHandle(frame.semaphore_render_finished)) {
      device_.DeleteSemaphore(frame.semaphore_render_finished);
    }
  }
}

void EditorApp::CreateRenderer() {
  /* Render Graph */
  auto render_graph = CreateUnique<rg::RenderGraph>();
  ColorOutput& color_output_data = render_graph->GetBlackboard().Add<ColorOutput>();
  color_output_data.texture_id =
      render_graph->ImportTexture("color", preview_panel_->GetTexture(), TextureLayout::kShaderReadOnly);

  render_graph->AddPass<ForwardPass>(ForwardPass::GetName());

  render_graph->Setup();
  render_graph->Compile(device_);

  std::ofstream output_file("log/render_graph.dot", std::ios::trunc);
  assert(output_file.is_open());
  render_graph->ExportGraphviz(output_file);
  system("dot -Tpng log/render_graph.dot > log/render_graph.png");

  /* Renderer */
  renderer_ = CreateUnique<Renderer>(device_, std::move(render_graph));
}

/************************************************************************************************
 * RUN
 ************************************************************************************************/
void EditorApp::Run() {
  fennecs::EntityHandle camera = scene_.CreateEntity("Editor camera");
  camera = scene_.GetEntityWorld().Attach<CameraComponent>(camera, PerspectiveCameraSpecs(1600.0f / 900.0f), true);
  camera = scene_.GetEntityWorld().Attach<TransformComponent>(camera, glm::vec3(0, 3, 15));
  camera = scene_.GetEntityWorld().Attach<ScriptComponent>(camera, new CameraMovementScript());

  fennecs::EntityHandle sponza = scene_.CreateEntity("Sponza");
  sponza = scene_.GetEntityWorld().Attach<MeshComponent>(sponza, AssetRegistry::Instance()->Load<Mesh>("meshes/sponza.obj"));
  sponza = scene_.GetEntityWorld().Attach<TransformComponent>(sponza);
  sponza.Get<TransformComponent>().transform.scale = glm::vec3(0.02);
  sponza.Get<TransformComponent>().transform.Rotate(M_PI_2, kUp);

  fennecs::EntityHandle dir_light = scene_.CreateEntity("Sky light");
  dir_light = scene_.GetEntityWorld().Attach<DirectionalLightSpecification>(dir_light, glm::vec3{1.0, 0.517, 0.152}, 0.01);
  dir_light = scene_.GetEntityWorld().Attach<TransformComponent>(dir_light, Transform(glm::vec3(0), glm::vec3(-0.5, 0, 0)));

  fennecs::EntityHandle point_light1 = scene_.CreateEntity("Point light1");
  point_light1 = scene_.GetEntityWorld().Attach<PointLightSpecification>(point_light1, glm::vec3{1.0, 0.5, 0.1}, 2, 4);
  point_light1 = scene_.GetEntityWorld().Attach<TransformComponent>(point_light1, Transform(glm::vec3(-8.9, 2.5, 24)));

  fennecs::EntityHandle point_light2 = scene_.CreateEntity("Point light2");
  point_light2 = scene_.GetEntityWorld().Attach<PointLightSpecification>(point_light2, glm::vec3{1.0, 0.5, 0.1}, 2, 4);
  point_light2 = scene_.GetEntityWorld().Attach<TransformComponent>(point_light2, Transform(glm::vec3(-8.9, 2.5, -22)));

  fennecs::EntityHandle point_light3 = scene_.CreateEntity("Point light3");
  point_light3 = scene_.GetEntityWorld().Attach<PointLightSpecification>(point_light3, glm::vec3{1.0, 0.5, 0.1}, 2, 4);
  point_light3 = scene_.GetEntityWorld().Attach<TransformComponent>(point_light3, Transform(glm::vec3(8.5, 2.5, 24)));

  fennecs::EntityHandle point_light4 = scene_.CreateEntity("Point light4");
  point_light4 = scene_.GetEntityWorld().Attach<PointLightSpecification>(point_light4, glm::vec3{1.0, 0.5, 0.1}, 2, 4);
  point_light4 = scene_.GetEntityWorld().Attach<TransformComponent>(point_light4, Transform(glm::vec3(8.5, 2.5, -22)));

  // fennecs::EntityHandle skybox = scene_.CreateChildEntity(camera, "Skybox");
  // skybox = scene_.GetEntityWorld().Attach<TransformComponent>(skybox);
  // skybox = scene_.GetEntityWorld().Attach<MeshComponent>(skybox,
  //   CreateSkyboxMesh({"assets/textures/skybox_morning_field/skybox_morning_field_right.jpeg",
  //   "assets/textures/skybox_morning_field/skybox_morning_field_left.jpeg",
  //   "assets/textures/skybox_morning_field/skybox_morning_field_top.jpeg",
  //   "assets/textures/skybox_morning_field/skybox_morning_field_bottom.jpeg",
  //   "assets/textures/skybox_morning_field/skybox_morning_field_front.jpeg",
  //   "assets/textures/skybox_morning_field/skybox_morning_field_back.jpeg"}));

  // fennecs::EntityHandle dir_light = scene_.CreateEntity("Directional light");
  // dir_light = scene_.GetEntityWorld().Attach<LightSourceComponent>(dir_light,
  //     DirectionalLightSpecs(LightColorSpecs(glm::vec3(0.5), glm::vec3(0.9), glm::vec3(0.01))));
  // dir_light = scene_.GetEntityWorld().Attach<TransformComponent>(dir_light, Transform(glm::vec3(0), glm::vec3(-0.5, 0, 0)));

  fennecs::EntityHandle street_lamp = scene_.CreateEntity("Street lamp");
  street_lamp = scene_.GetEntityWorld().Attach<TransformComponent>(street_lamp);
  street_lamp = scene_.GetEntityWorld().Attach<MeshComponent>(street_lamp, AssetRegistry::Instance()->Load<Mesh>("meshes/street_lamp.obj"));

  scene_.OnStart(event_dispatcher_);

  bool first_frame = true;
  current_time_ = timer_.Elapsed();
  while (running_) {
    current_timestep_ = timer_.Elapsed() - current_time_;
    current_time_ = timer_.Elapsed();

    InputEventManager::TriggerEvents();

    if (preview_panel_->Resized()) {
      preview_panel_->OnResize();

      const auto& specification = preview_panel_->GetTexture()->GetSpecification();
      scene_.OnViewportResize(specification.width, specification.height);

      ColorOutput& color_output_data = renderer_->GetBlackboard().Get<ColorOutput>();
      renderer_->GetRenderGraph().ReimportTexture(color_output_data.texture_id, preview_panel_->GetTexture());
    }

    scene_.OnUpdate(current_timestep_);
    selected_entity_ = entities_panel_->GetSelectedEntity();

    Render();

    window_.SetFPSToTitle(1 / current_timestep_);

    // FIXME: in order to update dpi (for macos), there needs to be a call to ImGuiImplementation::FrameStart
    if (first_frame) {
      OnResize();
      first_frame = false;
    }
  }

  device_.WaitIdle();
}

void EditorApp::Render() {
  device_.FrameBegin();

  uint32_t texture_idx = 0;
  if (!device_.AcquireNextTexture(swapchain_, &texture_idx)) {
    OnResize();
    return;
  }

  uint32_t current_frame = device_.CurrentFrame();
  // LOG_DEBUG("Current frame = {0}", current_frame);
  {
    ScopedTimer trace_timer{"WaitForFences()"};
    device_.WaitForFences(1, &frames_[current_frame].fence_render_finished);
    device_.ResetFence(frames_[current_frame].fence_render_finished);
  }

  CommandBuffer& command_buffer = *frames_[current_frame].command_buffer;
  {
    ScopedTimer trace_timer{"command_buffer.Reset() and Begin()"};
    command_buffer.Reset();
    command_buffer.Begin();
  }

  {
    ScopedTimer trace_timer{"scene.Render()"};
    scene_.Render(*renderer_, command_buffer, current_frame, timer_.Elapsed());
  }

  RenderUI(command_buffer, texture_idx);

  {
    ScopedTimer trace_timer{"command_buffer.End() and Submit()"};
    command_buffer.End();
    command_buffer.Submit(frames_[current_frame].fence_render_finished, frames_[current_frame].semaphore_render_finished);
  }

  device_.FrameEnd();

  {
    ScopedTimer trace_timer{"Present()"};
    if (!device_.Present(swapchain_, frames_[current_frame].semaphore_render_finished)) {
      OnResize();
    }
  }
}

void EditorApp::RenderUI(CommandBuffer& command_buffer, uint32_t texture_idx) {
  imgui_implementation_->FrameStart();

  preview_panel_->OnRender(scene_, selected_entity_);
  entities_panel_->OnRender(scene_);
  inspector_panel_->OnRender(selected_entity_);

  imgui_implementation_->Render(command_buffer, texture_idx);
  imgui_implementation_->FrameEnd();
}

void EditorApp::OnUpdateImGuiStyle() {
  ImGuiStyle& style = ImGui::GetStyle();
        
  // light style from Pacôme Danhiez (user itamago) https://github.com/ocornut/imgui/pull/511#issuecomment-175719267
  style.Alpha = 1.0f;
  style.FrameRounding = 3.0f;
  style.Colors[ImGuiCol_Text]                  = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
  style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
  style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.94f, 0.94f, 0.94f, 0.94f);
  // style.Colors[ImGuiCol_ChildWindowBg]         = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  style.Colors[ImGuiCol_PopupBg]               = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
  style.Colors[ImGuiCol_Border]                = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
  style.Colors[ImGuiCol_BorderShadow]          = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
  style.Colors[ImGuiCol_FrameBg]               = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
  style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
  style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
  style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
  style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
  style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
  style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
  style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
  style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
  style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
  style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
  // style.Colors[ImGuiCol_ComboBg]               = ImVec4(0.86f, 0.86f, 0.86f, 0.99f);
  style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
  style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_Button]                = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
  style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_ButtonActive]          = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_Header]                = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
  style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
  style.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  // style.Colors[ImGuiCol_Column]                = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
  // style.Colors[ImGuiCol_ColumnHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
  // style.Colors[ImGuiCol_ColumnActive]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
  style.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
  style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
  // style.Colors[ImGuiCol_CloseButton]           = ImVec4(0.59f, 0.59f, 0.59f, 0.50f);
  // style.Colors[ImGuiCol_CloseButtonHovered]    = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
  // style.Colors[ImGuiCol_CloseButtonActive]     = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
  style.Colors[ImGuiCol_PlotLines]             = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
  style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
  style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
  style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
  style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
  // style.Colors[ImGuiCol_ModalWindowDarkening]  = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
}

/************************************************************************************************
 * EVENTS
 ************************************************************************************************/
void EditorApp::OnQuit(const QuitEvent&) {
  running_ = false;
}

void EditorApp::OnResize() {
  // FIXME: (tralf-strues) otherwise the texture is destroyed, but is still being used by a command buffer :(
  device_.WaitIdle();

  DestroySwapchain();
  CreateSwapchain();

  imgui_implementation_->OnResize(swapchain_);
  imgui_implementation_->UpdateFonts("assets/.vulture/fonts/Roboto/Roboto-Medium.ttf", 16.0f);
}
