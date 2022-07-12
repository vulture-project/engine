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

#include "core/core.hpp"
#include "renderer/3d/scene3d.hpp"
#include "renderer/3d/transform.hpp"
#include "scene/script.hpp"

namespace vulture {

struct HierarchyComponent {
  std::optional<EntityHandle> parent{std::nullopt};
  std::vector<EntityHandle> children;

  HierarchyComponent() = default;
  HierarchyComponent(EntityHandle parent) : parent(parent) {}
};

struct TransformComponent {
  Transform transform;

  TransformComponent() = default;
  TransformComponent(const Transform& transform) : transform(transform) {}

  TransformComponent(const glm::vec3& translation) : transform(translation) {}

  glm::mat4 CalculateMatrix() const { return transform.CalculateMatrix(); }
};

struct CameraComponent {
  PerspectiveCameraSpecs specs;
  bool is_main{false};

  CameraNode3D* runtime_node{nullptr};

  CameraComponent() = default;
  CameraComponent(const PerspectiveCameraSpecs& specs, bool is_main = false) : specs(specs), is_main(is_main) {}
};

struct MeshComponent {
  SharedPtr<Mesh> mesh{nullptr};

  MeshNode3D* runtime_node{nullptr};

  MeshComponent() = default;
  MeshComponent(SharedPtr<Mesh> mesh) : mesh(mesh) {}
};

struct ScriptComponent {
  IScript* script;

  ScriptComponent(IScript* script = nullptr) : script(script) {}

  ~ScriptComponent() { delete script; }
};

struct LightSourceComponent {
  LightSourceSpecs specs;

  LightSourceNode3D* runtime_node{nullptr};

  LightSourceComponent() = default;
  LightSourceComponent(const DirectionalLightSpecs& specs) : specs(specs) {}
  LightSourceComponent(const PointLightSpecs& specs) : specs(specs) {}
  LightSourceComponent(const SpotLightSpecs& specs) : specs(specs) {}
};

}  // namespace vulture