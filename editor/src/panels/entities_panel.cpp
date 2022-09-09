/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file entities_panel.cpp
 * @date 2022-08-05
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

#include "panels/entities_panel.hpp"

#include "imgui.h"

using namespace vulture;

void EntitiesPanel::OnRender(Scene& scene) {
  if (ImGui::Begin("Entities")) {
    const ImGuiTreeNodeFlags base_flags =
        ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

    EntityId new_selected_entity{kInvalidEntityId};

    for (auto [entity_id, component_storage] : scene.GetEntityRegistry().GetEntities()) {
      const bool is_selected = (entity_id == selected_entity_);

      ImGuiTreeNodeFlags node_flags = base_flags;
      if (is_selected) {
        node_flags |= ImGuiTreeNodeFlags_Selected;
      }

      node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

      EntityHandle handle{entity_id, scene.GetEntityRegistry()};
      if (!handle.HasComponent<NameComponent>()) {
        ImGui::TreeNodeEx((void*)(intptr_t)entity_id, node_flags, "Entity %llu", entity_id);
      } else {
        ImGui::TreeNodeEx((void*)(intptr_t)entity_id, node_flags, "%s",
                          handle.GetComponent<NameComponent>()->name.c_str());
      }

      if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
        new_selected_entity = entity_id;
      }
    }

    if (new_selected_entity != kInvalidEntityId) {
      selected_entity_ = new_selected_entity;
    }

  }

  ImGui::End();
}

void EntitiesPanel::SetSelectedEntity(EntityId entity_id) {
  selected_entity_ = entity_id;
}

EntityId EntitiesPanel::GetSelectedEntity() const {
  return selected_entity_;
}