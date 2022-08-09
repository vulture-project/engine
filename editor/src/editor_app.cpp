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

#include "editor_app.hpp"
#include "renderer/3d/renderer3d.hpp"
#include "core/resource_manager.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>

using namespace vulture;

EditorApp::EditorApp() : window_(1280, 960) {}

EditorApp::~EditorApp()
{
  OnCloseImGui();
  preview_panel_.OnClose();
}

int EditorApp::Init() {
  InputEventManager::SetWindowAndDispatcher(&window_, &event_dispatcher_);
  Renderer3D::Init();

  // OnResize(window_.GetFramebufferWidth(), window_.GetFramebufferHeight());
  OnInitImGui();

  event_dispatcher_.GetSink<QuitEvent>().Connect<&EditorApp::OnQuit>(*this);

  /* Panels init */
  preview_panel_.OnInit();

  return 0;
}

void EditorApp::Run() {
  EntityHandle camera = scene_.CreateEntity("Editor camera");
  camera.AddComponent<CameraComponent>(PerspectiveCameraSpecs(1280 / 960), true);
  camera.AddComponent<TransformComponent>(glm::vec3(0, 3, 15));

  EntityHandle sponza = scene_.CreateEntity("Sponza");
  sponza.AddComponent<MeshComponent>(ResourceManager::LoadMesh("res/meshes/sponza.obj"));
  sponza.AddComponent<TransformComponent>();
  sponza.GetComponent<TransformComponent>()->transform.scale = glm::vec3(0.025);
  sponza.GetComponent<TransformComponent>()->transform.Rotate(M_PI_2, kDefaultUpVector);

  EntityHandle sponza_light = scene_.CreateEntity("Sponza point light");
  sponza_light.AddComponent<TransformComponent>(glm::vec3(0, 3, 0));
  sponza_light.AddComponent<LightSourceComponent>(PointLightSpecs(
        LightColorSpecs(glm::vec3(0.02), glm::vec3(0.9, 0.7, 1.0), glm::vec3(0.1)), LightAttenuationSpecs(15))); 

  EntityHandle skybox = scene_.CreateChildEntity(camera, "Skybox");
  skybox.AddComponent<TransformComponent>();
  skybox.AddComponent<MeshComponent>(CreateSkyboxMesh({"res/textures/skybox_morning_field/skybox_morning_field_right.jpeg",
                                                       "res/textures/skybox_morning_field/skybox_morning_field_left.jpeg",
                                                       "res/textures/skybox_morning_field/skybox_morning_field_top.jpeg",
                                                       "res/textures/skybox_morning_field/skybox_morning_field_bottom.jpeg",
                                                       "res/textures/skybox_morning_field/skybox_morning_field_front.jpeg",
                                                       "res/textures/skybox_morning_field/skybox_morning_field_back.jpeg"}));

  EntityHandle dir_light = scene_.CreateEntity("Directional light");
  dir_light.AddComponent<LightSourceComponent>(
      DirectionalLightSpecs(LightColorSpecs(glm::vec3(0), glm::vec3(0.9), glm::vec3(0.01))));
  dir_light.AddComponent<TransformComponent>(Transform(glm::vec3(0), glm::vec3(-0.5, 0, 0)));

  scene_.OnStart(event_dispatcher_);

  running_ = true;
  clock_t time_start = clock();
  while (running_) {
    float timestep = (0.0f + clock() - time_start) / CLOCKS_PER_SEC;
    time_start = clock();

    InputEventManager::TriggerEvents();

    uint32_t framebuffer_width  = preview_panel_.GetFramebuffer().GetFramebufferSpec().width;
    uint32_t framebuffer_height = preview_panel_.GetFramebuffer().GetFramebufferSpec().height;

    if (preview_panel_.Resized()) {
      scene_.OnViewportResize(framebuffer_width, framebuffer_height);
    }

    scene_.OnUpdate(timestep);

    preview_panel_.GetFramebuffer().Bind();
    Renderer3D::SetViewport(Viewport{0, 0, framebuffer_width, framebuffer_height});
    scene_.Render();
    preview_panel_.GetFramebuffer().Unbind();

    OnFrameStartImGui();
    OnGuiRender();
    OnFrameFinishImGui();

    window_.SetFPSToTitle(1 / timestep);
  }
}

void EditorApp::OnGuiRender() {
  preview_panel_.OnRender();
  entities_panel_.OnRender(scene_);
}

//================================================================
// Event Handlers
//================================================================
void EditorApp::OnQuit(const QuitEvent&) {
  running_ = false;
}

//================================================================
// ImGui Backend
//================================================================
void EditorApp::OnInitImGui() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  imgui_io_ = &ImGui::GetIO();
  imgui_io_->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  imgui_io_->ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
  // imgui_io_->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport / Platform Windows

  imgui_io_->FontDefault =
      imgui_io_->Fonts->AddFontFromFileTTF("res/fonts/Source_Sans_Pro/SourceSansPro-Regular.ttf", 18.0f);

  ImGui::StyleColorsLight();

  ImGuiStyle& style = ImGui::GetStyle();
  if (imgui_io_->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
  {
      style.WindowRounding = 0.0f;
      style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window_.GetNativeWindow(), true);
  ImGui_ImplOpenGL3_Init("#version 330");

  // TODO: (tralf-strues) Load fonts
}

void EditorApp::OnFrameStartImGui() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  static bool dockspace_open = true;
  static bool opt_fullscreen = true;
  static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

  ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
  if (opt_fullscreen)
  {
      const ImGuiViewport* viewport = ImGui::GetMainViewport();
      ImGui::SetNextWindowPos(viewport->WorkPos);
      ImGui::SetNextWindowSize(viewport->WorkSize);
      ImGui::SetNextWindowViewport(viewport->ID);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
      window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
      window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
  }
  else
  {
      dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
  }

  if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
      window_flags |= ImGuiWindowFlags_NoBackground;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("DockSpace Demo", &dockspace_open, window_flags);
  ImGui::PopStyleVar();

  if (opt_fullscreen)
      ImGui::PopStyleVar(2);

  // Submit the DockSpace
  ImGuiIO& io = ImGui::GetIO();
  if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
  {
      ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
      ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
  }

  ImGui::End();
}

void EditorApp::OnFrameFinishImGui() {
  assert(imgui_io_);

  ImGui::Render();
  int display_w, display_h;
  glfwGetFramebufferSize(window_.GetNativeWindow(), &display_w, &display_h);
  glViewport(0, 0, display_w, display_h);
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  if (imgui_io_->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
  {
      GLFWwindow* backup_current_context = glfwGetCurrentContext();
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
      glfwMakeContextCurrent(backup_current_context);
  }

  glfwSwapBuffers(window_.GetNativeWindow());
}

void EditorApp::OnCloseImGui() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}