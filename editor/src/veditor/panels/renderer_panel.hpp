/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file renderer_panel.hpp
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

#pragma once

#include <vulture/renderer/features/cascaded_shadow_mapping.hpp>
#include <vulture/renderer/renderer.hpp>
#include <vulture/ui/imgui_implementation.hpp>

namespace vulture {

class RendererPanel {
 public:
  RendererPanel(ImGuiImplementation& imgui_implementation);
  ~RendererPanel();

  void OnRender(Renderer& renderer, uint32_t frame_index);

 private:
  void RenderCSMFeature(CascadedShadowMapRenderFeature& feature, uint32_t frame_index);

 private:
  ImGuiImplementation& imgui_implementation_;

  struct CachedTexture {
    ImGuiTextureHandle imgui_texture{nullptr};
    TextureHandle      texture{kInvalidRenderResourceHandle};
  };
  PerFrameData<Array<CachedTexture, kCascadedShadowMapCascadesCount>> cached_cascade_textures_;
};

}  // namespace vulture