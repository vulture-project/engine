/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file renderer_panel.cpp
 * @date 2023-04-18
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

#include <veditor/panels/renderer_panel.hpp>
#include "renderer_panel.hpp"

using namespace vulture;

constexpr ImGuiTreeNodeFlags kComponentNodeBaseFlags =
    ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth |
    ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

RendererPanel::RendererPanel(ImGuiImplementation& imgui_implementation) : imgui_implementation_(imgui_implementation) {}

RendererPanel::~RendererPanel() {
  for (uint32_t frame = 0; frame < kFramesInFlight; ++frame) {
    for (uint32_t cascade = 0; cascade < kCascadedShadowMapCascadesCount; ++cascade) {
      if (cached_cascade_textures_[frame][cascade].imgui_texture != nullptr) {
        imgui_implementation_.RemoveTextureUI(cached_cascade_textures_[frame][cascade].imgui_texture);
      }
    }
  }
}

void RendererPanel::OnRender(Renderer& renderer, uint32_t frame_index) {
  if (ImGui::Begin("Renderer")) {
    for (const auto& feature : renderer.GetFeatures()) {
      if (feature->Name() == "Cascaded Shadow Mapping") {
        RenderCSMFeature(dynamic_cast<CascadedShadowMapRenderFeature&>(*feature), frame_index);
      }
    }
  }

  ImGui::End();
}

void RendererPanel::RenderCSMFeature(CascadedShadowMapRenderFeature& feature, uint32_t frame_index) {
  void* code = (void*)typeid(CascadedShadowMapRenderFeature).hash_code();
  if (ImGui::TreeNodeEx(code, kComponentNodeBaseFlags, "Cascaded Shadow Mapping")) {
    ImGui::SeparatorText("Properties");
    ImGui::DragFloat("Log split", &feature.GetLogSplitContribution(), 0.01f, 0.0f, 1.0f, "%.001f");
    ImGui::DragFloat("Near offset", &feature.GetCascadeNearOffset(), 1.0f, -500.0f, 500.0f, "%.0f");
    ImGui::DragFloat("Far offset", &feature.GetCascadeFarOffset(), 1.0f, -500.0f, 500.0f, "%.0f");
    ImGui::ColorEdit3("Shadow color", reinterpret_cast<float*>(&feature.GetShadowColor()));
    ImGui::Checkbox("Soft shadows", &feature.GetUseSoftShadows());
    ImGui::DragFloat("Bias", &feature.GetBias(), 0.0001f, -1.0f, 1.0f);

    enum Resolution {
      kResolution128,
      kResolution256,
      kResolution512,
      kResolution1024,
      kResolution2048,
      kResolution4096,
      kResolution8192,

      kResolutionsCount
    };
    static int resolution = kResolution4096;
    const char* kResolutionNames[kResolutionsCount] = {"128x128",   "256x256",   "512x512",  "1024x1024",
                                                       "2048x2048", "4096x4096", "8192x8192"};
    ImGui::SliderInt("Resolution", &resolution, 0, kResolutionsCount - 1, kResolutionNames[resolution]);

    uint32_t actual_resolution = 0;
    switch (resolution) {
      case kResolution128:  { actual_resolution = 128;  break; }
      case kResolution256:  { actual_resolution = 256;  break; }
      case kResolution512:  { actual_resolution = 512;  break; }
      case kResolution1024: { actual_resolution = 1024; break; }
      case kResolution2048: { actual_resolution = 2048; break; }
      case kResolution4096: { actual_resolution = 4096; break; }
      case kResolution8192: { actual_resolution = 8192; break; }
      default: { VULTURE_ASSERT(false, "Invalid shadow map resolution!"); }
    }

    feature.GetResolution() = actual_resolution;

    ImGui::SeparatorText("Cascades Debug");
    for (uint32_t cascade = 0; cascade < kCascadedShadowMapCascadesCount; ++cascade) {
      ImGui::Text("Cascade %u", cascade);

      CachedTexture& cached = cached_cascade_textures_[frame_index][cascade];

      if (cached.imgui_texture == nullptr) {
        cached.texture       = feature.GetShadowMap().GetHandle();
        cached.imgui_texture = imgui_implementation_.AddTextureUI(cached.texture, cascade);
      } else if (cached.texture != feature.GetShadowMap().GetHandle()) {
        imgui_implementation_.RemoveTextureUI(cached.imgui_texture);
        
        cached.texture       = feature.GetShadowMap().GetHandle();
        cached.imgui_texture = imgui_implementation_.AddTextureUI(cached.texture, cascade);
      }

      ImGui::Image(cached.imgui_texture, ImVec2{128, 128});
    }

    ImGui::TreePop();
  }
}
