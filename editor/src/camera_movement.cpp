/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file camera_movement.cpp
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

#include "camera_movement.hpp"

#include "scene/components.hpp"

using namespace vulture;

void CameraMovementScript::OnAttach(EntityHandle entity, Dispatcher& dispatcher) {
  entity_ = new EntityHandle(entity);
  dispatcher.GetSink<MouseMoveEvent>().Connect<&CameraMovementScript::OnMouseMove>(*this);
  dispatcher.GetSink<MouseButtonEvent>().Connect<&CameraMovementScript::OnMouseButton>(*this);
}

void CameraMovementScript::OnUpdate(float timestep) {
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

  transform->translation.y = std::max(2.5f, transform->translation.y);
}

void CameraMovementScript::OnMouseMove(const MouseMoveEvent& event) {
  if (!rotation_mode_) {
    return;
  }

  float dx = mouse_prev_x_ - event.x;
  float dy = mouse_prev_y_ - event.y;

  Transform* transform = &entity_->GetComponent<TransformComponent>()->transform;
  rotation_y_ += 0.001f * dx;
  rotation_z_ += 0.001f * dy;
  transform->rotation = glm::quat(glm::vec3(rotation_z_, rotation_y_, 0));

  mouse_prev_x_ = event.x;
  mouse_prev_y_ = event.y;
}

void CameraMovementScript::OnMouseButton(const MouseButtonEvent& event) {
  if (event.button == kRightMouseButton) {
    if (event.action == kPress) {
      rotation_mode_ = true;
      InputEventManager::GetCursorPosition(&mouse_prev_x_, &mouse_prev_y_);
      InputEventManager::SetCursorEnabled(false);
    } else if (event.action == kRelease) {
      rotation_mode_ = false;
      InputEventManager::SetCursorEnabled(true);
    }
  }
}