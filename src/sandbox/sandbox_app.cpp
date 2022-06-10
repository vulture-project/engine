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

bool running = true;

vulture::Dispatcher dispatcher;

SandboxApp::SandboxApp() : window_(1280, 960) {}

int SandboxApp::Init() {
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    LOG_ERROR(SandboxApp, "Failed to initialize GLAD");
    return -1;
  }

  InputEventManager::SetWindowAndDispatcher(&window_, &dispatcher);

  dispatcher.GetSink<KeyEvent>().Connect<&ProcessKeyEvent>();
  return 0;
}

class JumpEvent {};

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

    speed_ -= glm::vec3(0, 10, 0) * timestep;
    transform->translation += speed_ * timestep;

    transform->translation.y = std::max(0.0f, transform->translation.y);
  }

  void OnJump(const JumpEvent&) {
    speed_ = glm::vec3(0, 4, 0);
  }

private:
  glm::vec3 speed{0, 0, 0};
  EntityHandle* entity_{nullptr}; // FIXME:
  glm::vec3 speed_{0};
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

  double bench_x_coord = 11.4;
  #define CREATE_2BENCHES(NUM) \
    EntityHandle bench##NUM = scene_.CreateEntity(); \
    bench##NUM.AddComponent<MeshComponent>(ResourceManager::LoadMesh("res/meshes/bench.obj")); \
    bench##NUM.AddComponent<TransformComponent>(Transform(glm::vec3(bench_x_coord, 0.1, 12.2), glm::vec3(0, M_PI, 0), glm::vec3(1))); \
    EntityHandle bench_##NUM##_symm = scene_.CreateEntity(); \
    bench_##NUM##_symm.AddComponent<MeshComponent>(ResourceManager::LoadMesh("res/meshes/bench.obj")); \
    bench_##NUM##_symm.AddComponent<TransformComponent>(Transform(glm::vec3(bench_x_coord, 0.1, -12.2), glm::vec3(0), glm::vec3(1))); \
    bench_x_coord -= 4.57;

  CREATE_2BENCHES(0)
  CREATE_2BENCHES(1)
  CREATE_2BENCHES(2)
  CREATE_2BENCHES(3)
  CREATE_2BENCHES(4)
  CREATE_2BENCHES(5)

  #undef CREATE_2BENCHES

  double lamp_x_coord = 9;
  #define CREATE_2LAMPS(NUM) \
    EntityHandle street_lamp##NUM = scene_.CreateEntity(); \
    street_lamp##NUM.AddComponent<MeshComponent>(ResourceManager::LoadMesh("res/meshes/street_lamp.obj")); \
    street_lamp##NUM.AddComponent<TransformComponent>(Transform(glm::vec3(lamp_x_coord, 0, 12.5), glm::vec3(0), glm::vec3(0.6f))); \
    EntityHandle street_lamp##NUM##_symm = scene_.CreateEntity(); \
    street_lamp##NUM##_symm.AddComponent<MeshComponent>(ResourceManager::LoadMesh("res/meshes/street_lamp.obj")); \
    street_lamp##NUM##_symm.AddComponent<TransformComponent>(Transform(glm::vec3(lamp_x_coord, 0, -12.5), glm::vec3(0), glm::vec3(0.6f))); \
    lamp_x_coord -= 4.55;

  CREATE_2LAMPS(0)
  CREATE_2LAMPS(1)
  CREATE_2LAMPS(2)
  CREATE_2LAMPS(3)
  CREATE_2LAMPS(4)

  #undef CREATE_2LAMPS

  #define CREATE_2LIGHTS(NUM) \
    EntityHandle lamp_light_##NUM = scene_.CreateChildEntity(street_lamp##NUM); \
    lamp_light_##NUM.AddComponent<LightSourceComponent>(PointLightSpecs( \
        LightColorSpecs(glm::vec3(0.1), glm::vec3(0.3, 0.3, 0), glm::vec3(0.01)), LightAttenuationSpecs(15))); \
    lamp_light_##NUM.AddComponent<TransformComponent>(glm::vec3(2, 8, 2)); \
    EntityHandle lamp_light_##NUM##_symm = scene_.CreateChildEntity(street_lamp##NUM##_symm); \
    lamp_light_##NUM##_symm.AddComponent<LightSourceComponent>(PointLightSpecs( \
        LightColorSpecs(glm::vec3(0.1), glm::vec3(0.3, 0.3, 0), glm::vec3(0.01)), LightAttenuationSpecs(15))); \
    lamp_light_##NUM##_symm.AddComponent<TransformComponent>(glm::vec3(2, 8, 2));

  CREATE_2LIGHTS(0)
  CREATE_2LIGHTS(1)
  CREATE_2LIGHTS(2)
  CREATE_2LIGHTS(3)
  CREATE_2LIGHTS(4)
  
  #undef CREATE_2LAMPS

  EntityHandle dog = scene_.CreateEntity();
  dog.AddComponent<MeshComponent>(ResourceManager::LoadMesh("res/meshes/dog.obj"));
  dog.AddComponent<TransformComponent>(Transform(glm::vec3(0, 0, 0), glm::vec3(0), glm::vec3(0.5, 0.4, 0.4)));
  PlayerMovementScript* dog_movement = new PlayerMovementScript;
  dispatcher.GetSink<JumpEvent>().Connect<&PlayerMovementScript::OnJump>(*dog_movement);
  dog.AddComponent<ScriptComponent>(dog_movement);

  EntityHandle statue = scene_.CreateEntity();
  statue.AddComponent<MeshComponent>(ResourceManager::LoadMesh("res/meshes/statue.obj"));
  statue.AddComponent<TransformComponent>(Transform(glm::vec3(2, 0, 2), glm::vec3(0), glm::vec3(1)));

  EntityHandle camera = scene_.CreateChildEntity(dog);
  camera.AddComponent<CameraComponent>(PerspectiveCameraSpecs(aspect_ratio), true);
  camera.AddComponent<TransformComponent>(Transform(glm::vec3{0, 7, 8}, glm::vec3(-0.3, 0, 0)));

  EntityHandle skybox = scene_.CreateChildEntity(camera);
  skybox.AddComponent<MeshComponent>(CreateSkyboxMesh({"res/textures/skybox_night_sky/skybox_night_sky_right.png",
                                                       "res/textures/skybox_night_sky/skybox_night_sky_left.png",
                                                       "res/textures/skybox_night_sky/skybox_night_sky_top.png",
                                                       "res/textures/skybox_night_sky/skybox_night_sky_bottom.png",
                                                       "res/textures/skybox_night_sky/skybox_night_sky_front.png",
                                                       "res/textures/skybox_night_sky/skybox_night_sky_back.png"}));
  skybox.AddComponent<TransformComponent>();

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
}