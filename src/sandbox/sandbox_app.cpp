/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file sandbox_app.cpp
 * @date 2022-05-15
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

#include "sandbox_app.hpp"
#include "GLFW/glfw3.h"
#include "core/logger.hpp"
#include "glad/glad.h"
#include "renderer/3d/renderer3d.hpp"
#include "core/resource_manager.hpp"

using namespace vulture;

SandboxApp::SandboxApp() : window_(1280, 960) {}

int SandboxApp::Init() {
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    LOG_ERROR(SandboxApp, "Failed to initialize GLAD");
    return -1;
  }

  EventQueue::SetWindow(&window_);

  return 0;
}

class CameraMovementScript : public IScript {
 public:
  constexpr static float kSpeed = 10;

  virtual void OnUpdate(EntityHandle entity, float timestep) override {
    Transform* transform = &entity.GetComponent<TransformComponent>()->transform;

    glm::vec3 forward = transform->CalculateRotationMatrix() * glm::vec4(kDefaultForwardVector, 1);
    glm::vec3 right = transform->CalculateRotationMatrix() * glm::vec4(kDefaultRightVector, 1);

    float disp = kSpeed * timestep;

    if (Keyboard::Pressed(Keys::kWKey)) {
      transform->translation += disp * forward;
    } else if (Keyboard::Pressed(Keys::kSKey)) {
      transform->translation -= disp * forward;
    }

    if (Keyboard::Pressed(Keys::kDKey)) {
      transform->translation += disp * right;
    } else if (Keyboard::Pressed(Keys::kAKey)) {
      transform->translation -= disp * right;
    }
  }
};

