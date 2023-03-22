/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file vulkan_utils.hpp
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

#include <vulture/renderer/graphics_api/vulkan/vulkan_render_device.hpp>

namespace vulture {

inline VkFormat GetVKFormat(DataFormat format) {
  switch (format) {
    /* One-component */
    case (DataFormat::kR32_UINT):            { return VK_FORMAT_R32_UINT; }
    case (DataFormat::kR32_SINT):            { return VK_FORMAT_R32_SINT; }
    case (DataFormat::kR32_SFLOAT):          { return VK_FORMAT_R32_SFLOAT; }

    case (DataFormat::kD16_UNORM):           { return VK_FORMAT_D16_UNORM; }
    case (DataFormat::kD32_SFLOAT):          { return VK_FORMAT_D32_SFLOAT; }

    /* Two-component */
    case (DataFormat::kR32G32_UINT):         { return VK_FORMAT_R32G32_UINT; }
    case (DataFormat::kR32G32_SINT):         { return VK_FORMAT_R32G32_SINT; }
    case (DataFormat::kR32G32_SFLOAT):       { return VK_FORMAT_R32G32_SFLOAT; }

    case (DataFormat::kD24_UNORM_S8_UINT):   { return VK_FORMAT_D24_UNORM_S8_UINT; }

    /* Three-component */
    case (DataFormat::kR8G8B8_UNORM):        { return VK_FORMAT_R8G8B8_UNORM; }
    case (DataFormat::kR8G8B8_SRGB):         { return VK_FORMAT_R8G8B8_SRGB; }
    case (DataFormat::kR32G32B32_SFLOAT):    { return VK_FORMAT_R32G32B32_SFLOAT; }

    /* Four-component */
    case (DataFormat::kR8G8B8A8_UNORM):      { return VK_FORMAT_R8G8B8A8_UNORM; }
    case (DataFormat::kR8G8B8A8_SRGB):       { return VK_FORMAT_R8G8B8A8_SRGB; }
    case (DataFormat::kB8G8R8A8_SRGB):       { return VK_FORMAT_B8G8R8A8_SRGB; }
    case (DataFormat::kR32G32B32A32_SFLOAT): { return VK_FORMAT_R32G32B32A32_SFLOAT; }

    default: { assert(!"Invalid DataFormat!"); }
  }
}

inline DataFormat GetDataFormatFromVk(VkFormat vk_format) {
  switch (vk_format) {
    /* One-component */
    case (VK_FORMAT_R32_UINT):            { return DataFormat::kR32_UINT; }
    case (VK_FORMAT_R32_SINT):            { return DataFormat::kR32_SINT; }
    case (VK_FORMAT_R32_SFLOAT):          { return DataFormat::kR32_SFLOAT; }

    case (VK_FORMAT_D16_UNORM):           { return DataFormat::kD16_UNORM; }
    case (VK_FORMAT_D32_SFLOAT):          { return DataFormat::kD32_SFLOAT; }

    /* Two-component */
    case (VK_FORMAT_R32G32_UINT):         { return DataFormat::kR32G32_UINT; }
    case (VK_FORMAT_R32G32_SINT):         { return DataFormat::kR32G32_SINT; }
    case (VK_FORMAT_R32G32_SFLOAT):       { return DataFormat::kR32G32_SFLOAT; }

    case (VK_FORMAT_D24_UNORM_S8_UINT):   { return DataFormat::kD24_UNORM_S8_UINT; }

    /* Three-component */
    case (VK_FORMAT_R8G8B8_UNORM):        { return DataFormat::kR8G8B8_UNORM; }
    case (VK_FORMAT_R8G8B8_SRGB):         { return DataFormat::kR8G8B8_SRGB; }
    case (VK_FORMAT_R32G32B32_SFLOAT):    { return DataFormat::kR32G32B32_SFLOAT; }

    /* Four-component */
    case (VK_FORMAT_R8G8B8A8_UNORM):      { return DataFormat::kR8G8B8A8_UNORM; }
    case (VK_FORMAT_R8G8B8A8_SRGB):       { return DataFormat::kR8G8B8A8_SRGB; }
    case (VK_FORMAT_B8G8R8A8_SRGB):       { return DataFormat::kB8G8R8A8_SRGB; }
    case (VK_FORMAT_R32G32B32A32_SFLOAT): { return DataFormat::kR32G32B32A32_SFLOAT; }

    default: { assert(!"Invalid VkFormat!"); }
  }
}

inline uint32_t GetMaxMSAASamples(const VkPhysicalDeviceProperties& properties) {
  VkSampleCountFlags counts =
      properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts;

  uint32_t max_msaa_samples = 1;
  if      (counts & VK_SAMPLE_COUNT_64_BIT) { max_msaa_samples = 64; }
  else if (counts & VK_SAMPLE_COUNT_32_BIT) { max_msaa_samples = 32; }
  else if (counts & VK_SAMPLE_COUNT_16_BIT) { max_msaa_samples = 16; }
  else if (counts & VK_SAMPLE_COUNT_8_BIT)  { max_msaa_samples = 8;  }
  else if (counts & VK_SAMPLE_COUNT_4_BIT)  { max_msaa_samples = 4;  }
  else if (counts & VK_SAMPLE_COUNT_2_BIT)  { max_msaa_samples = 2;  }

  return max_msaa_samples;
}

inline VkFilter GetVKFilter(SamplerFilter filter) {
  switch (filter) {
    case SamplerFilter::kNearest: { return VK_FILTER_NEAREST; }
    case SamplerFilter::kLinear:  { return VK_FILTER_LINEAR; }
    default: { assert(!"Invalid SamplerFilter!"); }
  }
}

inline VkSamplerAddressMode GetVKSamplerAddressMode(SamplerAddressMode address_mode) {
  switch (address_mode) {
    case SamplerAddressMode::kRepeat:         { return VK_SAMPLER_ADDRESS_MODE_REPEAT; }
    case SamplerAddressMode::kMirroredRepeat: { return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT; }
    case SamplerAddressMode::kClampToEdge:    { return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; }
    case SamplerAddressMode::kClampToBorder:  { return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER; }

    default: { assert(!"Invalid SamplerAddressMode!"); }
  }
}

inline VkBorderColor GetVKBorderColor(SamplerBorderColor border_color) {
  switch (border_color) {
    case SamplerBorderColor::kFloatTransparentBlack: { return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK; }
    case SamplerBorderColor::kIntTransparentBlack:   { return VK_BORDER_COLOR_INT_TRANSPARENT_BLACK; }
    case SamplerBorderColor::kFloatOpaqueBlack:      { return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK; }
    case SamplerBorderColor::kIntOpaqueBlack:        { return VK_BORDER_COLOR_INT_OPAQUE_BLACK; }
    case SamplerBorderColor::kFloatOpaqueWhite:      { return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE; }
    case SamplerBorderColor::kIntOpaqueWhite:        { return VK_BORDER_COLOR_INT_OPAQUE_WHITE; }

    default: { assert(!"Invalid SamplerBorderColor!"); }
  }
}

inline VkSamplerMipmapMode GetVKSamplerMipmapMode(SamplerMipmapMode mipmap_mode) {
  switch (mipmap_mode) {
    case SamplerMipmapMode::kNearest: { return VK_SAMPLER_MIPMAP_MODE_NEAREST; }
    case SamplerMipmapMode::kLinear:  { return VK_SAMPLER_MIPMAP_MODE_LINEAR; }

    default: { assert(!"Invalid SamplerMipmapMode!"); }
  }
}

inline VkDescriptorType GetVKDescriptorType(DescriptorType type) {
  switch (type) {
    case DescriptorType::kUniformBuffer:   { return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; }
    case DescriptorType::kStorageBuffer:   { return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; }
    case DescriptorType::kInputAttachment: { return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT; }
    case DescriptorType::kTextureSampler:  { return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; }

    default: { assert(!"Invalid DescriptorType!"); }
  }
}

inline VkAttachmentLoadOp GetVKAttachmentLoadOperation(AttachmentLoadOperation operation) {
  switch (operation) {
    case AttachmentLoadOperation::kLoad:     { return VK_ATTACHMENT_LOAD_OP_LOAD; }
    case AttachmentLoadOperation::kClear:    { return VK_ATTACHMENT_LOAD_OP_CLEAR; }
    case AttachmentLoadOperation::kDontCare: { return VK_ATTACHMENT_LOAD_OP_DONT_CARE; }

    default: { assert(!"Invalid AttachmentLoadOperation!"); }
  }
}

inline VkAttachmentStoreOp GetVKAttachmentStoreOperation(AttachmentStoreOperation operation) {
  switch (operation) {
    case AttachmentStoreOperation::kStore:    { return VK_ATTACHMENT_STORE_OP_STORE; }
    case AttachmentStoreOperation::kDontCare: { return VK_ATTACHMENT_STORE_OP_DONT_CARE; }

    default: { assert(!"Invalid AttachmentStoreOperation!"); }
  }
}

inline VkImageLayout GetVKImageLayout(TextureLayout layout) {
  switch (layout) {
    case TextureLayout::kUndefined:              { return VK_IMAGE_LAYOUT_UNDEFINED; }
    case TextureLayout::kGeneral:                { return VK_IMAGE_LAYOUT_GENERAL; }
    case TextureLayout::kPresentSrc:             { return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; }

    case TextureLayout::kColorAttachment:        { return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; }
    case TextureLayout::kDepthStencilAttachment: { return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; }
    case TextureLayout::kDepthStencilReadOnly:   { return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL; }
    case TextureLayout::kShaderReadOnly:         { return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; }

    case TextureLayout::kTransferSrc:            { return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL; }
    case TextureLayout::kTransferDst:            { return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL; }

    default: { assert(!"Invalid TextureLayout!"); }
  }
}

inline VkPipelineBindPoint GetVKPipelineBindPoint(SubpassPipelineBindPoint bind_point) {
  switch (bind_point) {
    case SubpassPipelineBindPoint::kGraphics: { return VK_PIPELINE_BIND_POINT_GRAPHICS; }
    case SubpassPipelineBindPoint::kCompute:  { return VK_PIPELINE_BIND_POINT_COMPUTE; }

    default: { assert(!"Invalid SubpassPipelineBindPoint!"); }
  }
}

inline VkPrimitiveTopology GetVKPrimitiveTopology(Topology topology) {
  switch (topology) {
    case Topology::kPointList:     { return VK_PRIMITIVE_TOPOLOGY_POINT_LIST; }

    case Topology::kLineList:      { return VK_PRIMITIVE_TOPOLOGY_LINE_LIST; }
    case Topology::kLineStrip:     { return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP; }

    case Topology::kTriangleList:  { return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; }
    case Topology::kTriangleStrip: { return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP; }
    case Topology::kTriangleFan:   { return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN; }

    default: { assert(!"Invalid Topology!"); }
  }
}

inline VkCullModeFlagBits GetVKCullMode(CullMode cull_mode) {
  switch (cull_mode) {
    case CullMode::kNone:         { return VK_CULL_MODE_NONE; }
    case CullMode::kFrontOnly:    { return VK_CULL_MODE_FRONT_BIT; }
    case CullMode::kBackOnly:     { return VK_CULL_MODE_BACK_BIT; }
    case CullMode::kFrontAndBack: { return VK_CULL_MODE_FRONT_AND_BACK; }

    default: { assert(!"Invalid CullMode!"); }
  }
}

inline VkFrontFace GetVKFrontFace(FrontFace front_face) {
  switch (front_face) {
    case FrontFace::kClockwise:        { return VK_FRONT_FACE_CLOCKWISE; }
    case FrontFace::kCounterClockwise: { return VK_FRONT_FACE_COUNTER_CLOCKWISE; }

    default: { assert(!"Invalid FrontFace!"); }
  }
}

inline VkPolygonMode GetVKPolygonMode(PolygonMode polygon_mode) {
  switch (polygon_mode) {
    case PolygonMode::kFill: { return VK_POLYGON_MODE_FILL; }
    case PolygonMode::kLine: { return VK_POLYGON_MODE_LINE; }

    default: { assert(!"Invalid PolygonMode!"); }
  }
}

inline VkCompareOp GetVKCompareOp(CompareOperation op) {
  switch (op) {
    case CompareOperation::kNever:          { return VK_COMPARE_OP_NEVER; }
    case CompareOperation::kLess:           { return VK_COMPARE_OP_LESS; }
    case CompareOperation::kEqual:          { return VK_COMPARE_OP_EQUAL; }
    case CompareOperation::kLessOrEqual:    { return VK_COMPARE_OP_LESS_OR_EQUAL; }
    case CompareOperation::kGreater:        { return VK_COMPARE_OP_GREATER; }
    case CompareOperation::kNotEqual:       { return VK_COMPARE_OP_NOT_EQUAL; }
    case CompareOperation::kGreaterOrEqual: { return VK_COMPARE_OP_GREATER_OR_EQUAL; }
    case CompareOperation::kAlways:         { return VK_COMPARE_OP_ALWAYS; }

    default: { assert(!"Invalid CompareOperation!"); }
  }
}

inline VkBlendFactor GetVKBlendFactor(ColorBlendFactor factor) {
  switch (factor) {
    case ColorBlendFactor::kZero:             { return VK_BLEND_FACTOR_ZERO; }
    case ColorBlendFactor::kOne:              { return VK_BLEND_FACTOR_ONE; }

    case ColorBlendFactor::kSrcColor:         { return VK_BLEND_FACTOR_SRC_COLOR; }
    case ColorBlendFactor::kOneMinusSrcColor: { return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR; }
    case ColorBlendFactor::kDstColor:         { return VK_BLEND_FACTOR_DST_COLOR; }
    case ColorBlendFactor::kOneMinusDstColor: { return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR; }

    case ColorBlendFactor::kSrcAlpha:         { return VK_BLEND_FACTOR_SRC_ALPHA; }
    case ColorBlendFactor::kOneMinusSrcAlpha: { return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; }
    case ColorBlendFactor::kDstAlpha:         { return VK_BLEND_FACTOR_DST_ALPHA; }
    case ColorBlendFactor::kOneMinusDstAlpha: { return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA; }

    default: { assert(!"Invalid ColorBlendFactor!"); }
  }
}

inline VkBlendOp GetVKBlendOp(ColorBlendOperation op) {
  switch (op) {
    case ColorBlendOperation::kAdd:             { return VK_BLEND_OP_ADD; }
    case ColorBlendOperation::kSubtract:        { return VK_BLEND_OP_SUBTRACT; }
    case ColorBlendOperation::kReverseSubtract: { return VK_BLEND_OP_REVERSE_SUBTRACT; }
    case ColorBlendOperation::kMin:             { return VK_BLEND_OP_MIN; }
    case ColorBlendOperation::kMax:             { return VK_BLEND_OP_MAX; }

    default: { assert(!"Invalid ColorBlendOperation!"); }
  }
}

}  // namespace vulture
