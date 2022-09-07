/**
 * @author Sergey Zelenkin (https://github.com/vssense)
 * @file event.hpp
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

#pragma once

#include <queue>
#include <vector>

#include "platform/window.hpp"
#include "event_system/event_system.hpp"

namespace vulture {

enum Action {
  kRelease,
  kPress,
  kHold
};

enum MouseButton {
  kLeftMouseButton,
  kRightMouseButton,
  kMiddleMouseButton,
  kX1MouseButton,
  kX2MouseButton
};

struct ButtonMods {
  ButtonMods() = default;
  explicit ButtonMods(int mods);

  bool shift_pressed : 1;
  bool ctrl_pressed  : 1;
  bool alt_pressed   : 1;
};

struct QuitEvent {};

struct ScrollEvent {
  ScrollEvent(int dx, int dy) : dx(dx), dy(dy) {}

  int dx;
  int dy;
};

struct MouseMoveEvent {         // Do we need dx and dy here?
  MouseMoveEvent(int x, int y) : x(x), y(y) {}

  int x;
  int y;
};

struct MouseButtonEvent {       // Do we need coords here?
  MouseButtonEvent() = default;
  MouseButtonEvent(int button, int action, int mods);

  MouseButton button;
  Action action;
  ButtonMods mods;
};

struct KeyEvent {
  KeyEvent() = default;
  KeyEvent(int key, int scancode, int action, int mods);

  int key;
  int scancode;
  Action action;
  ButtonMods mods;
};

enum class Keys {
  kAKey = GLFW_KEY_A,
  kBKey = GLFW_KEY_B,
  kCKey = GLFW_KEY_C,
  kDKey = GLFW_KEY_D,
  kEKey = GLFW_KEY_E,
  kFKey = GLFW_KEY_F,
  kGKey = GLFW_KEY_G,
  kHKey = GLFW_KEY_H,
  kIKey = GLFW_KEY_I,
  kJKey = GLFW_KEY_J,
  kKKey = GLFW_KEY_K,
  kLKey = GLFW_KEY_L,
  kMKey = GLFW_KEY_M,
  kNKey = GLFW_KEY_N,
  kOKey = GLFW_KEY_O,
  kPKey = GLFW_KEY_P,
  kQKey = GLFW_KEY_Q,
  kRKey = GLFW_KEY_R,
  kSKey = GLFW_KEY_S,
  kTKey = GLFW_KEY_T,
  kUKey = GLFW_KEY_U,
  kVKey = GLFW_KEY_V,
  kWKey = GLFW_KEY_W,
  kXKey = GLFW_KEY_X,
  kYKey = GLFW_KEY_Y,
  kZKey = GLFW_KEY_Z,

  k0Key = GLFW_KEY_0,
  k1Key = GLFW_KEY_1,
  k2Key = GLFW_KEY_2,
  k3Key = GLFW_KEY_3,
  k4Key = GLFW_KEY_4,
  k5Key = GLFW_KEY_5,
  k6Key = GLFW_KEY_6,
  k7Key = GLFW_KEY_7,
  k8Key = GLFW_KEY_8,
  k9Key = GLFW_KEY_9,

  kEscKey = GLFW_KEY_ESCAPE,
  kTabKey = GLFW_KEY_TAB,
  kCapsKey = GLFW_KEY_CAPS_LOCK,
  kLeftShiftKey = GLFW_KEY_LEFT_SHIFT,
  kRightShiftKey = GLFW_KEY_RIGHT_SHIFT,
  kLeftCtrlKey = GLFW_KEY_LEFT_CONTROL,
  kRightCtrlKey = GLFW_KEY_RIGHT_CONTROL,
  kLeftAltKey = GLFW_KEY_LEFT_ALT,
  kRightAltKey = GLFW_KEY_RIGHT_ALT,
  kSpaceKey = GLFW_KEY_SPACE,
  kEnterKey = GLFW_KEY_ENTER,
};

class InputEventManager {
 private:
  InputEventManager() = default;

 public:
  static void SetWindowAndDispatcher(Window* window, Dispatcher* dispatcher);

  static void TriggerEvents();

  static void GetCursorPosition(float* x, float* y);
  static void SetCursorEnabled(bool enabled);

 private:
  static void KeyCallback        (GLFWwindow* window, int key, int scancode, int action, int mods);
  static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
  static void MouseMoveCallback  (GLFWwindow* window, double xpos, double ypos);
  static void CloseCallback      (GLFWwindow* window);
  static void ScrollCallback     (GLFWwindow* window, double dx, double dy);

  static Window* window_;
  static Dispatcher* dispatcher_;
};

class Keyboard {
 private:
  Keyboard() = default;

 public:
  static bool Pressed(Keys key) {
    return keys_pressed_[static_cast<size_t>(key)];
  }

  static std::vector<bool>& GetKeys() {
    return keys_pressed_;
  }

 private:
  static const size_t kMaxKey = GLFW_KEY_LAST;
  static std::vector<bool> keys_pressed_;
};

}  // namespace vulture
