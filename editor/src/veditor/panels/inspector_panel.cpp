/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file inspector_panel.cpp
 * @date 2022-08-10
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

#include <imgui.h>

#include <veditor/panels/inspector_panel.hpp>

using namespace vulture;

const ImGuiTreeNodeFlags kComponentNodeBaseFlags =
    ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth |
    ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

void InspectorPanel::OnRender(fennecs::EntityHandle entity) {
  if (ImGui::Begin("Inspector")) {

    if (!entity.IsNull()) {
      if (entity.Has<TransformComponent>()) {
        RenderTransformComponent(entity.Get<TransformComponent>());
      }

      if (entity.Has<CameraComponent>()) {
        RenderCameraComponent(entity.Get<CameraComponent>());
      }

      if (entity.Has<MeshComponent>()) {
        RenderMeshComponent(entity.Get<MeshComponent>());
      }

      if (entity.Has<DirectionalLightSpecification>()) {
        RenderDirectionalLightSpecification(entity.Get<DirectionalLightSpecification>());
      }

      if (entity.Has<PointLightSpecification>()) {
        RenderPointLightSpecification(entity.Get<PointLightSpecification>());
      }

      if (entity.Has<SpotLightSpecification>()) {
        RenderSpotLightSpecification(entity.Get<SpotLightSpecification>());
      }
    }
  }

  ImGui::End();
}

void InspectorPanel::RenderTransformComponent(TransformComponent& transform_component) {
  if (ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), kComponentNodeBaseFlags, "Transform")) {
    ImGui::DragFloat3("Translation", reinterpret_cast<float*>(&transform_component.transform.translation));
    ImGui::DragFloat3("Scale", reinterpret_cast<float*>(&transform_component.transform.scale));

    ImGui::TreePop();
  }
}

void InspectorPanel::RenderCameraComponent(CameraComponent& camera_component) {
  if (ImGui::TreeNodeEx((void*)typeid(CameraComponent).hash_code(), kComponentNodeBaseFlags, "Camera")) {
    ImGui::Checkbox("Main", &camera_component.is_main);
    ImGui::Checkbox("Fixed aspect ratio", &camera_component.fixed_aspect);

    ImGui::SliderFloat("FOV", &camera_component.specs.fov, 1.0f, 90.0f, "%.1f");
    ImGui::SliderFloat("Near", &camera_component.specs.near, 0.001f, 10.0f, "%.3f");
    ImGui::SliderFloat("Far", &camera_component.specs.far, 10.0f, 1000.0f, "%.1f");

    ImGui::TreePop();
  }
}

void InspectorPanel::RenderMeshComponent(MeshComponent&) {}

void InspectorPanel::RenderDirectionalLightSpecification(DirectionalLightSpecification& dir_light_specification) {
  if (ImGui::TreeNodeEx((void*)typeid(DirectionalLightSpecification).hash_code(), kComponentNodeBaseFlags, "Directional Light")) {
    ImGui::ColorEdit3("Color", reinterpret_cast<float*>(&dir_light_specification.color));
    ImGui::DragFloat("Intensity", &dir_light_specification.intensity, 0.01f, 0.0f, 100.0f, "%.2f");

    ImGui::TreePop();
  }
}

void InspectorPanel::RenderPointLightSpecification(PointLightSpecification& point_light_specification) {
  if (ImGui::TreeNodeEx((void*)typeid(PointLightSpecification).hash_code(), kComponentNodeBaseFlags, "Point Light")) {
    ImGui::ColorEdit3("Color", reinterpret_cast<float*>(&point_light_specification.color));
    ImGui::DragFloat("Intensity", &point_light_specification.intensity, 0.01f, 0.0f, 100.0f, "%.2f");
    ImGui::DragFloat("Range", &point_light_specification.range, 0.01f, 0.0f, 100.0f, "%.2f");

    ImGui::TreePop();
  }
}

void InspectorPanel::RenderSpotLightSpecification(SpotLightSpecification& spot_light_specification) {
  if (ImGui::TreeNodeEx((void*)typeid(SpotLightSpecification).hash_code(), kComponentNodeBaseFlags, "Spot Light")) {
    ImGui::ColorEdit3("Color", reinterpret_cast<float*>(&spot_light_specification.color));
    ImGui::DragFloat("Intensity", &spot_light_specification.intensity, 0.01f, 0.0f, 100.0f, "%.2f");
    ImGui::DragFloat("Range", &spot_light_specification.range, 0.01f, 0.0f, 100.0f, "%.2f");
    ImGui::DragFloat("Inner cone", &spot_light_specification.inner_cone,0.05f, 0.0f, M_PI, "%.2f");
    ImGui::DragFloat("Outer cone", &spot_light_specification.outer_cone,0.05f, 0.0f, M_PI, "%.2f");

    ImGui::TreePop();
  }
}
