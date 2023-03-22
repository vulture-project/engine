/**
 * @author Sergey Zelenkin (https://github.com/vssense)
 * @file window.cpp
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

#include <GLFW/glfw3.h>

#include <cassert>
#include <string>
#include <vulture/platform/window.hpp>

namespace vulture {

const char* Window::kDefaultTitle = "Success is inevitable";

Window::Window(const char* title) {
  if (!glfwInit()) {
    assert(!"Can't init glfw while creating a window");
  }

//   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

// #ifdef __APPLE__
//   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 1);
// #endif

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);  // TODO: (tralf-strues) 
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  window_ = glfwCreateWindow(kDefaultWidth, kDefaultHeight, title, nullptr, nullptr);
  assert(window_ && "Can't create a window");

  glfwMakeContextCurrent(window_);
}

Window::Window(size_t width, size_t height, const char* title) {
  if (!glfwInit()) {
    assert(!"Can't init glfw while creating a window");
  }

//   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

// #ifdef __APPLE__
//   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 1);
// #endif

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);  // TODO: (tralf-strues) 
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  window_ = glfwCreateWindow(width, height, title, nullptr, nullptr);
  assert(window_ && "Can't create a window");

  glfwMakeContextCurrent(window_);
}

void Window::SetTitle(const char* title) {
  assert(window_);
  assert(title);

  glfwSetWindowTitle(window_, title);
}
void Window::SetFPSToTitle(double fps) {
  assert(window_);

  std::string title = std::to_string(fps);
  glfwSetWindowTitle(window_, title.c_str());
}

NativeWindow* Window::GetNativeWindow() { return window_; }

Window::~Window() {
  assert(window_);
  glfwDestroyWindow(window_);

  glfwTerminate();
}

uint32_t Window::GetWidth() const {
  int32_t width = 0;
  glfwGetWindowSize(window_, &width, nullptr);
  assert(width >= 0);

  return static_cast<uint32_t>(width);
}

uint32_t Window::GetHeight() const {
  int32_t height = 0;
  glfwGetWindowSize(window_, nullptr, &height);
  assert(height >= 0);

  return static_cast<uint32_t>(height);
}

uint32_t Window::GetFramebufferWidth() const {
  int32_t width = 0;
  glfwGetFramebufferSize(window_, &width, nullptr);
  assert(width >= 0);

  return static_cast<uint32_t>(width);
}

uint32_t Window::GetFramebufferHeight() const {
  int32_t height = 0;
  glfwGetFramebufferSize(window_, nullptr, &height);
  assert(height >= 0);

  return static_cast<uint32_t>(height);
}

}  // namespace vulture