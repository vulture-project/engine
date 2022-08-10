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

#include "panels/inspector_panel.hpp"

#include "imgui.h"

using namespace vulture;

const ImGuiTreeNodeFlags kComponentNodeBaseFlags =
    ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth |
    ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

void InspectorPanel::OnRender(EntityHandle entity) {
  if (ImGui::Begin("Inspector")) {
    if (entity.HasComponent<TransformComponent>()) {
      RenderTransformComponent(*entity.GetComponent<TransformComponent>());
    }

    if (entity.HasComponent<CameraComponent>()) {
      RenderCameraComponent(*entity.GetComponent<CameraComponent>());
    }

    if (entity.HasComponent<MeshComponent>()) {
      RenderMeshComponent(*entity.GetComponent<MeshComponent>());
    }

    if (entity.HasComponent<LightSourceComponent>()) {
      RenderLightSourceComponent(*entity.GetComponent<LightSourceComponent>());
    }

    ImGui::End();
  }
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
    ImGui::Checkbox("Fixed aspect ratio", &camera_component.fixed_aspect_ratio);

    ImGui::SliderFloat("FOV", &camera_component.specs.fov, 1.0f, 90.0f, "%.1f");
    ImGui::SliderFloat("Near", &camera_component.specs.near, 0.001f, 10.0f, "%.3f");
    ImGui::SliderFloat("Far", &camera_component.specs.far, 10.0f, 1000.0f, "%.1f");

    ImGui::TreePop();
  }
}

void InspectorPanel::RenderMeshComponent(MeshComponent&) {}

void InspectorPanel::RenderLightSourceComponent(LightSourceComponent& light_component) {
  if (ImGui::TreeNodeEx((void*)typeid(LightSourceComponent).hash_code(), kComponentNodeBaseFlags, "Light")) {
    const char* type_names[] = {"Directional", "Point", "Spot"};

    ImGui::Combo("Type", reinterpret_cast<int*>(&light_component.specs.type), type_names, IM_ARRAYSIZE(type_names),
                 IM_ARRAYSIZE(type_names));

    switch (light_component.specs.type) {
      case LightType::kDirectional: {
        ImGui::ColorEdit3("Ambient", reinterpret_cast<float*>(&light_component.specs.directional.color_specs.ambient));
        ImGui::ColorEdit3("Diffuse", reinterpret_cast<float*>(&light_component.specs.directional.color_specs.diffuse));
        ImGui::ColorEdit3("Specular",
                          reinterpret_cast<float*>(&light_component.specs.directional.color_specs.specular));
        break;
      }

      case LightType::kPoint: {
        ImGui::ColorEdit3("Ambient", reinterpret_cast<float*>(&light_component.specs.point.color_specs.ambient));
        ImGui::ColorEdit3("Diffuse", reinterpret_cast<float*>(&light_component.specs.point.color_specs.diffuse));
        ImGui::ColorEdit3("Specular", reinterpret_cast<float*>(&light_component.specs.point.color_specs.specular));

        ImGui::SliderFloat("Att. linear", &light_component.specs.point.attenuation_specs.linear, 0.0f, 1.0f, "%.3f");
        ImGui::SliderFloat("Att. quadratic", &light_component.specs.point.attenuation_specs.quadratic, 0.0f, 1.0f,
                           "%.3f");
        break;
      }

      case LightType::kSpot: {
        ImGui::ColorEdit3("Ambient", reinterpret_cast<float*>(&light_component.specs.spot.color_specs.ambient));
        ImGui::ColorEdit3("Diffuse", reinterpret_cast<float*>(&light_component.specs.spot.color_specs.diffuse));
        ImGui::ColorEdit3("Specular", reinterpret_cast<float*>(&light_component.specs.spot.color_specs.specular));

        ImGui::SliderFloat("Att. linear", &light_component.specs.spot.attenuation_specs.linear, 0.0f, 1.0f, "%.3f");
        ImGui::SliderFloat("Att. quadratic", &light_component.specs.spot.attenuation_specs.quadratic, 0.0f, 1.0f,
                           "%.3f");

        ImGui::SliderFloat("Inner cos", &light_component.specs.spot.inner_cone_cosine, 0.0f, 1.0f, "%.2f");
        ImGui::SliderFloat("Outer cos", &light_component.specs.spot.outer_cone_cosine, 0.0f, 1.0f, "%.2f");
        break;
      }

      default: {
        assert(!"Invalid light type!");
        break;
      }
    }

    ImGui::TreePop();
  }
}
