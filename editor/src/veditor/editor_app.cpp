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
#include <vulture/asset/loaders/dae_loader.hpp>
#include <vulture/asset/loaders/fbx_loader.hpp>
#include <vulture/asset/loaders/glb_loader.hpp>
#include <vulture/asset/loaders/jpg_loader.hpp>
#include <vulture/asset/loaders/obj_loader.hpp>
#include <vulture/asset/loaders/png_loader.hpp>
#include <vulture/asset/loaders/shader_loader.hpp>
#include <vulture/asset/loaders/skybox_loader.hpp>
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
  AssetRegistry::Instance()->RegisterLoader(CreateShared<DAELoader>(device_));
  AssetRegistry::Instance()->RegisterLoader(CreateShared<FBXLoader>(device_));
  AssetRegistry::Instance()->RegisterLoader(CreateShared<GLBLoader>(device_));
  AssetRegistry::Instance()->RegisterLoader(CreateShared<TGALoader>(device_));
  AssetRegistry::Instance()->RegisterLoader(CreateShared<JPGLoader>(device_));
  AssetRegistry::Instance()->RegisterLoader(CreateShared<PNGLoader>(device_));
  AssetRegistry::Instance()->RegisterLoader(CreateShared<ShaderLoader>(device_));
  AssetRegistry::Instance()->RegisterLoader(CreateShared<SkyboxLoader>(device_));

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
    frame.command_buffer                    = device_.CreateCommandBuffer(CommandBufferType::kGraphics);
    frame.fence_render_finished              = device_.CreateFence();
    frame.semaphore_render_finished          = device_.CreateSemaphore();
    frame.semaphore_swapchain_texture_ready = device_.CreateSemaphore();
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

    if (ValidRenderHandle(frame.semaphore_swapchain_texture_ready)) {
      device_.DeleteSemaphore(frame.semaphore_swapchain_texture_ready);
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
  AssetRegistry& asset_registry = *AssetRegistry::Instance();

  fennecs::EntityHandle camera = scene_.CreateEntity("Editor camera");
  camera = scene_.GetEntityWorld().Attach<CameraComponent>(camera, PerspectiveCameraSpecs(1600.0f / 900.0f), true);
  camera = scene_.GetEntityWorld().Attach<TransformComponent>(camera, glm::vec3(0, 3, 15));
  camera = scene_.GetEntityWorld().Attach<ScriptComponent>(camera, new CameraMovementScript());

  fennecs::EntityHandle sponza = scene_.CreateEntity("Sponza");
  // sponza = scene_.GetEntityWorld().Attach<MeshComponent>(sponza, asset_registry.Load<Mesh>("meshes/sponza.obj"));
  sponza = scene_.GetEntityWorld().Attach<MeshComponent>(sponza, asset_registry.Load<Mesh>("meshes/sponza_pbr_new/sponza_pbr_new.gltf"));
  sponza = scene_.GetEntityWorld().Attach<TransformComponent>(sponza);
  // sponza.Get<TransformComponent>().transform.scale = glm::vec3(0.02);
  sponza.Get<TransformComponent>().transform.scale = glm::vec3(2);
  sponza.Get<TransformComponent>().transform.rotation = glm::quat(glm::vec3(0.0f, M_PI + M_PI_2, 0.0f));
  // sponza.Get<TransformComponent>().transform.rotation = glm::quat(glm::vec3(0.0f, M_PI_2, M_PI_2));

  // sponza.Get<TransformComponent>().transform.scale = glm::vec3(1);
  // sponza.Get<TransformComponent>().transform.Rotate(M_PI_2, kUp);






  SharedPtr<Shader> pbr_shader = asset_registry.Load<Shader>(".vulture/shaders/BuiltIn.PBR.shader");

  SharedPtr<Material> material = CreateShared<Material>(device_);
  material->AddShader(pbr_shader);

  MaterialPass& material_pass = material->GetMaterialPass(pbr_shader->GetTargetPassId());
  material_pass.GetProperty<uint32_t>("useAlbedoMap") = 1;
  material_pass.SetTextureSampler("uAlbedoMap", asset_registry.Load<Texture>("textures/rusted-steel-ue/rusted-steel_albedo.png"));
  material_pass.GetProperty<uint32_t>("useNormalMap") = 1;
  material_pass.SetTextureSampler("uNormalMap", asset_registry.Load<Texture>("textures/rusted-steel-ue/rusted-steel_normal-dx.png"));
  material_pass.GetProperty<uint32_t>("useMetallicMap") = 1;
  material_pass.SetTextureSampler("uMetallicMap", asset_registry.Load<Texture>("textures/rusted-steel-ue/rusted-steel_metallic.png"));
  material_pass.GetProperty<uint32_t>("useRoughnessMap") = 1;
  material_pass.SetTextureSampler("uRoughnessMap", asset_registry.Load<Texture>("textures/rusted-steel-ue/rusted-steel_roughness.png"));
  material_pass.GetProperty<uint32_t>("useCombinedMetallicRoughnessMap") = 0;
  material_pass.SetTextureSampler("uCombinedMetallicRoughnessMap", asset_registry.Load<Texture>(".vulture/textures/blank.png"));

  material->WriteMaterialPassDescriptors();

  SharedPtr<Mesh> sphere_mesh = asset_registry.Load<Mesh>("meshes/sphere.fbx");
  Submesh& sphere_submesh = sphere_mesh->GetSubmeshes()[0];
  sphere_submesh.SetMaterial(material);

  fennecs::EntityHandle sphere = scene_.CreateEntity("Sphere");
  sphere = scene_.GetEntityWorld().Attach<MeshComponent>(sphere, sphere_mesh);
  sphere = scene_.GetEntityWorld().Attach<TransformComponent>(sphere, Transform(glm::vec3(-2.0f, 2.0f, 0.0f)));
  sphere.Get<TransformComponent>().transform.rotation = glm::quat(glm::vec3(M_PI_2, 0.0f, 0.0f));





  fennecs::EntityHandle dir_light = scene_.CreateEntity("Sky light");
  dir_light = scene_.GetEntityWorld().Attach<DirectionalLightSpecification>(dir_light, glm::vec3{1.0, 1.0, 1.0}, 0.5);
  dir_light = scene_.GetEntityWorld().Attach<TransformComponent>(dir_light, Transform(glm::vec3(0), glm::vec3(-0.5, 0, 0)));

  fennecs::EntityHandle point_light1 = scene_.CreateEntity("Point light1");
  point_light1 = scene_.GetEntityWorld().Attach<PointLightSpecification>(point_light1, glm::vec3{1.0, 0.5, 0.1}, 7.5, 3);
  point_light1 = scene_.GetEntityWorld().Attach<TransformComponent>(point_light1, Transform(glm::vec3(-9.583, 8.256, 19.639)));

  fennecs::EntityHandle point_light2 = scene_.CreateEntity("Point light2");
  point_light2 = scene_.GetEntityWorld().Attach<PointLightSpecification>(point_light2, glm::vec3{1.0, 0.5, 0.1}, 7.5, 3);
  point_light2 = scene_.GetEntityWorld().Attach<TransformComponent>(point_light2, Transform(glm::vec3(-9.616, 8.256, -20.075)));

  fennecs::EntityHandle point_light3 = scene_.CreateEntity("Point light3");
  point_light3 = scene_.GetEntityWorld().Attach<PointLightSpecification>(point_light3, glm::vec3{1.0, 0.5, 0.1}, 7.5, 3);
  point_light3 = scene_.GetEntityWorld().Attach<TransformComponent>(point_light3, Transform(glm::vec3(9.479, 8.256, 19.635)));

  fennecs::EntityHandle point_light4 = scene_.CreateEntity("Point light4");
  point_light4 = scene_.GetEntityWorld().Attach<PointLightSpecification>(point_light4, glm::vec3{1.0, 0.5, 0.1}, 7.5, 3);
  point_light4 = scene_.GetEntityWorld().Attach<TransformComponent>(point_light4, Transform(glm::vec3(9.509, 8.256, -20.320)));

  fennecs::EntityHandle point_light5 = scene_.CreateEntity("Point light5");
  point_light5 = scene_.GetEntityWorld().Attach<PointLightSpecification>(point_light5, glm::vec3{1.0, 0.5, 0.1}, 7.5, 3);
  point_light5 = scene_.GetEntityWorld().Attach<TransformComponent>(point_light5, Transform(glm::vec3(1.845, 6.067, 31.221)));

  fennecs::EntityHandle point_light6 = scene_.CreateEntity("Point light6");
  point_light6 = scene_.GetEntityWorld().Attach<PointLightSpecification>(point_light6, glm::vec3{1.0, 0.5, 0.1}, 7.5, 3);
  point_light6 = scene_.GetEntityWorld().Attach<TransformComponent>(point_light6, Transform(glm::vec3(-0.129, 7.713, -27.196)));

  fennecs::EntityHandle skybox = scene_.CreateEntity("Skybox");
  skybox = scene_.GetEntityWorld().Attach<MeshComponent>(skybox, asset_registry.Load<Mesh>("textures/skybox_morning_field/morning_field.skybox"));
  skybox = scene_.GetEntityWorld().Attach<TransformComponent>(skybox);

  // fennecs::EntityHandle dir_light = scene_.CreateEntity("Directional light");
  // dir_light = scene_.GetEntityWorld().Attach<LightSourceComponent>(dir_light,
  //     DirectionalLightSpecs(LightColorSpecs(glm::vec3(0.5), glm::vec3(0.9), glm::vec3(0.01))));
  // dir_light = scene_.GetEntityWorld().Attach<TransformComponent>(dir_light, Transform(glm::vec3(0), glm::vec3(-0.5, 0, 0)));

  // fennecs::EntityHandle street_lamp = scene_.CreateEntity("Street lamp");
  // street_lamp = scene_.GetEntityWorld().Attach<TransformComponent>(street_lamp);
  // street_lamp = scene_.GetEntityWorld().Attach<MeshComponent>(street_lamp, asset_registry.Load<Mesh>("meshes/street_lamp.obj"));

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

  uint32_t current_frame_idx = device_.CurrentFrame();
  Frame& current_frame = frames_[current_frame_idx];

  uint32_t texture_idx = 0;
  if (!device_.AcquireNextTexture(swapchain_, &texture_idx, current_frame.semaphore_swapchain_texture_ready)) {
    OnResize();
    return;
  }

  // LOG_DEBUG("Current frame = {0}", current_frame);
  {
    ScopedTimer trace_timer{"WaitForFences()"};
    device_.WaitForFences(1, &current_frame.fence_render_finished);
    device_.ResetFence(current_frame.fence_render_finished);
  }

  CommandBuffer& command_buffer = *current_frame.command_buffer;
  {
    ScopedTimer trace_timer{"command_buffer.Reset() and Begin()"};
    command_buffer.Reset();
    command_buffer.Begin();
  }

  {
    ScopedTimer trace_timer{"scene.Render()"};
    scene_.Render(*renderer_, command_buffer, current_frame_idx, timer_.Elapsed());
  }

  RenderUI(command_buffer, texture_idx);

  {
    ScopedTimer trace_timer{"command_buffer.End() and Submit()"};
    command_buffer.End();
    command_buffer.Submit(current_frame.fence_render_finished, current_frame.semaphore_render_finished,
                          current_frame.semaphore_swapchain_texture_ready);
  }

  device_.FrameEnd();

  {
    ScopedTimer trace_timer{"Present()"};
    if (!device_.Present(swapchain_, current_frame.semaphore_render_finished)) {
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
  style.Colors[ImGuiCol_Text]                   = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
  style.Colors[ImGuiCol_TextDisabled]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
  style.Colors[ImGuiCol_WindowBg]               = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  style.Colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  style.Colors[ImGuiCol_PopupBg]                = ImVec4(1.00f, 1.00f, 1.00f, 0.98f);
  style.Colors[ImGuiCol_Border]                 = ImVec4(0.00f, 0.00f, 0.00f, 0.16f);
  style.Colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  style.Colors[ImGuiCol_FrameBg]                = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  style.Colors[ImGuiCol_FrameBgHovered]         = ImVec4(1.00f, 0.93f, 0.85f, 0.40f);
  style.Colors[ImGuiCol_FrameBgActive]          = ImVec4(1.00f, 0.93f, 0.85f, 1.00f);
  style.Colors[ImGuiCol_TitleBg]                = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  style.Colors[ImGuiCol_TitleBgActive]          = ImVec4(1.00f, 0.93f, 0.85f, 1.00f);
  style.Colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  style.Colors[ImGuiCol_MenuBarBg]              = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  style.Colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
  style.Colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.69f, 0.69f, 0.69f, 0.80f);
  style.Colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.49f, 0.49f, 0.49f, 0.80f);
  style.Colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
  style.Colors[ImGuiCol_CheckMark]              = ImVec4(0.92f, 0.60f, 0.38f, 1.00f);
  style.Colors[ImGuiCol_SliderGrab]             = ImVec4(0.92f, 0.60f, 0.38f, 1.00f);
  style.Colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.92f, 0.60f, 0.38f, 0.78f);
  style.Colors[ImGuiCol_Button]                 = ImVec4(0.92f, 0.60f, 0.38f, 0.40f);
  style.Colors[ImGuiCol_ButtonHovered]          = ImVec4(0.92f, 0.60f, 0.38f, 1.00f);
  style.Colors[ImGuiCol_ButtonActive]           = ImVec4(0.92f, 0.60f, 0.38f, 1.00f);
  style.Colors[ImGuiCol_Header]                 = ImVec4(1.00f, 0.85f, 0.71f, 0.31f);
  style.Colors[ImGuiCol_HeaderHovered]          = ImVec4(1.00f, 0.85f, 0.71f, 0.80f);
  style.Colors[ImGuiCol_HeaderActive]           = ImVec4(1.00f, 0.85f, 0.71f, 1.00f);
  style.Colors[ImGuiCol_Separator]              = ImVec4(0.64f, 0.45f, 0.27f, 0.39f);
  style.Colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.64f, 0.45f, 0.27f, 0.78f);
  style.Colors[ImGuiCol_SeparatorActive]        = ImVec4(0.64f, 0.45f, 0.27f, 1.00f);
  style.Colors[ImGuiCol_ResizeGrip]             = ImVec4(0.64f, 0.45f, 0.27f, 0.39f);
  style.Colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.64f, 0.45f, 0.27f, 0.67f);
  style.Colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.64f, 0.45f, 0.27f, 0.95f);
  style.Colors[ImGuiCol_Tab]                    = ImVec4(1.00f, 0.85f, 0.71f, 0.31f);
  style.Colors[ImGuiCol_TabHovered]             = ImVec4(1.00f, 0.85f, 0.71f, 0.80f);
  style.Colors[ImGuiCol_TabActive]              = ImVec4(1.00f, 0.85f, 0.71f, 1.00f);
  style.Colors[ImGuiCol_TabUnfocused]           = ImVec4(1.00f, 0.85f, 0.71f, 0.31f);
  style.Colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(1.00f, 0.85f, 0.71f, 1.00f);
  style.Colors[ImGuiCol_DockingPreview]         = ImVec4(0.26f, 0.59f, 0.98f, 0.22f);
  style.Colors[ImGuiCol_DockingEmptyBg]         = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
  style.Colors[ImGuiCol_PlotLines]              = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
  style.Colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
  style.Colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
  style.Colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.45f, 0.00f, 1.00f);
  style.Colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.78f, 0.87f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.57f, 0.57f, 0.64f, 1.00f);
  style.Colors[ImGuiCol_TableBorderLight]       = ImVec4(0.68f, 0.68f, 0.74f, 1.00f);
  style.Colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  style.Colors[ImGuiCol_TableRowBgAlt]          = ImVec4(0.30f, 0.30f, 0.30f, 0.09f);
  style.Colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
  style.Colors[ImGuiCol_DragDropTarget]         = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
  style.Colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
  style.Colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(0.70f, 0.70f, 0.70f, 0.70f);
  style.Colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.20f, 0.20f, 0.20f, 0.20f);
  style.Colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

  style.WindowPadding     = ImVec2(4.0f, 4.0f);
  style.FramePadding      = ImVec2(4.0f, 3.0f);
  style.CellPadding       = ImVec2(4.0f, 2.0f);
  style.ItemSpacing       = ImVec2(8.0f, 4.0f);
  style.ItemInnerSpacing  = ImVec2(6.0f, 4.0f);
  style.TouchExtraPadding = ImVec2(0.0f, 0.0f);
  style.IndentSpacing     = 8.0f;
  style.ScrollbarSize     = 12.0f;
  style.GrabMinSize       = 12.0f;

  style.WindowBorderSize  = 1.0f;
  style.ChildBorderSize   = 1.0f;
  style.PopupBorderSize   = 1.0f;
  style.FrameBorderSize   = 1.0f;
  style.TabBorderSize     = 0.0f;

  style.WindowRounding    = 0.0f;
  style.ChildRounding     = 0.0f;
  style.FrameRounding     = 3.0f;
  style.PopupRounding     = 0.0f;
  style.ScrollbarRounding = 9.0f;
  style.GrabRounding      = 3.0f;
  style.LogSliderDeadzone = 4.0f;
  style.TabRounding       = 2.0f;

  if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
  {
      style.WindowRounding = 0.0f;
  }
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
