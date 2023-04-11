/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file components.hpp
 * @date 2022-05-16
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

#include <optional>
#include <vulture/core/core.hpp>
#include <vulture/renderer/camera.hpp>
#include <vulture/renderer/geometry/mesh.hpp>
#include <vulture/renderer/transform.hpp>
#include <vulture/scene/script.hpp>

namespace vulture {

struct NameComponent {
  std::string name{"Unnamed"};

  NameComponent() = default;
  NameComponent(const std::string& name) : name(name) {}
};

struct HierarchyComponent {
  std::optional<fennecs::EntityHandle> parent{std::nullopt};
  std::vector<fennecs::EntityHandle> children;

  HierarchyComponent() = default;
  HierarchyComponent(fennecs::EntityHandle parent) : parent(parent) {}
};

struct TransformComponent {
  Transform transform;

  TransformComponent() = default;
  TransformComponent(const Transform& transform) : transform(transform) {}

  TransformComponent(const glm::vec3& translation) : transform(translation) {}

  glm::mat4 CalculateMatrix() const { return transform.CalculateMatrix(); }
};

struct CameraComponent {
  Camera camera;
  bool is_main{false};
  bool fixed_aspect{false};

  CameraComponent() = default;
  CameraComponent(const PerspectiveCameraSpecification& specs, bool is_main = false) : camera(specs), is_main(is_main) {
    camera.orthographic_specification.aspect = specs.aspect;
  }
};

struct MeshComponent {
  SharedPtr<Mesh> mesh{nullptr};

  MeshComponent() = default;
  MeshComponent(SharedPtr<Mesh> mesh) : mesh(mesh) {}
};

struct ScriptComponent {
  IScript* script;

  ScriptComponent(IScript* script = nullptr) : script(script) {}

  ~ScriptComponent() { delete script; }
};

}  // namespace vulture