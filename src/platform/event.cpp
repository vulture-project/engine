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

#include "platform/event.hpp"
#include "core/logger.hpp"

#include <GLFW/glfw3.h>

#include <cassert>

namespace vulture {

// Static fields initialization
Window* InputEventManager::window_ = nullptr;
Dispatcher* InputEventManager::dispatcher_ = nullptr;

std::vector<bool> Keyboard::keys_pressed_(kMaxKey + 1, false);

ButtonMods::ButtonMods(int mods) {
  shift_pressed = static_cast<bool>(mods & GLFW_MOD_SHIFT);
  ctrl_pressed = static_cast<bool>(mods & GLFW_MOD_CONTROL);
  alt_pressed = static_cast<bool>(mods & GLFW_MOD_ALT);
}

MouseButtonEvent::MouseButtonEvent(int button, int action, int mods)
    : button(static_cast<MouseButton>(button)), action(static_cast<Action>(action)), mods(mods) {}

KeyEvent::KeyEvent(int key, int scancode, int action, int mods)
    : key(key), scancode(scancode), action(static_cast<Action>(action)), mods(mods) {}

void InputEventManager::ScrollCallback(GLFWwindow* /*window*/, double dx, double dy) {
  LOG_INFO(Logger, "Triggering scroll event {} {}", dx, dy);

  dispatcher_->Trigger<ScrollEvent>(static_cast<int>(dx), static_cast<int>(dy));
}

void InputEventManager::MouseButtonCallback(GLFWwindow* /*window*/, int button, int action, int mods) {
  LOG_INFO(Logger, "Triggering mouse button event {} {} {}", button, action, mods);

  dispatcher_->Trigger<MouseButtonEvent>(button, action, mods);
}

void InputEventManager::KeyCallback(GLFWwindow* /*window*/, int key, int scancode, int action, int mods) {
  LOG_INFO(Logger, "Triggering key event {} {} {} {}", key, scancode, action, mods);

  if (action == GLFW_PRESS) {
    Keyboard::GetKeys()[key] = true;
  } else if (action == GLFW_RELEASE) {
    Keyboard::GetKeys()[key] = false;
  }

  dispatcher_->Trigger<KeyEvent>(key, scancode, action, mods);
}

void InputEventManager::MouseMoveCallback(GLFWwindow* /*window*/, double xpos, double ypos) {
  //LOG_INFO(Logger, "Triggering mouse move event {} {}", xpos, ypos);

  dispatcher_->Trigger<MouseMoveEvent>(static_cast<int>(xpos), static_cast<int>(ypos));
}

void InputEventManager::CloseCallback(GLFWwindow* /*window*/) {
  LOG_INFO(Logger, "Triggering quit event");

  dispatcher_->Trigger<QuitEvent>();
}

void InputEventManager::TriggerEvents() {
  assert(window_);
  assert(dispatcher_);

  glfwPollEvents();
}

void InputEventManager::SetWindowAndDispatcher(Window* window, Dispatcher* dispatcher) {
  assert(window);
  assert(dispatcher);

  dispatcher_ = dispatcher;

  window_ = window;
  GLFWwindow* native_window = window_->GetNativeWindow();

  glfwSetKeyCallback(native_window, KeyCallback);
  glfwSetWindowCloseCallback(native_window, CloseCallback);
  glfwSetCursorPosCallback(native_window, MouseMoveCallback);
  glfwSetMouseButtonCallback(native_window, MouseButtonCallback);
  glfwSetScrollCallback(native_window, ScrollCallback);
}

}  // namespace vulture
