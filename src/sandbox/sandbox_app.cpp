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

#include <glad/glad.h>
#include <glfw/glfw3.h>

#include "sandbox_app.hpp"
#include "core/logger.hpp"
#include "renderer/3d/renderer3d.hpp"
#include "resource_loaders/parse_obj.hpp"

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

void SandboxApp::Run() {
  int32_t frameBufferWidth = 0;
  int32_t frameBufferHeight = 0;
  glfwGetFramebufferSize(window_.GetNativeWindow(), &frameBufferWidth, &frameBufferHeight);

  CameraNode3D* cameraNode = new CameraNode3D(
      PerspectiveCameraSpecs((float)frameBufferWidth / (float)frameBufferHeight), Transform(glm::vec3{10, 3, -3}));

  scene_.AddCamera(cameraNode);
  scene_.SetMainCamera(cameraNode);

  scene_.AddLightSource(
      new LightSourceNode3D(PointLightSpecs(LightColorSpecs(glm::vec3{0.1}, glm::vec3{0.4, 0.34, 0}, glm::vec3{0.1}),
                                            LightAttenuationSpecs(3)),
                            Transform(glm::vec3{4, 3, 0})));

  scene_.AddLightSource(
      new LightSourceNode3D(PointLightSpecs(LightColorSpecs(glm::vec3{0.1}, glm::vec3{0.4, 0.2, 0.2}, glm::vec3{0.1}),
                                            LightAttenuationSpecs(3)),
                            Transform(glm::vec3{-4, 3, 0})));

  directional_light_node_ =
      new LightSourceNode3D(DirectionalLightSpecs(LightColorSpecs(glm::vec3{0.2}, glm::vec3{0.2}, glm::vec3{0.1})),
                            Transform(glm::vec3{0}, glm::vec3{-0.5, 0, 0}));
  scene_.AddLightSource(directional_light_node_);

  spot_light_node_ = new LightSourceNode3D(SpotLightSpecs(LightColorSpecs(glm::vec3{0.3}, glm::vec3{0.3}, glm::vec3{0}),
                                                          LightAttenuationSpecs(2), cosf(0.2), cos(0.3)));
  scene_.AddLightSource(spot_light_node_);

  scene_.AddModel(new ModelNode3D(ParseMeshWavefront("res/meshes/nk.obj")));

  scene_.AddModel(
      new ModelNode3D(ParseMeshWavefront("res/meshes/wooden_watch_tower.obj"), Transform(glm::vec3{0, -0.75, 0})));

  scene_.AddModel(new ModelNode3D(ParseMeshWavefront("res/meshes/street_lamp.obj"),
                                  Transform(glm::vec3{3, 0, 0}, glm::vec3{0}, glm::vec3{0.6})));

  scene_.AddModel(new ModelNode3D(ParseMeshWavefront("res/meshes/street_lamp.obj"),
                                  Transform(glm::vec3{-3, 0, 0}, glm::vec3{0}, glm::vec3{0.6})));

  // skybox_node_ = new ModelNode3D(CreateSkyboxMesh(
  //     {"res/textures/skybox_forest/skybox_forest_right.png", "res/textures/skybox_forest/skybox_forest_left.png",
  //      "res/textures/skybox_forest/skybox_forest_top.png", "res/textures/skybox_forest/skybox_forest_bottom.png",
  //      "res/textures/skybox_forest/skybox_forest_front.png", "res/textures/skybox_forest/skybox_forest_back.png"}));

  // skybox_node_ = new ModelNode3D(CreateSkyboxMesh({"res/textures/skybox_ocean_sunset/skybox_ocean_sunset_right.png",
  //                                                   "res/textures/skybox_ocean_sunset/skybox_ocean_sunset_left.png",
  //                                                   "res/textures/skybox_ocean_sunset/skybox_ocean_sunset_top.png",
  //                                                   "res/textures/skybox_ocean_sunset/skybox_ocean_sunset_bottom.png",
  //                                                   "res/textures/skybox_ocean_sunset/skybox_ocean_sunset_front.png",
  //                                                   "res/textures/skybox_ocean_sunset/skybox_ocean_sunset_back.png"}));

  skybox_node_ = new ModelNode3D(CreateSkyboxMesh({"res/textures/skybox_night_sky/skybox_night_sky_right.png",
                                                   "res/textures/skybox_night_sky/skybox_night_sky_left.png",
                                                   "res/textures/skybox_night_sky/skybox_night_sky_top.png",
                                                   "res/textures/skybox_night_sky/skybox_night_sky_bottom.png",
                                                   "res/textures/skybox_night_sky/skybox_night_sky_front.png",
                                                   "res/textures/skybox_night_sky/skybox_night_sky_back.png"}));

  scene_.AddModel(skybox_node_);

  skybox_node_->transform.translation = scene_.GetMainCamera()->transform.translation;

  Renderer3D::Init();
  Renderer3D::SetViewport(
      Viewport{0, 0, static_cast<uint32_t>(frameBufferWidth), static_cast<uint32_t>(frameBufferHeight)});

  // FIXME:
  glfwSetInputMode(window_.GetNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPos(window_.GetNativeWindow(), frameBufferWidth / 2, frameBufferHeight / 2);

  Event event{};
  bool running = true;

  while (running) {
    while (PollEvent(&event)) {
      ProcessEvent(&event, &running);
    }

    Renderer3D::RenderScene(&scene_);
    glfwSwapBuffers(window_.GetNativeWindow());
  }
}

void SandboxApp::ProcessEvent(Event* event, bool* running) {
  assert(event);
  assert(running);

  switch (event->GetType()) {
    case kQuit:
      *running = false;
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

  static float prev_x = 0;
  static float prev_y = 0;

  float dx = prev_x - event->GetMove().x;
  float dy = prev_y - event->GetMove().y;

  scene_.GetMainCamera()->transform.rotation.y += 0.001f * dx;
  scene_.GetMainCamera()->transform.rotation.x += 0.001f * dy;

  spot_light_node_->transform = scene_.GetMainCamera()->transform;
  skybox_node_->transform.translation = scene_.GetMainCamera()->transform.translation;

  prev_x = event->GetMove().x;
  prev_y = event->GetMove().y;
}

void SandboxApp::ProcessKeyEvent(Event* event) {
  assert(event);

  int key = event->GetKey().key;
  int action = (int)event->GetKey().action;

  float speed = 0.5;

  glm::vec3 forward = scene_.GetMainCamera()->CalculateForwardVector();
  glm::vec3 right = scene_.GetMainCamera()->CalculateRightVector();

  if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    scene_.GetMainCamera()->transform.translation += speed * forward;
  }

  if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    scene_.GetMainCamera()->transform.translation -= speed * forward;
  }

  if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    scene_.GetMainCamera()->transform.translation -= speed * right;
  }

  if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    scene_.GetMainCamera()->transform.translation += speed * right;
  }

  if (key == GLFW_KEY_Q && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    scene_.GetMainCamera()->transform.translation.y -= 1;
  }

  if (key == GLFW_KEY_E && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    scene_.GetMainCamera()->transform.translation.y += 1;
  }

  if (key == GLFW_KEY_F && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    spot_light_node_->SetEnabled(!spot_light_node_->IsEnabled());
  }

  if (key == GLFW_KEY_J && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    directional_light_node_->SetEnabled(!directional_light_node_->IsEnabled());
  }

  spot_light_node_->transform = scene_.GetMainCamera()->transform;
  skybox_node_->transform.translation = scene_.GetMainCamera()->transform.translation;
}