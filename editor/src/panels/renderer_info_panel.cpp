/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file renderer_info_panel.cpp
 * @date 2022-09-05
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

#include "panels/renderer_info_panel.hpp"
#include "imgui.h"

using namespace vulture;

void RendererInfoPanel::OnRender() {
  Renderer3D::Info info = Renderer3D::GetInfo();

  if (ImGui::Begin("Renderer Info")) {
    const char* modes[] = {"Default", "Normals", "Normal maps", "Tangents", "Depth"};
    ImGui::Combo("Render mode", reinterpret_cast<int*>(&render_mode_), modes, IM_ARRAYSIZE(modes));

    ImGui::Text("Viewport: %ux%u", info.viewport.width, info.viewport.height);
    ImGui::Text("Frame time: %f ms", info.frame_time_ms);
    ImGui::Text("FPS: %f", info.fps);
    ImGui::Text("Draw calls: %u", info.draw_calls);
    ImGui::Text("Meshes: %u", info.meshes);
    ImGui::Text("Cameras: %u", info.cameras);
    ImGui::Text("Light sources: %u", info.light_sources);
  }

  ImGui::End();
}

void RendererInfoPanel::SetRenderMode(Renderer3D::DebugRenderMode render_mode) {
  render_mode_ = render_mode;
}

Renderer3D::DebugRenderMode RendererInfoPanel::GetRenderMode() const {
  return render_mode_;
}