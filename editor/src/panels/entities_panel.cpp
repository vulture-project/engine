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

    fennecs::EntityHandle selected_entity{fennecs::EntityHandle::Null()};

    size_t index{};

    fennecs::EntityStream stream = scene.GetEntityWorld().Query<NameComponent>();
    for (fennecs::EntityHandle entity = stream.Next(); !entity.IsNull(); entity = stream.Next()) {
      ImGuiTreeNodeFlags node_flags = base_flags;
      if (entity.IsEqual(selected_entity_)) {
        node_flags |= ImGuiTreeNodeFlags_Selected;
      }

      node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

      ImGui::TreeNodeEx((void*)(intptr_t)index++, node_flags, "%s", entity.Get<NameComponent>().name.c_str());

      if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
        selected_entity = entity; 
      }
    }

    if (!selected_entity.IsNull()) {
      selected_entity_ = selected_entity;
    }
  }

  ImGui::End();
}

void EntitiesPanel::SetSelectedEntity(fennecs::EntityHandle entity) {
  selected_entity_ = entity;
}

fennecs::EntityHandle EntitiesPanel::GetSelectedEntity() const {
  return selected_entity_;
}