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

// #include "audio/audio_device.hpp"
// #include "audio/audio_context.hpp"
// #include "audio/audio_source.hpp"
// #include "audio/audio_listener.hpp"
// #include "audio/buffer_manager.hpp"

using namespace vulture;

// FIXME: (tralf-strues) get rid of global variables!
bool running = true;
EntityHandle* spot_light{nullptr}; // FIXME: (tralf-strues)
EntityHandle* dir_light{nullptr};

vulture::Dispatcher dispatcher;

// AudioDevice* device;
// AudioContext* context;
// BufferManager* manager;
// AudioListener* listener;

SandboxApp::SandboxApp() : window_(1280, 960) {}

int SandboxApp::Init() {
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    LOG_ERROR(SandboxApp, "Failed to initialize GLAD");
    return -1;
  }

  InputEventManager::SetWindowAndDispatcher(&window_, &dispatcher);

  dispatcher.GetSink<KeyEvent>().Connect<&ProcessKeyEvent>();

  // device = new AudioDevice();
  // device->Open();

  // context = new AudioContext(device);
  // context->MakeCurrent();

  // listener = new AudioListener(context);
  // listener->MakeCurrent();
  // listener->SetVolume(1);
  
  // manager = new BufferManager();
  // manager->LoadAudioFile("res/sounds/woof.wav", "woof");
  // manager->LoadAudioFile("res/sounds/sci-fidrone.ogg", "noice");

  // context->CreateSource("woof_source");
  // context->CreateSource("noice_source");

  // vulture::AudioSource::Handle source_handle = context->GetSource("noice_source").value();
  // source_handle.SetBuf(manager->GetBuffer("noice").value());
  // source_handle.SetLocation({0, 0, 0});
  // source_handle.SetLooping(true);
  // source_handle.Play();

  // vulture::AudioSource::Handle source_handle2 = context->GetSource("woof_source").value();
  // source_handle2.SetBuf(manager->GetBuffer("woof").value());

  return 0;
}

SandboxApp::~SandboxApp() {
  {
    // vulture::AudioSource::Handle source_handle = context->GetSource("noice_source").value();
    // source_handle.Stop();
  }

  // delete listener;
  // delete context;
  // delete manager;

  // device->Close();
  // delete device;
}

class JumpEvent {};

class PlayerMovementScript : public IScript {
 public:
  constexpr static float kSpeed = 15;

  void SetSkybox(EntityHandle skybox) {
    skybox_ = new EntityHandle(skybox);
  }

  virtual void OnAttach(EntityHandle entity, Dispatcher& dispatcher) override {
    entity_ = new EntityHandle(entity);
    dispatcher.GetSink<MouseMoveEvent>().Connect<&PlayerMovementScript::OnMouseMove>(*this);
    dispatcher.GetSink<JumpEvent>().Connect<&PlayerMovementScript::OnJump>(*this);
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

    transform->translation.y = std::max(2.5f, transform->translation.y);

    // listener->SetLocation(transform->translation);

    skybox_->GetComponent<TransformComponent>()->transform.translation = transform->translation;
  }

  void OnMouseMove(const MouseMoveEvent& event) {
    static float prev_x = 0;
    static float prev_y = 0;
    
    float dx = prev_x - event.x;
    float dy = prev_y - event.y;

    Transform* transform = &entity_->GetComponent<TransformComponent>()->transform;
    rotation_y += 0.001f * dx;
    rotation_z += 0.001f * dy;
    transform->rotation = glm::quat(glm::vec3(rotation_z, rotation_y, 0));

    // listener->SetOrientation(transform->CalculateMatrix() * glm::vec4(kDefaultForwardVector, 1),
    //                          transform->CalculateMatrix() * glm::vec4(kDefaultUpVector, 1));

    prev_x = event.x;
    prev_y = event.y;
  }

  void OnJump(const JumpEvent&) {
    speed_ = glm::vec3(0, 4, 0);
  }

private:
  EntityHandle* entity_{nullptr}; // FIXME:
  EntityHandle* skybox_{nullptr};
  glm::vec3 speed_{0};
  float rotation_y{0}; // Yaw
  float rotation_z{0}; // Pitch
};

