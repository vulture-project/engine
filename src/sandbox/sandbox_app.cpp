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

#include "audio/AudioDevice.h"
#include "audio/AudioSource.h"

using namespace vulture;

bool running = true;

vulture::Dispatcher dispatcher;

using namespace sound;

AudioDevice device;
AudioContext* context;
sound::Source* source_woof;
AudioBuffer* buffer_woof;

SandboxApp::SandboxApp() : window_(1280, 960) {}

int SandboxApp::Init() {
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    LOG_ERROR(SandboxApp, "Failed to initialize GLAD");
    return -1;
  }

  InputEventManager::SetWindowAndDispatcher(&window_, &dispatcher);

  dispatcher.GetSink<KeyEvent>().Connect<&ProcessKeyEvent>();


  device.Open();
  context = device.CreateContext();
  context->MakeCurrentPlaying();
  source_woof = context->CreateSource();
  buffer_woof = new AudioBuffer("res/sounds/woof.wav");
  source_woof->SetBuf(buffer_woof);

  return 0;
}

SandboxApp::~SandboxApp() {
  source_woof->ReleaseBuf();

  delete buffer_woof;

  context->DestroySource(source_woof);
  device.DestroyContext(context);
  device.Close();
};

class JumpEvent {};

glm::vec3 speed{0, 0, 0};

class PlayerMovementScript : public IScript {
 public:
  constexpr static float kSpeed = 15;

  void OnMouseMove(const MouseMoveEvent& event) {
    static float prev_x = 0;
    static float prev_y = 0;
    
    float dx = prev_x - event.x;
    float dy = prev_y - event.y;

    Transform* transform = &entity_->GetComponent<TransformComponent>()->transform;
    transform->rotation.y += 0.001f * dx;
    // transform->rotation.x += 0.001f * dy;
    
    prev_x = event.x;
    prev_y = event.y;
  }

  virtual void OnAttach(EntityHandle entity, Dispatcher& dispatcher) override {
    entity_ = new EntityHandle(entity);
    dispatcher.GetSink<MouseMoveEvent>().Connect<&PlayerMovementScript::OnMouseMove>(*this);
  }

  virtual void OnUpdate(float timestep) override {
    Transform* transform = &entity_->GetComponent<TransformComponent>()->transform;

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

    speed -= glm::vec3(0, 10, 0) * timestep;
    transform->translation += speed * timestep;

    transform->translation.y = std::max(0.0f, transform->translation.y);
  }

  void OnJump(const JumpEvent&) {
    speed = glm::vec3(0, 4, 0);
  }

private:
  EntityHandle* entity_{nullptr}; // FIXME:
};

