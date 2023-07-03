/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file preview_panel.cpp
 * @date 2022-08-04
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
#include <glm/gtc/type_ptr.hpp>

#include <veditor/panels/preview_panel.hpp>

using namespace vulture;

PreviewPanel::PreviewPanel(RenderDevice& device, ImGuiImplementation& imgui_implementation,
                           Dispatcher& event_dispatcher)
    : device_(device), imgui_implementation_(imgui_implementation) {
  event_dispatcher.GetSink<KeyEvent>().Connect<&PreviewPanel::OnKeyPressed>(*this);
}

void PreviewPanel::OnInit() {
  TextureSpecification specification{};
  specification.format = DataFormat::kR8G8B8A8_UNORM;
  specification.usage  = kTextureUsageBitColorAttachment | kTextureUsageBitSampled;
  specification.width  = 1600;
  specification.height = 900;
  color_output_ = CreateShared<Texture>(device_, specification);

  imgui_color_output_ = imgui_implementation_.AddTextureUI(color_output_->GetHandle());
}

void PreviewPanel::OnClose() {
  resized_ = false;

  imgui_implementation_.RemoveTextureUI(imgui_color_output_);
  imgui_color_output_ = nullptr;
}

void PreviewPanel::OnRender(Scene& scene, fennecs::EntityHandle selected_entity) {
  assert(color_output_);

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});

  if (ImGui::Begin("Preview")) {
    ImVec2 panel_size{};

    // Framebuffer scale for retina support
    panel_size.x = ImGui::GetContentRegionAvail().x * ImGui::GetIO().DisplayFramebufferScale.x;
    panel_size.y = ImGui::GetContentRegionAvail().y * ImGui::GetIO().DisplayFramebufferScale.y;

    UpdateIsResized(panel_size.x, panel_size.y);

    if (!resized_) {
      ImGui::Image(imgui_color_output_, ImGui::GetContentRegionAvail());

      // Gizmos
      ImGuizmo::AllowAxisFlip(false);

      if (!selected_entity.IsNull() && selected_entity.Has<TransformComponent>()) {
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();

        ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(),
                          ImGui::GetWindowHeight());

        fennecs::EntityHandle main_camera = scene.GetMainCamera();
        if (!main_camera.IsNull()) {
          // Camera Info
          glm::mat4 view = main_camera.Get<CameraComponent>().camera.ViewMatrix();
          glm::mat4 proj = main_camera.Get<CameraComponent>().camera.ProjMatrix();

          // Entity Transform
          TransformComponent& transform_component = selected_entity.Get<TransformComponent>();
          glm::mat4 transform = transform_component.CalculateMatrix();

          ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(proj), gizmo_operation_, ImGuizmo::MODE::LOCAL,
                               glm::value_ptr(transform));

          if (ImGuizmo::IsUsing()) {
            glm::vec3 new_translation{0.0f};
            glm::vec3 new_rotation_deg{0.0f};
            glm::vec3 new_scale{0.0f};

            ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transform), glm::value_ptr(new_translation),
                                                  glm::value_ptr(new_rotation_deg), glm::value_ptr(new_scale));

            transform_component.transform.position = new_translation;
            transform_component.transform.scale    = new_scale;
            transform_component.transform.rotation = glm::quat(new_rotation_deg * ((float)M_PI / 180.0f));
          }
        }
      }
    }
  }

  ImGui::End();

  ImGui::PopStyleVar();
}

void PreviewPanel::UpdateIsResized(uint32_t width, uint32_t height) {
  resized_ = false;
  
  TextureSpecification specification = color_output_->GetSpecification();
  if (width > 0 && height > 0 && (specification.width != width || specification.height != height)) {
    resized_ = true;
    resized_width_ = width;
    resized_height_ = height;
  }
}

void PreviewPanel::OnResize() {
  if (resized_) {
    // FIXME: (tralf-strues) otherwise the texture is destroyed, but is still being used by a command buffer :(
    device_.WaitIdle();

    TextureSpecification specification = color_output_->GetSpecification();
    specification.width  = resized_width_;
    specification.height = resized_height_;
    color_output_->Recreate(specification);

    imgui_implementation_.RemoveTextureUI(imgui_color_output_);
    imgui_color_output_ = imgui_implementation_.AddTextureUI(color_output_->GetHandle());

    resized_ = false;
  }
}

SharedPtr<Texture> PreviewPanel::GetTexture() { assert(color_output_); return color_output_; }

bool PreviewPanel::Resized() const { return resized_; }

void PreviewPanel::OnKeyPressed(const KeyEvent& event) {
  if ((!event.mods.alt_pressed) || (event.action != Action::kRelease)) {
    return;
  }

  switch (static_cast<Keys>(event.key)) {
    case Keys::kTKey: { gizmo_operation_ = ImGuizmo::TRANSLATE; break; }
    case Keys::kRKey: { gizmo_operation_ = ImGuizmo::ROTATE; break; }
    case Keys::kSKey: { gizmo_operation_ = ImGuizmo::SCALE; break; }
    case Keys::kUKey: { gizmo_operation_ = ImGuizmo::UNIVERSAL; break; }
    
    default: { break; }
  }
}