void SandboxApp::Run() {
  int32_t frame_buffer_width = 0;
  int32_t frame_buffer_height = 0;
  glfwGetFramebufferSize(window_.GetNativeWindow(), &frame_buffer_width, &frame_buffer_height);

  float aspect_ratio = (float)frame_buffer_width / (float)frame_buffer_height;

  EntityHandle guard = scene_.CreateEntity();
  guard.AddComponent<MeshComponent>(ResourceManager::LoadMesh("res/meshes/Low_poly_guard.obj"));
  guard.AddComponent<TransformComponent>(Transform(glm::vec3(5, 0, 0), glm::vec3(0), glm::vec3(3)));

  EntityHandle nk_normals = scene_.CreateEntity();
  nk_normals.AddComponent<MeshComponent>(ResourceManager::LoadMesh("res/meshes/nk_normals.obj"));
  nk_normals.AddComponent<TransformComponent>();

  EntityHandle nk = scene_.CreateEntity();
  nk.AddComponent<MeshComponent>(ResourceManager::LoadMesh("res/meshes/nk.obj"));
  nk.AddComponent<TransformComponent>(glm::vec3(26.336, 0, 0));

  EntityHandle brick_cubes = scene_.CreateEntity();
  brick_cubes.AddComponent<MeshComponent>(ResourceManager::LoadMesh("res/meshes/brick_cubes.obj"));
  brick_cubes.AddComponent<TransformComponent>(Transform(glm::vec3(-5, 2, 0), glm::vec3(0), glm::vec3(2)));

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

  #define CREATE_2LAMPS(NUM) \
    EntityHandle street_lamp##NUM = scene_.CreateEntity(); \
    street_lamp##NUM.AddComponent<MeshComponent>(ResourceManager::LoadMesh("res/meshes/street_lamp.obj")); \
    street_lamp##NUM.AddComponent<TransformComponent>(Transform(glm::vec3(9 - NUM * 4.55, 0, 12.5), glm::vec3(0), glm::vec3(0.6f))); \
    EntityHandle street_lamp##NUM##_symm = scene_.CreateEntity(); \
    street_lamp##NUM##_symm.AddComponent<MeshComponent>(ResourceManager::LoadMesh("res/meshes/street_lamp.obj")); \
    street_lamp##NUM##_symm.AddComponent<TransformComponent>(Transform(glm::vec3(9 - NUM * 4.55, 0, -12.5), glm::vec3(0), glm::vec3(0.6f))); \

  CREATE_2LAMPS(0)
  CREATE_2LAMPS(1)
  CREATE_2LAMPS(2)
  CREATE_2LAMPS(3)
  CREATE_2LAMPS(4)

  #undef CREATE_2LAMPS

  #define CREATE_2LIGHTS(NUM) \
    EntityHandle lamp_light_##NUM = scene_.CreateChildEntity(street_lamp##NUM); \
    lamp_light_##NUM.AddComponent<LightSourceComponent>(PointLightSpecs( \
        LightColorSpecs(glm::vec3(0.005), glm::vec3(0.5, 0.5, 0.2), glm::vec3(0.05)), LightAttenuationSpecs(10))); \
    lamp_light_##NUM.AddComponent<TransformComponent>(glm::vec3(0, 8, 0)); \
    EntityHandle lamp_light_##NUM##_symm = scene_.CreateChildEntity(street_lamp##NUM##_symm); \
    lamp_light_##NUM##_symm.AddComponent<LightSourceComponent>(PointLightSpecs( \
        LightColorSpecs(glm::vec3(0.005), glm::vec3(0.5, 0.5, 0.2), glm::vec3(0.05)), LightAttenuationSpecs(10))); \
    lamp_light_##NUM##_symm.AddComponent<TransformComponent>(glm::vec3(0, 8, 0));

  CREATE_2LIGHTS(0)
  CREATE_2LIGHTS(1)
  CREATE_2LIGHTS(2)
  CREATE_2LIGHTS(3)
  CREATE_2LIGHTS(4)
  
  #undef CREATE_2LAMPS

  EntityHandle dog = scene_.CreateEntity();
  dog.AddComponent<MeshComponent>(ResourceManager::LoadMesh("res/meshes/dog.obj"));
  dog.AddComponent<TransformComponent>(Transform(glm::vec3(0, 0, 0), glm::vec3(0), glm::vec3(0.5, 0.4, 0.4)));
  // PlayerMovementScript* dog_movement = new PlayerMovementScript;
  // dispatcher.GetSink<JumpEvent>().Connect<&PlayerMovementScript::OnJump>(*dog_movement);
  // dog.AddComponent<ScriptComponent>(dog_movement);

  /*
  EntityHandle statue1 = scene_.CreateEntity();
  statue1.AddComponent<MeshComponent>(ResourceManager::LoadMesh("res/meshes/statue.obj"));
  statue1.AddComponent<TransformComponent>(Transform(glm::vec3(-6, 0, 8), glm::vec3(0, glm::radians(240.0f), 0), glm::vec3(1)));

  EntityHandle statue2 = scene_.CreateEntity();
  statue2.AddComponent<MeshComponent>(ResourceManager::LoadMesh("res/meshes/statue.obj"));
  statue2.AddComponent<TransformComponent>(Transform(glm::vec3(-10, 0, 2), glm::vec3(0, glm::radians(250.0f), 0), glm::vec3(1)));

  EntityHandle statue3 = scene_.CreateEntity();
  statue3.AddComponent<MeshComponent>(ResourceManager::LoadMesh("res/meshes/statue.obj"));
  statue3.AddComponent<TransformComponent>(Transform(glm::vec3(-12, 0, 10), glm::vec3(0, glm::radians(150.0f), 0), glm::vec3(1)));
  */

  // EntityHandle point_light = scene_.CreateEntity();
  // point_light.AddComponent<TransformComponent>();
  // point_light.AddComponent<LightSourceComponent>(PointLightSpecs(
  //     LightColorSpecs(glm::vec3(0.1), glm::vec3(0.3, 0.3, 0), glm::vec3(0.01)), LightAttenuationSpecs(15)));

  EntityHandle camera = scene_.CreateEntity();
  camera.AddComponent<CameraComponent>(PerspectiveCameraSpecs(aspect_ratio), true);
  // camera.AddComponent<TransformComponent>(Transform(glm::vec3{0, 7, 10}, glm::angleAxis(-0.3f, glm::vec3(1.0f, 0.0f, 0.0f))));
  camera.AddComponent<TransformComponent>(glm::vec3(0, 3, -3));
  PlayerMovementScript* camera_movement = new PlayerMovementScript();
  camera.AddComponent<ScriptComponent>(camera_movement);

  EntityHandle skybox = scene_.CreateEntity();
  skybox.AddComponent<MeshComponent>(CreateSkyboxMesh({"res/textures/skybox_morning_field/skybox_morning_field_right.jpeg",
                                                       "res/textures/skybox_morning_field/skybox_morning_field_left.jpeg",
                                                       "res/textures/skybox_morning_field/skybox_morning_field_top.jpeg",
                                                       "res/textures/skybox_morning_field/skybox_morning_field_bottom.jpeg",
                                                       "res/textures/skybox_morning_field/skybox_morning_field_front.jpeg",
                                                       "res/textures/skybox_morning_field/skybox_morning_field_back.jpeg"}));
  skybox.AddComponent<TransformComponent>();
  skybox.GetComponent<TransformComponent>()->transform.scale = glm::vec3(100);
  camera_movement->SetSkybox(skybox);

  dir_light = new EntityHandle(scene_.CreateEntity());
  dir_light->AddComponent<LightSourceComponent>(
      DirectionalLightSpecs(LightColorSpecs(glm::vec3(0), glm::vec3(0.9), glm::vec3(0.01))));
  dir_light->AddComponent<TransformComponent>(Transform(glm::vec3(0), glm::vec3(-0.5, 0, 0)));

  spot_light = new EntityHandle(scene_.CreateChildEntity(camera));
  spot_light->AddComponent<LightSourceComponent>(SpotLightSpecs(
      LightColorSpecs(glm::vec3(0, 0, 0), glm::vec3(0.5, 0.5, 0.25), glm::vec3(0)), LightAttenuationSpecs(100), cosf(0.3), cos(0.5)));
  spot_light->AddComponent<TransformComponent>(glm::vec3(0, 0, 0));

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
// andle = context->GetSource("noice_source").value();
    // source_handle.Stop();
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
    // context->GetSource("woof_source").value().Play();
    //insert sound here
  }

  if (key == GLFW_KEY_F && action == GLFW_PRESS) {
    spot_light->GetComponent<LightSourceComponent>()->runtime_node->SetEnabled(
        !spot_light->GetComponent<LightSourceComponent>()->runtime_node->IsEnabled());
  }

  if (key == GLFW_KEY_G && action == GLFW_PRESS) {
    dir_light->GetComponent<LightSourceComponent>()->runtime_node->SetEnabled(
        !dir_light->GetComponent<LightSourceComponent>()->runtime_node->IsEnabled());
  }
}