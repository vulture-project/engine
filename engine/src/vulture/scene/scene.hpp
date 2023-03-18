/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file scene.hpp
 * @date 2022-05-15
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

#include <fennecs/entity/world.hpp>
#include <vulture/event_system/event_system.hpp>
#include <vulture/renderer/3d/renderer3d.hpp>
#include <vulture/renderer/3d/scene3d.hpp>
#include <vulture/scene/components.hpp>

namespace vulture {

class Scene {
 public:
  fennecs::EntityWorld& GetEntityWorld();

  void OnStart(Dispatcher& dispatcher);

  void OnViewportResize(uint32_t width, uint32_t height);

  /**
   * @brief Simulate the game world.
   *
   * Simulate physics and run scripts.
   */
  void OnUpdate(float timestep);

  /**
   * @brief Render the scene.
   * 
   * @param renderer 
   * @param render_mode 
   */
  void Render(Renderer3D* renderer, Framebuffer* framebuffer, Renderer3D::DebugRenderMode render_mode = Renderer3D::DebugRenderMode::kDefault);

  /**
   * @brief Create a parentless entity.
   * 
   * @param name 
   * @return EntityHandle 
   */
  fennecs::EntityHandle CreateEntity(const std::string& name = "Untitled entity");

  /**
   * @brief Create a child entity from parent.
   * 
   * @param parent 
   * @param name 
   * @return EntityHandle 
   */
  fennecs::EntityHandle CreateChildEntity(fennecs::EntityHandle& parent, const std::string& name = "Untitled entity");

  glm::mat4 ComputeWorldSpaceMatrix(fennecs::EntityHandle entity);
  Transform ComputeWorldSpaceTransform(fennecs::EntityHandle entity);

 private:
  fennecs::EntityWorld world_;
  Scene3D scene_;
};

}  // namespace vulture