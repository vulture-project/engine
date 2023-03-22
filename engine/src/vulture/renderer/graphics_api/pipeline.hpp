/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file pipeline.hpp
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

#include <stdint.h>

#include <vector>
#include <vulture/renderer/graphics_api/data_format.hpp>
#include <vulture/renderer/graphics_api/descriptor.hpp>
#include <vulture/renderer/graphics_api/render_resource_handles.hpp>

namespace vulture {

/* Pipeline Stages */
enum PipelineStageBit : uint32_t {
  kPipelineStageBitNone = 0x0000'0000,
  
  // ...
  
  /** @brief Stage in which indirect draw structures are consumed */
  kPipelineStageBitDrawIndirect = 0x0000'0002,

  /** @brief Stage in which vertex and index buffers are consumed */
  kPipelineStageBitVertexInput = 0x0000'0004,

  /** @brief Vertex shader stage */
  kPipelineStageBitVertexShade = 0x0000'0008,
  
  // ...

  /** 
   * @brief Stage in which depth/stencil tests before the fragment shader are performed. Additionally, in this stage
   *        load operations are performed for framebuffer depth/stencil attachments.
   */
  kPipelineStageBitEarlyFragmentTests = 0x0000'0100,

  /** @brief Fragment shader stage */
  kPipelineStageBitFragmentShader = 0x0000'0080,

  /**
   * @brief Stage in which depth/stencil tests after the fragment shader are performed. Additionally, in this stage
   *        store operations are performed for framebuffer depth/stencil attachments.
   */
  kPipelineStageBitLateFragmentTests = 0x0000'0200,

  /**
   * @brief Stage in which the final color values are output from the pipeline.
   * 
   * This stage is after:
   * 1. Blending final colors
   * 2. Subpass load and store operations
   * 3. Multisample resolve
   */
  kPipelineStageBitColorAttachmentOutput = 0x0000'0400,

  /** @brief Specifies all copy commands */
  kPipelineStageBitTransfer = 0x0000'1000
};

using PipelineStageFlags = uint32_t;

/* Memory access dependency */
enum MemoryAccessDependencyBit : uint32_t {
  kMemoryAccessBitNone = 0,

  kMemoryAccessBitIndirectCommandRead         = 0x0000'0001,
  kMemoryAccessBitIndexRead                   = 0x0000'0002,
  kMemoryAccessBitVertexAttributeRead         = 0x0000'0004,
  kMemoryAccessBitUniformRead                 = 0x0000'0008,
  kMemoryAccessBitInputAttachmentRead         = 0x0000'0010,
  kMemoryAccessBitShaderRead                  = 0x0000'0020,
  kMemoryAccessBitShaderWrite                 = 0x0000'0040,
  kMemoryAccessBitColorAttachmentRead         = 0x0000'0080,
  kMemoryAccessBitColorAttachmentWrite        = 0x0000'0100,
  kMemoryAccessBitDepthStencilAttachmentRead  = 0x0000'0200,
  kMemoryAccessBitDepthStencilAttachmentWrite = 0x0000'0400,
  kMemoryAccessBitTransferRead                = 0x0000'0800,
  kMemoryAccessBitTransferWrite               = 0x0000'1000,
  kMemoryAccessBitHostRead                    = 0x0000'2000,
  kMemoryAccessBitHostWrite                   = 0x0000'4000,
  kMemoryAccessBitMemoryRead                  = 0x0000'8000,
  kMemoryAccessBitMemoryWrite                 = 0x0001'0000,
};

using MemoryAccessDependencyFlags = uint32_t;

/* Shader Attribute Layout */
struct ShaderAttributeSpec {
  uint32_t    location{0};
  std::string name;
  DataFormat  format{DataFormat::kInvalid};
};

struct ShaderAttributeLayout {
  // TODO:
};

/* Input Info */
struct InputVertexDataInfo {
  struct VertexAttributeInfo {
    uint32_t   location{0};
    DataFormat format{DataFormat::kInvalid};
    uint32_t   offset{0};
  };

  struct BindingInfo {
    uint32_t binding_slot{0};
    uint32_t stride{0};

    std::vector<VertexAttributeInfo> attributes_info{};
  };

