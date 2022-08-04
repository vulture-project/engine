/**
 * @author Viktor Baranov (github.com/baranov-V-V)
 * @file registry.ipp
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

#include "ecs/registry.hpp"

#include <iostream>

namespace vulture {

EntityId EntityRegistry::CreateEntity() {
  EntityId id = entity_id_generator_.Next();
  entities_.emplace(id, ComponentMap{});
  return id;
}

void EntityRegistry::DestroyEntity(EntityId id) {
  assert(entities_.find(id) != entities_.end());

  for (auto it = entities_[id].begin(); it != entities_[id].end(); ++it) {
    ComponentTypeId component_type_id = it->first;

    // IComponentHolder* component_to_remove_holder = components_[component_type_id].find(id)->second;
    IComponentHolder* component_to_remove_holder = it->second;
    components_[component_type_id].erase(id);
    delete component_to_remove_holder;
  }

	entities_.erase(id);
}

void EntityRegistry::DestroyAllEntities() {
  for (auto iter = entities_.begin(); iter != entities_.end();) {
    EntityId id_to_delete = iter->first;
    iter++;
    DestroyEntity(id_to_delete);
  }
}

EntityRegistry::~EntityRegistry() {
  DestroyAllEntities();
}

}  // namespace vulture