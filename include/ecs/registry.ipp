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

namespace vulture {

template <typename ComponentT, typename... Args>
void Registry::AddComponent(EntityId id, Args&&... args) {
  ComponentTypeId component_type_id = ComponentHolder<ComponentT>::GetTypeId();

  assert(entities_.find(id) != entities_.end());
  assert(components_[component_type_id].find(id) == components_[component_type_id].end()); 
      
  IComponentHolder* component_holder = new ComponentHolder<ComponentT>(std::forward<Args>(args)...);

  entities_[id].emplace(component_type_id, component_holder);
  components_[component_type_id].emplace(id, component_holder);
}

template <typename ComponentT>
void Registry::RemoveComponent(EntityId id) {
  ComponentTypeId component_type_id = ComponentHolder<ComponentT>::GetTypeId();

  assert(entities_.find(id) != entities_.end());
  assert(components_[component_type_id].find(id) != components_[component_type_id].end());

  components_[component_type_id].erase(id);
  IComponentHolder* component_to_remove_holder = entities_[id].find(component_type_id)->second;
  entities_[id].erase(component_type_id);
  
  delete component_to_remove_holder;
}

template <typename ComponentT>
ComponentT* Registry::GetComponent(EntityId id) {
  ComponentTypeId component_type_id = ComponentHolder<ComponentT>::GetTypeId();

  auto component_holder_iter = components_[component_type_id].find(id);

  if (component_holder_iter == components_[component_type_id].end()) {
    return nullptr;
  }

  return reinterpret_cast<ComponentHolder<ComponentT>*>(component_holder_iter->second)->Get();
}

template <typename ComponentT>
bool Registry::HasComponent(EntityId id) {
  ComponentTypeId component_type_id = ComponentHolder<ComponentT>::GetTypeId();

  auto component_holder_iter = components_[component_type_id].find(id);

  return component_holder_iter != components_[component_type_id].end();
}

template <typename ComponentT>
EntityMap& Registry::GetEntityMap() {
  ComponentTypeId component_type_id = ComponentHolder<ComponentT>::GetTypeId();

  return components_[component_type_id];
}

}  // namespace vulture