  std::vector<BindingInfo> bindings{};
};

DECLARE_ENUM_TO_STR(
    Topology,
    kPointList,  ///< Specifies a series of separate points

    kLineList,   ///< Specifies a series of separate lines
    kLineStrip,  ///< Specifies a series of connected lines, with consecutive ones sharing a vertex

    kTriangleList,   ///< Specifies a series of separate triangles
    kTriangleStrip,  ///< Specifies a series of connected triangles, with consecutive ones sharing an edge
    kTriangleFan);   ///< Specifies a series of connected triangles, with all ones sharing a common vertex

struct InputAssemblyInfo {
  Topology topology{Topology::kTriangleList};
};

/* Rasterization Info */
DECLARE_ENUM_TO_STR(CullMode, kNone, kFrontOnly, kBackOnly, kFrontAndBack);
DECLARE_ENUM_TO_STR(FrontFace, kClockwise, kCounterClockwise);

DECLARE_ENUM_TO_STR(PolygonMode,
                    kFill,   ///< Fill the polygon
                    kLine);  ///< Only render edges of the polygon

struct RasterizationInfo {
  CullMode    cull_mode    {CullMode::kNone};
  FrontFace   front_face   {FrontFace::kCounterClockwise};
  PolygonMode polygon_mode {PolygonMode::kFill};
};

/* Depth and stencil testing */
DECLARE_ENUM_TO_STR(CompareOperation, kNever, kLess, kEqual, kLessOrEqual, kGreater, kNotEqual, kGreaterOrEqual,
                    kAlways);

struct DepthTestDescription {
  bool             test_enable  {true};
  bool             write_enable {true};
  CompareOperation compare_op   {CompareOperation::kNever};
};

struct StencilTestDescription {
  /* TODO: */
};

/* Color attachment blending */
/**
 * @brief Specifies blending factor.
 * 
 * Let
 * 1. R_src, G_src, B_src, A_src - source color components
 * 2. R_dst, G_dst, B_dst, A_dst - destination color components
 * 
 * Then factors are defined as follows:
 *
 * Factor            | RGB blend factors                 | Alpha blend factor             |
 * -----------------:|:---------------------------------:|:------------------------------:|
 * kZero             | (0, 0, 0)                         | 0                              |
 * kOne              | (1, 1, 1)                         | 1                              |
 * kSrcColor         | (R_src, G_src, B_src)             | A_src                          |
 * kOneMinusSrcColor | (1 - R_src, 1 - G_src, 1 - B_src) | 1 - A_src                      |
 * kDstColor         | (R_dst, G_dst, B_dst)             | A_dst                          |
 * kOneMinusDstColor | (1 - R_dst, 1 - G_dst, 1 - B_dst) | 1 - A_dst                      |
 * kSrcAlpha         | (A_src, A_src, A_src)             | A_src                          |
 * kOneMinusSrcAlpha | (1 - A_src, 1 - A_src, 1 - A_src) | 1 - A_src                      |
 * kDstAlpha         | (A_dst, A_dst, A_dst)             | A_dst                          |
 * kOneMinusDstAlpha | (1 - A_dst, 1 - A_dst, 1 - A_dst) | 1 - A_dst                      |
 */
DECLARE_ENUM_TO_STR(ColorBlendFactor,
                    kZero,
                    kOne,

                    kSrcColor,
                    kOneMinusSrcColor,
                    kDstColor,
                    kOneMinusDstColor,

                    kSrcAlpha,
                    kOneMinusSrcAlpha,
                    kDstAlpha,
                    kOneMinusDstAlpha);

/**
 * @brief Specifies blending operation.
 * 
 * Let
 * 1. R_src, G_src, B_src, A_src - source color components
 * 2. R_dst, G_dst, B_dst, A_dst - destination color components
 * 3. SF_r, SF_g, SF_b, SF_a - source blend factor components
 * 4. DF_r, DF_g, DF_b, DF_a - destination blend factor components
 * 
 * Then operations are defined as follows:
 * 
 * Operation        | Final R/G/B                    | Final A                        |
 * ----------------:|:------------------------------:|:------------------------------:|
 * kAdd             | R_src * SF_r + R_dst * DF_r    | A_src * SF_a + A_dst * DF_a    |
 * kSubtract        | R_src * SF_r - R_dst * DF_r    | A_src * SF_a - A_dst * DF_a    |
 * kReverseSubtract | R_dst * DF_r - R_src * SF_r    | A_dst * DF_a - A_src * SF_a    |
 * kMin             | min(R_src, R_dst)              | min(A_src, A_dst)              |
 * kMax             | max(R_src, R_dst)              | max(A_src, A_dst)              |
 */
DECLARE_ENUM_TO_STR(ColorBlendOperation, kAdd, kSubtract, kReverseSubtract, kMin, kMax);

struct ColorAttachmentsBlendDescription {
  bool enable{true};

  ColorBlendFactor    src_color_blend_factor {ColorBlendFactor::kZero};
  ColorBlendFactor    dst_color_blend_factor {ColorBlendFactor::kZero};
  ColorBlendOperation color_blend_op         {ColorBlendOperation::kAdd};

  ColorBlendFactor    src_alpha_blend_factor {ColorBlendFactor::kZero};
  ColorBlendFactor    dst_alpha_blend_factor {ColorBlendFactor::kZero};
  ColorBlendOperation alpha_blend_op         {ColorBlendOperation::kAdd};
};

/* Push constants */
struct PushConstantRange {
  uint32_t         offset        {0};
  uint32_t         size          {0};
  ShaderStageFlags shader_stages {kShaderStageBitNone};
};

/* Pipeline */
constexpr uint32_t kMaxPipelineShaderModules      = 4;
constexpr uint32_t kMaxPipelineDescriptorSets     = 4;
constexpr uint32_t kMaxPipelinePushConstantRanges = 4;

struct PipelineDescription {
  const InputVertexDataInfo*       input_vertex_data_info{nullptr};
  InputAssemblyInfo                input_assembly_info{};
  
  uint32_t                         descriptor_sets_count{0};
  DescriptorSetLayoutHandle        descriptor_set_layouts[kMaxPipelineDescriptorSets]{kInvalidRenderResourceHandle};

  uint32_t                         push_constant_ranges_count{0};
  PushConstantRange                push_constant_ranges[kMaxPipelinePushConstantRanges];

  uint32_t                         shader_modules_count{0};
  ShaderModuleHandle               shader_modules[kMaxPipelineShaderModules]{kInvalidRenderResourceHandle};

  RasterizationInfo                rasterization_info{};
  DepthTestDescription             depth_test_description{};
  StencilTestDescription           stencil_test_description{};
  ColorAttachmentsBlendDescription blend_description{};
};

}  // namespace vulture
