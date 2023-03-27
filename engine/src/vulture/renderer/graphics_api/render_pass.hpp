/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file render_pass.hpp
 * @date 2023-03-18
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

#include <optional>
#include <string>
#include <vector>
#include <vulture/core/core.hpp>
#include <vulture/renderer/graphics_api/pipeline.hpp>
#include <vulture/renderer/graphics_api/texture.hpp>

namespace vulture {

/* Render Pass Attachment */
DECLARE_ENUM_TO_STR(AttachmentLoad, kLoad, kClear, kDontCare);
DECLARE_ENUM_TO_STR(AttachmentStore, kStore, kDontCare);

struct AttachmentDescription {
  DataFormat      format         {DataFormat::kInvalid};
  uint32_t        samples        {1};

  AttachmentLoad  load_op        {AttachmentLoad::kDontCare};
  AttachmentStore store_op       {AttachmentStore::kDontCare};

  TextureLayout   initial_layout {TextureLayout::kUndefined};
  TextureLayout   final_layout    {TextureLayout::kUndefined};
};

/* Subpass */
struct AttachmentReference {
  uint32_t      attachment_idx {0};
  TextureLayout layout         {TextureLayout::kUndefined};
};

enum class SubpassPipelineBindPoint : uint32_t {
  kInvalid,
  kGraphics,
  kCompute
};

struct SubpassDescription {
  SubpassPipelineBindPoint bind_point{SubpassPipelineBindPoint::kInvalid};

  /** @note All color attachments should have the same number of samples! **/
  std::vector<AttachmentReference> color_attachments;

  /**
   * @brief Used for resolving multisampling.
   * @note If there are resolve attachments, then there should be a resolve attachment for each color attachment!
   */
  std::vector<AttachmentReference> resolve_attachments;
  
  std::optional<AttachmentReference> depth_stencil_attachment;

  std::vector<AttachmentReference> input_attachments;

  /** @brief Attachments not used by the subpass, but for which the data must be preserved. */
  std::vector<uint32_t> preserve_attachments;
};

struct SubpassDependency {
  uint32_t                    dependency_subpass_idx {0};
  uint32_t                    dependent_subpass_idx  {0};

  PipelineStageFlags          dependency_stage_mask  {kPipelineStageBitNone};
  PipelineStageFlags          dependent_stage_mask   {kPipelineStageBitNone};

  MemoryAccessDependencyFlags dependency_access_mask {kMemoryAccessBitNone};
  MemoryAccessDependencyFlags dependent_access_mask  {kMemoryAccessBitNone};
};

/* Clear Value */
union ClearColorValue {
  float    rgba_float32[4];
  int32_t  rgba_int32[4];
  uint32_t rgba_uint32[4];
};

struct ClearDepthStencilValue {
  float    depth;
  uint32_t stencil;
};

union ClearValue {
  ClearColorValue        color;
  ClearDepthStencilValue depth_stencil;

  ClearValue() = default;
  ClearValue(float r, float g, float b, float a) : color({.rgba_float32={r, g, b, a}}) {}
  ClearValue(const glm::vec4& rgba) : color({.rgba_float32={rgba.r, rgba.g, rgba.b, rgba.a}}) {}
  ClearValue(float depth, uint32_t stencil) : depth_stencil({depth, stencil}) {}
};

/* Render Pass */
struct RenderPassDescription {
  std::vector<AttachmentDescription> attachments;
  std::vector<SubpassDescription>    subpasses;
  std::vector<SubpassDependency>     subpass_dependencies;
};

}  // namespace vulture
