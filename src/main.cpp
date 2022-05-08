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
#include "renderer/3d/renderer3d.hpp"
#include "resource_loaders/parse_obj.hpp"

#include "core/logger.hpp"

using namespace vulture;
using namespace input;

Scene3D g_Scene;

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

  g_Scene.GetMainCamera()->transform.rotation.y += 0.001f * dx;
  g_Scene.GetMainCamera()->transform.rotation.x += 0.001f * dy;

  // g_Scene.camera->forward =
  //     glm::normalize(glm::rotate(glm::identity<glm::mat4>(), 0.001f * dx, glm::vec3{0, 1, 0}) *
  //     glm::vec4(g_Scene.camera->forward, 1));

  // g_Scene.camera->forward.y += 0.001f * dy;
  // g_Scene.camera->forward = glm::normalize(g_Scene.camera->forward);

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

  glm::vec3 forward = g_Scene.GetMainCamera()->CalculateForwardVector();
  glm::vec3 right = g_Scene.GetMainCamera()->CalculateRightVector();

  if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    // transformComponent->translation += speed * cameraComponent->forward;
    // g_Scene.camera->pos += speed * g_Scene.camera->forward;
    g_Scene.GetMainCamera()->transform.translation += speed * forward;
  }

  if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    // transformComponent->translation -= speed * cameraComponent->forward;
    // g_Scene.camera->pos -= speed * g_Scene.camera->forward;
    g_Scene.GetMainCamera()->transform.translation -= speed * forward;
  }

  if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    // transformComponent->translation -= speed * glm::cross(cameraComponent->forward, glm::vec3{0, 1, 0});
    // g_Scene.camera->pos -= speed * glm::cross(g_Scene.camera->forward, glm::vec3{0, 1, 0});
    g_Scene.GetMainCamera()->transform.translation -= speed * right;
  }

  if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    // transformComponent->translation += speed * glm::cross(cameraComponent->forward, glm::vec3{0, 1, 0});
    // g_Scene.camera->pos += speed * glm::cross(g_Scene.camera->forward, glm::vec3{0, 1, 0});
    g_Scene.GetMainCamera()->transform.translation += speed * right;
  }

  if (key == GLFW_KEY_Q && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    // transformComponent->translation.y += 1;
    // g_Scene.camera->pos.y += 1;
    g_Scene.GetMainCamera()->transform.translation.y -= 1;
  }

  if (key == GLFW_KEY_E && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    // transformComponent->translation.y -= 1;
    // g_Scene.camera->pos.y -= 1;
    g_Scene.GetMainCamera()->transform.translation.y += 1;
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
  Logger::OpenLogFile();
  LOG_INFO(main, "Hi from main {}", 24);

  Window _window{1280, 960};
  EventQueue::SetWindow(&_window);
  NativeWindow* window = _window.GetNativeWindow();

  int32_t frameBufferWidth = 0;
  int32_t frameBufferHeight = 0;
  glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  CameraNode3D* cameraNode = new CameraNode3D(
      PerspectiveCameraSpecs((float)frameBufferWidth / (float)frameBufferHeight), Transform(glm::vec3{10, 2, 10}));

  g_Scene.AddCamera(cameraNode);
  g_Scene.SetMainCamera(cameraNode);

  g_Scene.AddLightSource(new LightSourceNode3D(LightSourceSpecs(LightSourceSpecs::Type::kPoint, glm::vec3{0.1},
                                                                glm::vec3{0.8, 0.7, 0}, glm::vec3{0.2}, 0.35, 0.65),
                                               Transform(glm::vec3{4, 3, 0})));

  g_Scene.AddLightSource(new LightSourceNode3D(LightSourceSpecs(LightSourceSpecs::Type::kPoint, glm::vec3{0.1},
                                                                glm::vec3{0.8, 0.4, 0.4}, glm::vec3{0.2}, 0.35, 0.65),
                                               Transform(glm::vec3{-4, 3, 0})));

  g_Scene.AddLightSource(new LightSourceNode3D(
      LightSourceSpecs(LightSourceSpecs::Type::kPoint, glm::vec3{0}, glm::vec3{0.8, 0.6, 0.8}, glm::vec3{0}, 0.7, 1.8),
      Transform(glm::vec3{10, 1, 10})));

  g_Scene.AddModel(new ModelNode3D(ParseMeshObj("res/meshes/skameiki.obj")));
  g_Scene.AddModel(new ModelNode3D(ParseMeshObj("res/meshes/wooden watch tower.obj"), Transform(glm::vec3{0, -0.75, 0})));

  g_Scene.AddModel(new ModelNode3D(ParseMeshObj("res/meshes/street_lamp.obj"),
                                   Transform(glm::vec3{3, 0, 0}, glm::vec3{0}, glm::vec3{0.2})));

  g_Scene.AddModel(new ModelNode3D(ParseMeshObj("res/meshes/street_lamp.obj"),
                                   Transform(glm::vec3{-3, 0, 0}, glm::vec3{0}, glm::vec3{0.2})));

  // SharedPtr<Shader> shader = Shader::Create("res/shaders/basic.shader");

  Renderer3D::Init();
  Renderer3D::SetViewport(
      Viewport{0, 0, static_cast<uint32_t>(frameBufferWidth), static_cast<uint32_t>(frameBufferHeight)});

  // FIXME:
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPos(window, frameBufferWidth / 2, frameBufferHeight / 2);

  Event event{};
  bool running = true;

  while (running) {
    while (PollEvent(&event)) {
      ProcessEvent(&event, &running);
    }

    Renderer3D::RenderScene(&g_Scene);
    glfwSwapBuffers(window);
  }

  Logger::Close();

  return 0;
}