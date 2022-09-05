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

#include "panels/preview_panel.hpp"
#include "imgui.h"

using namespace vulture;

void PreviewPanel::OnInit() {
  FramebufferSpec framebuffer_spec{};
  framebuffer_spec.width       = 1280;
  framebuffer_spec.height      = 960;
  framebuffer_spec.attachments = {FramebufferAttachmentFormat::kRGBA8,
                                  FramebufferAttachmentFormat::kDepth24Stencil8};
  framebuffer_ = Framebuffer::Create(framebuffer_spec);
}

void PreviewPanel::OnClose() {
  delete framebuffer_;
  resized_ = false;
}

void PreviewPanel::OnRender() {
  assert(framebuffer_);

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});

  if (ImGui::Begin("Preview")) {
    ImVec2 panel_size{};

    // Framebuffer scale for retina support
    panel_size.x = ImGui::GetContentRegionAvail().x * ImGui::GetIO().DisplayFramebufferScale.x;
    panel_size.y = ImGui::GetContentRegionAvail().y * ImGui::GetIO().DisplayFramebufferScale.y;

    OnResize(static_cast<uint32_t>(panel_size.x), static_cast<uint32_t>(panel_size.y));

    uint64_t texture_id = framebuffer_->GetColorAttachmentId();
    FramebufferSpec framebuffer_spec = framebuffer_->GetFramebufferSpec();
    ImGui::Image(reinterpret_cast<void*>(texture_id), ImGui::GetContentRegionAvail(), ImVec2{0, 1}, ImVec2{1, 0});
  }

  ImGui::End();

  ImGui::PopStyleVar();
}

void PreviewPanel::OnResize(uint32_t width, uint32_t height) {
  resized_ = false;

  FramebufferSpec spec = framebuffer_->GetFramebufferSpec();
  if (width > 0 && height > 0 && (spec.width != width || spec.height != height)) {
    framebuffer_->Resize(width, height);
    resized_ = true;
  }
}

Framebuffer& PreviewPanel::GetFramebuffer() { assert(framebuffer_); return *framebuffer_; }

bool PreviewPanel::Resized() const { return resized_; }