void SandboxApp::Run() {
  int32_t frame_buffer_width = 0;
  int32_t frame_buffer_height = 0;
  glfwGetFramebufferSize(window_.GetNativeWindow(), &frame_buffer_width, &frame_buffer_height);

  float aspect_ratio = (float)frame_buffer_width / (float)frame_buffer_height;

  EntityHandle nk = scene_.CreateEntity();
  nk.AddComponent<MeshComponent>(ResourceManager::LoadMesh("res/meshes/nk.obj"));
  nk.AddComponent<TransformComponent>();

  EntityHandle watch_tower = scene_.CreateEntity();
  // watch_tower.AddComponent<MeshComponent>(ResourcseManager::LoadMesh("res/meshes/wooden_watch_tower.obj"));
  watch_tower.AddComponent<TransformComponent>();

  EntityHandle bench = scene_.CreateEntity();
  bench.AddComponent<MeshComponent>(ResourceManager::LoadMesh("res/meshes/bench.obj"));
  bench.AddComponent<TransformComponent>(Transform(glm::vec3(1, 0, 0), glm::vec3(0), glm::vec3(1)));

  EntityHandle dog = scene_.CreateEntity();
  dog.AddComponent<MeshComponent>(ResourceManager::LoadMesh("res/meshes/dog.obj"));
  dog.AddComponent<TransformComponent>(Transform(glm::vec3(8, 0, 0), glm::vec3(0), glm::vec3(0.5, 0.4, 0.4)));
  PlayerMovementScript* dog_movement = new PlayerMovementScript;
  dispatcher.GetSink<JumpEvent>().Connect<&PlayerMovementScript::OnJump>(*dog_movement);
  dog.AddComponent<ScriptComponent>(dog_movement);

  EntityHandle camera = scene_.CreateChildEntity(dog);
  camera.AddComponent<CameraComponent>(PerspectiveCameraSpecs(aspect_ratio), true);
  camera.AddComponent<TransformComponent>(Transform(glm::vec3{0, 7, 8}, glm::vec3(-0.3, 0, 0)));

  EntityHandle street_lamp1 = scene_.CreateEntity();
  street_lamp1.AddComponent<MeshComponent>(ResourceManager::LoadMesh("res/meshes/street_lamp.obj"));
  street_lamp1.AddComponent<TransformComponent>(Transform(glm::vec3(3, 0, 0), glm::vec3(0), glm::vec3(0.6f)));

  EntityHandle street_lamp2 = scene_.CreateEntity();
  street_lamp2.AddComponent<MeshComponent>(ResourceManager::LoadMesh("res/meshes/street_lamp.obj"));
  street_lamp2.AddComponent<TransformComponent>(Transform(glm::vec3(-3, 0, 0), glm::vec3(0), glm::vec3(0.6f)));

  EntityHandle skybox = scene_.CreateChildEntity(camera);
  skybox.AddComponent<MeshComponent>(CreateSkyboxMesh({"res/textures/skybox_night_sky/skybox_night_sky_right.png",
                                                       "res/textures/skybox_night_sky/skybox_night_sky_left.png",
                                                       "res/textures/skybox_night_sky/skybox_night_sky_top.png",
                                                       "res/textures/skybox_night_sky/skybox_night_sky_bottom.png",
                                                       "res/textures/skybox_night_sky/skybox_night_sky_front.png",
                                                       "res/textures/skybox_night_sky/skybox_night_sky_back.png"}));
  skybox.AddComponent<TransformComponent>();

  EntityHandle street_lamp_light1 = scene_.CreateChildEntity(street_lamp1);
  street_lamp_light1.AddComponent<LightSourceComponent>(PointLightSpecs(
      LightColorSpecs(glm::vec3(0.1), glm::vec3(0.4, 0.34, 0), glm::vec3(0.1)), LightAttenuationSpecs(3)));
  street_lamp_light1.AddComponent<TransformComponent>(glm::vec3(0, 5.3, 0));

  EntityHandle street_lamp_light2 = scene_.CreateChildEntity(street_lamp2);
  street_lamp_light2.AddComponent<LightSourceComponent>(PointLightSpecs(
      LightColorSpecs(glm::vec3(0.1), glm::vec3(0.4, 0.2, 0.2), glm::vec3(0.1)), LightAttenuationSpecs(3)));
  street_lamp_light2.AddComponent<TransformComponent>(glm::vec3(0, 5.3, 0));

  EntityHandle dir_light = scene_.CreateEntity();
  dir_light.AddComponent<LightSourceComponent>(
      DirectionalLightSpecs(LightColorSpecs(glm::vec3(0.2), glm::vec3(0.2), glm::vec3(0.1))));
  dir_light.AddComponent<TransformComponent>(Transform(glm::vec3(0), glm::vec3(-0.5, 0, 0)));

  EntityHandle spot_light = scene_.CreateChildEntity(camera);
  spot_light.AddComponent<LightSourceComponent>(SpotLightSpecs(
      LightColorSpecs(glm::vec3(0.3), glm::vec3(0.3), glm::vec3(0)), LightAttenuationSpecs(2), cosf(0.2), cos(0.3)));
  spot_light.AddComponent<TransformComponent>();

  Renderer3D::Init();
  Renderer3D::SetViewport(
      Viewport{0, 0, static_cast<uint32_t>(frame_buffer_width), static_cast<uint32_t>(frame_buffer_height)});

  // FIXME:
  glfwSetInputMode(window_.GetNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPos(window_.GetNativeWindow(), frame_buffer_width / 2, frame_buffer_height / 2);

  scene_.OnStart(dispatcher);

  clock_t time_start = clock();
  while (running) {
    float timestep = (0.0f + clock() - time_start) / CLOCKS_PER_SEC;
    time_start = clock();

    InputEventManager::TriggerEvents();

    scene_.OnUpdate(timestep);
    scene_.Render();

    glfwSwapBuffers(window_.GetNativeWindow());
    window_.SetFPSToTitle(1 / timestep);
  }
}

void ProcessKeyEvent(const vulture::KeyEvent& event) {
  int key = event.key;
  int action = (int)event.action;

  if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) {
    running = false;
  }

  if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) {
    dispatcher.Trigger<JumpEvent>();
  }

  if (key == GLFW_KEY_E && action == GLFW_PRESS) {
    source_woof->Play();
  }
}