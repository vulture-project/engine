/**
 * @author Sergey Zelenkin (https://github.com/vssense)
 * @file event.cpp
 * @date 2022-04-27
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

#include "platform/input.hpp"
#include "core/logger.hpp"

#include <GLFW/glfw3.h>

#include <cassert>

namespace vulture {

// Static fields initialization
GLFWwindow* Input::window_ = nullptr;

std::vector<bool> Mouse::button_pressed_(kMaxKey, false);
std::vector<bool> Keyboard::key_pressed_(kMaxKey + 1, false);


float Mouse::mouse_position_x = 0;
float Mouse::mouse_position_y = 0;
float Mouse::scroll_dx_ = 0;
float Mouse::scroll_dy_ = 0;

void Input::ScrollCallback(GLFWwindow* /*window*/, double dx, double dy) {
  //LOG_INFO(Logger, "scroll {} {}", dx, dy);

  Mouse::Scroll(dx, dy);
}

void Input::MouseButtonCallback(GLFWwindow* /*window*/, int button, int action, int /*mods*/) {
  //LOG_INFO(Logger, "mouse button {} {} {}", button, action, mods);

  if (action == GLFW_PRESS) {
      Mouse::GetButtons()[button] = true;
  } else if (action == GLFW_RELEASE) {
      Mouse::GetButtons()[button] = false;
  }
}

void Input::KeyCallback(GLFWwindow* /*window*/, int key, int scancode, int action, int mods) {
  //LOG_INFO(Logger, "key {} {} {} {}", key, scancode, action, mods);

  if (action == GLFW_PRESS) {
    Keyboard::GetKeys()[key] = true;
  } else if (action == GLFW_RELEASE) {
    Keyboard::GetKeys()[key] = false;
  }
}

void Input::MouseMoveCallback(GLFWwindow* /*window*/, double xpos, double ypos) {
  //LOG_INFO(Logger, "mouse move {} {}", xpos, ypos);

  Mouse::MouseMove(static_cast<float>(xpos), static_cast<float>(ypos));
}

void Input::CloseCallback(GLFWwindow* /*window*/) {
  LOG_INFO(Logger, "Triggering quit event");

  // dispatcher_->Trigger<QuitEvent>();
}

void Input::GetMousePosition(float* x, float* y) {
  assert(x);
  assert(y);

  glfwPollEvents();

  double xpos = 0.0f;
  double ypos = 0.0f;
  glfwGetCursorPos(window_, &xpos, &ypos);

  *x = xpos;
  *y = ypos;

  // Mouse::GetMousePosition(x, y);
}
bool Input::GetKeyPressed(Keys key) {
  glfwPollEvents();
  return Keyboard::Pressed(key);
}

bool Input::GetMousePressed(MouseButton button) {
  glfwPollEvents();
  return Mouse::Pressed(button);
}

void Input::SetMouseEnabled(bool enabled) {
  glfwSetInputMode(window_, GLFW_CURSOR, enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}

void Input::SetWindow(GLFWwindow* window) {
  assert(window);

  window_ = window;

  glfwSetKeyCallback(window, KeyCallback);
  glfwSetWindowCloseCallback(window, CloseCallback);
  glfwSetCursorPosCallback(window, MouseMoveCallback);
  glfwSetMouseButtonCallback(window, MouseButtonCallback);
  glfwSetScrollCallback(window, ScrollCallback);
}

}  // namespace vulture
