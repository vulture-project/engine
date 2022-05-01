/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file main.cpp
 * @date 2022-04-26
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

#include <GLFW/glfw3.h>
#include <alloca.h>
#include <glad/glad.h>

#include <fstream>
#include <sstream>
#include <string>

// #include "ECS/component_factory.h"
// #include "ECS/entity_factory.h"
// #include "ECS/entity_manager.h"
// #include "components/components.hpp"
#include "platform/event.hpp"
#include "platform/window.hpp"
#include "renderer/renderer3d.hpp"
#include "renderer/resource_loader.hpp"
#include "renderer/scene.hpp"

using namespace input;

Scene g_Scene;

constexpr PointLightSpecs kPointLightSpecs = {glm::vec3{1, 0, 0}, glm::vec3{0.5}, glm::vec3{1, 1, 1}};

void ProcessMoveEvent(Event* event) {
  assert(event);

  static float prev_x = 0;
  static float prev_y = 0;

  float dx = prev_x - event->GetMove().x;
  float dy = prev_y - event->GetMove().y;
  // std::cout << dx << ' ' << dy << '\n';

  // CameraComponent* cameraComponent = g_Scene.camera->GetComponent<CameraComponent>();

  // cameraComponent->forward = glm::normalize(glm::rotate(glm::identity<glm::mat4>(), 0.001f * dx, glm::vec3{0, 1, 0})
  // *
  //                                           glm::vec4(cameraComponent->forward, 1));
  // cameraComponent->forward.y += 0.001f * dy;
  // cameraComponent->forward = glm::normalize(cameraComponent->forward);

  g_Scene.camera->forward =
      glm::normalize(glm::rotate(glm::identity<glm::mat4>(), 0.001f * dx, glm::vec3{0, 1, 0}) * glm::vec4(g_Scene.camera->forward, 1));

  g_Scene.camera->forward.y += 0.001f * dy;
  g_Scene.camera->forward = glm::normalize(g_Scene.camera->forward);

  prev_x = event->GetMove().x;
  prev_y = event->GetMove().y;
}

void ProcessKeyEvent(Event* event) {
  assert(event);

  int key = event->GetKey().key;
  int action = (int)event->GetKey().action;

  float speed = 0.5;

  // CameraComponent* cameraComponent = g_Scene.camera->GetComponent<CameraComponent>();
  // TransformComponent* transformComponent = g_Scene.camera->GetComponent<TransformComponent>();

  if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    // transformComponent->translation += speed * cameraComponent->forward;
    g_Scene.camera->pos += speed * g_Scene.camera->forward;
  }

  if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    // transformComponent->translation -= speed * cameraComponent->forward;
    g_Scene.camera->pos -= speed * g_Scene.camera->forward;
  }

  if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    // transformComponent->translation -= speed * glm::cross(cameraComponent->forward, glm::vec3{0, 1, 0});
    g_Scene.camera->pos -= speed * glm::cross(g_Scene.camera->forward, glm::vec3{0, 1, 0});
  }

  if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    // transformComponent->translation += speed * glm::cross(cameraComponent->forward, glm::vec3{0, 1, 0});
    g_Scene.camera->pos += speed * glm::cross(g_Scene.camera->forward, glm::vec3{0, 1, 0});
  }

  if (key == GLFW_KEY_Q && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    // transformComponent->translation.y += 1;
    g_Scene.camera->pos.y += 1;
  }

  if (key == GLFW_KEY_E && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    // transformComponent->translation.y -= 1;
    g_Scene.camera->pos.y -= 1;
  }
}

