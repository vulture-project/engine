/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file camera_movement.hpp
 * @date 2022-09-05
 *
 * The MIT License (MIT)
 * Copyright (c) 2022 Nikita Mochalov
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

#include <vulture/event_system/event_system.hpp>
#include <vulture/platform/event.hpp>
#include <vulture/scene/script.hpp>

namespace vulture {

class CameraMovementScript : public IScript {
 public:
  constexpr static float kSpeed = 15;

  virtual void OnAttach(fennecs::EntityHandle entity, Dispatcher& dispatcher) override;
  virtual void OnUpdate(float timestep) override;

 private:
  void OnMouseMove(const MouseMoveEvent& event);
  void OnMouseButton(const MouseButtonEvent& event);

 private:
  fennecs::EntityHandle entity_{fennecs::EntityHandle::Null()};  // FIXME: Fix you =)
  float rotation_y_{0};            // Yaw
  float rotation_z_{0};            // Pitch

  bool rotation_mode_{false};

  float mouse_prev_x_{0};
  float mouse_prev_y_{0};
};

}  // namespace vulture