void SandboxApp::Run() {
  int32_t frame_buffer_width = 0;
  int32_t frame_buffer_height = 0;
  glfwGetFramebufferSize(window_.GetNativeWindow(), &frame_buffer_width, &frame_buffer_height);

  float aspect_ratio = (float)frame_buffer_width / (float)frame_buffer_height;

  EntityHandle camera = scene_.CreateEntity();
  camera.AddComponent<CameraComponent>(PerspectiveCameraSpecs(aspect_ratio), true);
  camera.AddComponent<TransformComponent>(glm::vec3{10, 3, 10});
  camera.AddComponent<ScriptComponent>(new CameraMovementScript());

  EntityHandle street_lamp_light1 = scene_.CreateEntity();
  street_lamp_light1.AddComponent<LightSourceComponent>(PointLightSpecs(
      LightColorSpecs(glm::vec3(0.1), glm::vec3(0.4, 0.34, 0), glm::vec3(0.1)), LightAttenuationSpecs(3)));
  street_lamp_light1.AddComponent<TransformComponent>(glm::vec3(4, 3, 0));

  EntityHandle street_lamp_light2 = scene_.CreateEntity();
  street_lamp_light2.AddComponent<LightSourceComponent>(PointLightSpecs(
      LightColorSpecs(glm::vec3(0.1), glm::vec3(0.4, 0.2, 0.2), glm::vec3(0.1)), LightAttenuationSpecs(3)));
  street_lamp_light2.AddComponent<TransformComponent>(glm::vec3(-4, 3, 0));

  EntityHandle dir_light = scene_.CreateEntity();
  dir_light.AddComponent<LightSourceComponent>(
      DirectionalLightSpecs(LightColorSpecs(glm::vec3(0.2), glm::vec3(0.2), glm::vec3(0.1))));
  dir_light.AddComponent<TransformComponent>(Transform(glm::vec3(0), glm::vec3(-0.5, 0, 0)));

  EntityHandle spot_light = scene_.CreateEntity();
  spot_light.AddComponent<LightSourceComponent>(SpotLightSpecs(
      LightColorSpecs(glm::vec3(0.3), glm::vec3(0.3), glm::vec3(0)), LightAttenuationSpecs(2), cosf(0.2), cos(0.3)));
  spot_light.AddComponent<TransformComponent>(*camera.GetComponent<TransformComponent>());

  EntityHandle nk = scene_.CreateEntity();
  nk.AddComponent<MeshComponent>(ResourceManager::LoadMesh("res/meshes/nk.obj"));
  nk.AddComponent<TransformComponent>();

  EntityHandle watch_tower = scene_.CreateEntity();
  watch_tower.AddComponent<MeshComponent>(ResourceManager::LoadMesh("res/meshes/wooden_watch_tower.obj"));
  watch_tower.AddComponent<TransformComponent>();

  EntityHandle street_lamp1 = scene_.CreateEntity();
  street_lamp1.AddComponent<MeshComponent>(ResourceManager::LoadMesh("res/meshes/street_lamp.obj"));
  street_lamp1.AddComponent<TransformComponent>(Transform(glm::vec3(3, 0, 0), glm::vec3(0), glm::vec3(0.6)));

  EntityHandle street_lamp2 = scene_.CreateEntity();
  street_lamp2.AddComponent<MeshComponent>(ResourceManager::LoadMesh("res/meshes/street_lamp.obj"));
  street_lamp2.AddComponent<TransformComponent>(Transform(glm::vec3(-3, 0, 0), glm::vec3(0), glm::vec3(0.6)));

  // EntityHandle skybox = scene_.CreateEntity();
  // skybox.AddComponent<MeshComponent>(CreateSkyboxMesh({"res/textures/skybox_night_sky/skybox_night_sky_right.png",
  //                                                      "res/textures/skybox_night_sky/skybox_night_sky_left.png",
  //                                                      "res/textures/skybox_night_sky/skybox_night_sky_top.png",
  //                                                      "res/textures/skybox_night_sky/skybox_night_sky_bottom.png",
  //                                                      "res/textures/skybox_night_sky/skybox_night_sky_front.png",
  //                                                      "res/textures/skybox_night_sky/skybox_night_sky_back.png"}));
  // skybox.AddComponent<TransformComponent>();
  //
  // skybox_node_ = new ModelNode3D(CreateSkyboxMesh(
  //     {"res/textures/skybox_forest/skybox_forest_right.png", "res/textures/skybox_forest/skybox_forest_left.png",
  //      "res/textures/skybox_forest/skybox_forest_top.png", "res/textures/skybox_forest/skybox_forest_bottom.png",
  //      "res/textures/skybox_forest/skybox_forest_front.png", "res/textures/skybox_forest/skybox_forest_back.png"}));
  //
  // skybox_node_ = new ModelNode3D(CreateSkyboxMesh({"res/textures/skybox_ocean_sunset/skybox_ocean_sunset_right.png",
  //                                                   "res/textures/skybox_ocean_sunset/skybox_ocean_sunset_left.png",
  //                                                   "res/textures/skybox_ocean_sunset/skybox_ocean_sunset_top.png",
  //                                                   "res/textures/skybox_ocean_sunset/skybox_ocean_sunset_bottom.png",
  //                                                   "res/textures/skybox_ocean_sunset/skybox_ocean_sunset_front.png",
  //                                                   "res/textures/skybox_ocean_sunset/skybox_ocean_sunset_back.png"}));
  //
  // scene_.AddModel(skybox_node_);
  //
  // skybox_node_->transform.translation = scene_.GetMainCamera()->transform.translation;

  Renderer3D::Init();
  Renderer3D::SetViewport(
      Viewport{0, 0, static_cast<uint32_t>(frame_buffer_width), static_cast<uint32_t>(frame_buffer_height)});

  // FIXME:
  glfwSetInputMode(window_.GetNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPos(window_.GetNativeWindow(), frame_buffer_width / 2, frame_buffer_height / 2);

  Event event{};

  clock_t time_start = clock();
  while (running) {
    float timestep = (0.0f + clock() - time_start) / CLOCKS_PER_SEC;
    time_start = clock();

    while (PollEvent(&event)) {
      ProcessEvent(&event);
    }

    scene_.OnUpdate(timestep);
    scene_.Render();

    glfwSwapBuffers(window_.GetNativeWindow());
    window_.SetFPSToTitle(1 / timestep);
  }
}

void SandboxApp::ProcessEvent(Event* event) {
  assert(event);

  switch (event->GetType()) {
    case kQuit:
      running = false;
      break;

    case kKey:
      ProcessKeyEvent(event);
      break;

    case kMouseMove:
      ProcessMoveEvent(event);
      break;

    default:
      break;
  }
}

void SandboxApp::ProcessMoveEvent(Event* event) {
  assert(event);

  // static float prev_x = 0;
  // static float prev_y = 0;
  //
  // float dx = prev_x - event->GetMove().x;
  // float dy = prev_y - event->GetMove().y;
  //
  // scene_.GetMainCamera()->transform.rotation.y += 0.001f * dx;
  // scene_.GetMainCamera()->transform.rotation.x += 0.001f * dy;
  //
  // spot_light_node_->transform = scene_.GetMainCamera()->transform;
  // skybox_node_->transform.translation = scene_.GetMainCamera()->transform.translation;
  //
  // prev_x = event->GetMove().x;
  // prev_y = event->GetMove().y;
}

void SandboxApp::ProcessKeyEvent(Event* event) {
  assert(event);

  int key = event->GetKey().key;
  int action = (int)event->GetKey().action;

  // float speed = 0.5;
  //
  // glm::vec3 forward = scene_.GetMainCamera()->CalculateForwardVector();
  // glm::vec3 right = scene_.GetMainCamera()->CalculateRightVector();
  
  if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) {
    running = false;
  }

  // if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
  //   scene_.GetMainCamera()->transform.translation += speed * forward;
  // }
  //
  // if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
  //   scene_.GetMainCamera()->transform.translation -= speed * forward;
  // }
  //
  // if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
  //   scene_.GetMainCamera()->transform.translation -= speed * right;
  // }
  //
  // if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
  //   scene_.GetMainCamera()->transform.translation += speed * right;
  // }
  //
  // if (key == GLFW_KEY_Q && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
  //   scene_.GetMainCamera()->transform.translation.y -= 1;
  // }
  //
  // if (key == GLFW_KEY_E && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
  //   scene_.GetMainCamera()->transform.translation.y += 1;
  // }
  //
  // if (key == GLFW_KEY_F && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
  //   spot_light_node_->SetEnabled(!spot_light_node_->IsEnabled());
  // }
  //
  // if (key == GLFW_KEY_J && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
  //   directional_light_node_->SetEnabled(!directional_light_node_->IsEnabled());
  // }
  //
  // spot_light_node_->transform = scene_.GetMainCamera()->transform;
  // skybox_node_->transform.translation = scene_.GetMainCamera()->transform.translation;
}