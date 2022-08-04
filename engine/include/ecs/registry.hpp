/**
 * @author Viktor Baranov (github.com/baranov-V-V)
 * @file registry.hpp
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

#include "ecs/component_holder.hpp"
#include "ecs/generator.hpp"

#include <cassert>
#include <cstdint>
#include <unordered_map>

namespace vulture {

using EntityId = uint64_t;

constexpr EntityId kInvalidEntityId = 0;

using EntityIdGenerator = IdGenerator<EntityId>;

using EntityMap = std::unordered_map<EntityId, IComponentHolder*>;
using ComponentMap = std::unordered_map<ComponentTypeId, IComponentHolder*>;

class EntityRegistry {
 public:
  EntityId CreateEntity();

  void DestroyEntity(EntityId id);

  void DestroyAllEntities();

  template <typename ComponentT, typename... Args>
  void AddComponent(EntityId id, Args&&... args);

  template <typename ComponentT>
  void RemoveComponent(EntityId id);

  template <typename ComponentT>
  ComponentT* GetComponent(EntityId id);

  template <typename ComponentT>
  bool HasComponent(EntityId id);

  template <typename ComponentT>
  EntityMap& GetEntityMap();

  ~EntityRegistry();

 private:
  using EntityStorage = std::unordered_map<EntityId, ComponentMap>;
  using ComponentStorage = std::unordered_map<ComponentTypeId, EntityMap>;

 private:
  EntityStorage entities_;
  ComponentStorage components_;
  EntityIdGenerator entity_id_generator_;
};

}  // namespace vulture

#include "ecs/registry.ipp"