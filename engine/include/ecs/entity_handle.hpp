/**
 * @author Viktor Baranov (github.com/baranov-V-V)
 * @file entity_handle.hpp
 * @date 2022-05-10
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

#include "ecs/registry.hpp"

namespace vulture {

class EntityHandle {
 public:
  EntityHandle(EntityId id, EntityRegistry& registry);
  EntityHandle(const EntityHandle& other) : id_(other.id_), registry_(other.registry_) {}

  EntityHandle& operator=(const EntityHandle& other);

  bool Invalid() const;

  void Destroy();

  template <typename ComponentT, typename... Args>
  void AddComponent(Args&&... args);

  template <typename ComponentTy>
  void RemoveComponent();

  template <typename ComponentT>
  ComponentT* GetComponent();

  template <typename ComponentT>
  bool HasComponent();

 private:
  EntityId id_{kInvalidEntityId};
  EntityRegistry& registry_;
};

}  // namespace vulture

#include "ecs/entity_handle.ipp"