void ProcessEvent(Event* event, bool* running) {
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

int main() {
  Window _window{};
  EventQueue::SetWindow(&_window);
  NativeWindow* window = _window.GetNativeWindow();

  int32_t frameBufferWidth = 0;
  int32_t frameBufferHeight = 0;
  glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  // ecs::EntityFactory entity_factory;
  // ecs::EntityManager entity_manager;
  // ecs::ComponentFactory component_factory;

  // g_Scene.camera = entity_factory.CreateEntity<ecs::Entity>();
  // entity_manager.AddEntity(g_Scene.camera);
  // g_Scene.camera->AddComponent(component_factory.CreateComponent<TransformComponent>(10, 5, 10));
  // g_Scene.camera->AddComponent(component_factory.CreateComponent<CameraComponent>(glm::vec3{-1, -0.5, -1}));

  // ecs::Entity* light = entity_factory.CreateEntity<ecs::Entity>();
  // entity_manager.AddEntity(light);
  // light->AddComponent(component_factory.CreateComponent<TransformComponent>(10, 0, 15));
  // light->AddComponent(component_factory.CreateComponent<PointLightComponent>(glm::vec3{0.5}, glm::vec3{0.5},
  // glm::vec3{0.5})); g_Scene.light_sources.push_back(light);

  // ecs::Entity* skameiki = entity_factory.CreateEntity<ecs::Entity>();
  // entity_manager.AddEntity(skameiki);
  // skameiki->AddComponent(component_factory.CreateComponent<TransformComponent>(0, 0, 0));
  // skameiki->AddComponent(component_factory.CreateComponent<MeshComponent>(CreateShared<Mesh>(ParseMeshObj("res/meshes/skameiki.obj"))));

  // g_Scene.root = new SceneNode(skameiki);
  // g_Scene.root->AddChild(new SceneNode(g_Scene.camera));
  // g_Scene.root->AddChild(new SceneNode(light));

  g_Scene.camera = CreateShared<CameraNode>(PerspectiveCameraSpecs((float)frameBufferWidth / (float)frameBufferHeight),
                                            glm::vec3{10, 5, 10}, glm::vec3{-1, -0.5, -1});

  g_Scene.light = CreateShared<PointLightNode>(kPointLightSpecs, glm::vec3{10, 0, 15});
  g_Scene.meshes.emplace_back(ParseMeshObj("res/meshes/skameiki.obj"), glm::vec3{0});

  SharedPtr<Shader> shader = Shader::Create("res/shaders/basic.shader");

  Renderer3D::Init();
  Renderer3D::SetViewport(
      Viewport{0, 0, static_cast<uint32_t>(frameBufferWidth), static_cast<uint32_t>(frameBufferHeight)});

  // FIXME:
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  Event event{};
  bool running = true;

  while (running) {
    while (PollEvent(&event)) {
      ProcessEvent(&event, &running);
    }

    Renderer3D::RenderScene(g_Scene, shader);
    glfwSwapBuffers(window);
  }

  return 0;
}

// int main(int argc, const char* argv[])
// {
//     GLFWwindow* window = nullptr;
//     if (!glfwInit())
//     {
//         std::cout << "Failed to initialize GLFW" << std::endl;
//         return -1;
//     }

//     glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//     glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

// #ifdef __APPLE__
//     glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
// #endif

//     window = glfwCreateWindow(640, 480, "Hello, OpenGL!", /*monitor=*/nullptr, /*share=*/nullptr);
//     if (!window)
//     {
//         std::cout << "Failed to create window" << std::endl;
//         glfwTerminate();
//         return -1;
//     }

//     int32_t frameBufferWidth, frameBufferHeight;
//     glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);

//     glfwMakeContextCurrent(window);

//     if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
//     {
//         std::cout << "Failed to initialize GLAD" << std::endl;
//         glfwTerminate();
//         return -1;
//     }

//     g_Scene.camera = CreateShared<Camera>(glm::vec3{10, 5, 10}, glm::vec3{-1, -0.5, -1});
//     g_Scene.light = CreateShared<LightSource>(glm::vec3{10, 0, 15}, glm::vec3{1, 1, 0});
//     g_Scene.meshes.push_back(CreateShared<MeshInstance>(glm::vec3{0, 0, 0},
//     ParseMeshObj("res/meshes/skameiki.obj")));

//     SharedPtr<Shader> shader = Shader::Create("res/shaders/basic.shader");

//     Renderer3D::Init();
//     Renderer3D::SetViewport(Viewport{0, 0, static_cast<uint32_t>(frameBufferWidth),
//                                            static_cast<uint32_t>(frameBufferHeight)});

//     glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
//     glfwSetKeyCallback(window, keyCallback);
//     glfwSetCursorPosCallback(window, mouseMoveCallback);

//     while (!glfwWindowShouldClose(window))
//     {
//         Renderer3D::RenderScene(g_Scene, shader);

//         glfwSwapBuffers(window);
//         glfwPollEvents();
//     }

//     glfwTerminate();

//     return 0;
// }