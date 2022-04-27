/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file scene.hpp
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

#include <glm/gtx/string_cast.hpp>  // FIXME: Get rid of
#include <list>

#include "renderer/buffer.hpp"
#include "renderer/core.hpp"

struct Mesh {
  // TODO:
  // Texture texture;
  // Material material;
  SharedPtr<VertexArray> vertex_array;

  Mesh(const SharedPtr<VertexArray>& vertex_array) : vertex_array(vertex_array) {}
};

struct SceneNodeProperties {
  float scale{1.0f};
  glm::vec3 position{0.0f};
  glm::mat4 model_transform{glm::identity<glm::mat4>()}; // TODO: make relative to parent

  SceneNodeProperties() = default;

  SceneNodeProperties(float scale, const glm::vec3& position) : scale(scale), position(position) {
    RecalculateModelTransform();
  }

  void RecalculateModelTransform() {
    model_transform =
        glm::translate(glm::identity<glm::mat4>(), position) * glm::scale(glm::identity<glm::mat4>(), glm::vec3{scale});
  }
};

struct MeshInstance {
  SceneNodeProperties props;
  SharedPtr<Mesh> mesh;

  MeshInstance() = default;
  MeshInstance(const glm::vec3& position, const SharedPtr<Mesh>& mesh) : props(1, position), mesh(mesh) {}
};

struct LightSource {
  SceneNodeProperties props;
  glm::vec3 color;

  LightSource() = default;
  LightSource(const glm::vec3& position, const glm::vec3& color) : props(1, position), color(color) {}
};

struct Camera {
  SceneNodeProperties props;
  glm::vec3 forward;
  float rotation{0};

  Camera() = default;
  Camera(const glm::vec3& position, const glm::vec3& forward) : props(1, position), forward(forward) {}

  glm::mat4 GetProjectionView(float width, float height) const;
};

struct Scene {
  std::list<SharedPtr<MeshInstance>> meshes;
  SharedPtr<LightSource> light{nullptr}; // TODO: LightSource will be virtual
  SharedPtr<Camera> camera{nullptr}; // TODO: Camera will be virtual
};