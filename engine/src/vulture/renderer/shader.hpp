/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file shader.hpp
 * @date 2022-04-27
 *
 * The MIT License (MIT)
 * Copyright (c) vulture-project
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

#include <fmt/format.h>

#include <glm/glm.hpp>
#include <string>
#include <vulture/asset/asset.hpp>
#include <vulture/core/core.hpp>
#include <vulture/renderer/buffer.hpp>

namespace vulture {

/* Rasterization Info */
enum class CullMode : uint32_t {
  kNone,
  kFrontOnly,
  kBackOnly,
  kFrontAndBack
};

/* Depth testing */
enum class CompareOperation {
  kNever,
  kLess,
  kEqual,
  kLessOrEqual,
  kGreater,
  kNotEqual,
  kGreaterOrEqual,
  kAlways
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
enum class ColorBlendFactor {
  kZero,
  kOne,
  
  kSrcColor,
  kOneMinusSrcColor,
  kDstColor,
  kOneMinusDstColor,
  
  kSrcAlpha,
  kOneMinusSrcAlpha,
  kDstAlpha,
  kOneMinusDstAlpha
};

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
enum class ColorBlendOperation {
  kAdd,
  kSubtract,
  kReverseSubtract,
  kMin,
  kMax
};

enum class GeometryPass {
  kNonGeometry,
  kDeferred,
  kForward
};

class Shader : public IAsset {
 public:
  GeometryPass geometry_pass_{GeometryPass::kDeferred};

  CullMode cull_mode_{CullMode::kBackOnly};

  bool enable_depth_test_{true};
  bool enable_depth_write_{true};
  CompareOperation depth_compare_op_{CompareOperation::kLess};

  bool enable_blending_{true};
  ColorBlendOperation blend_op_{ColorBlendOperation::kAdd};
  ColorBlendFactor src_blend_factor_{ColorBlendFactor::kSrcAlpha};
  ColorBlendFactor dst_blend_factor_{ColorBlendFactor::kOneMinusSrcAlpha};

 public:
  static SharedPtr<Shader> Create(const std::string& filename);
  // static SharedPtr<Shader> Create(const std::string& filename_vs, const std::string& filename_fs);
  static SharedPtr<Shader> Create(const std::string& vertex_shader, const std::string& fragment_shader);

  virtual void Bind() const = 0;
  virtual void Unbind() const = 0;

  virtual void SetUpPipeline() const = 0;

  virtual const AttributeLocationMap& GetAttributeLocations() const = 0;

  template<typename... Args>
  void LoadUniformInt(int value, const std::string& format, Args&&... args) {
    LoadUniformInt(value, fmt::format(format, std::forward<Args>(args)...));
  }

  template<typename... Args>
  void LoadUniformFloat(float value, const std::string& format, Args&&... args) {
    LoadUniformFloat(value, fmt::format(format, std::forward<Args>(args)...));
  }

  template<typename... Args>
  void LoadUniformFloat2(const glm::vec2& value, const std::string& format, Args&&... args) {
    LoadUniformFloat2(value, fmt::format(format, std::forward<Args>(args)...));
  }

  template<typename... Args>
  void LoadUniformFloat3(const glm::vec3& value, const std::string& format, Args&&... args) {
    LoadUniformFloat3(value, fmt::format(format, std::forward<Args>(args)...));
  }

  template<typename... Args>
  void LoadUniformFloat4(const glm::vec4& value, const std::string& format, Args&&... args) {
    LoadUniformFloat4(value, fmt::format(format, std::forward<Args>(args)...));
  }

  template<typename... Args>
  void LoadUniformMat4(const glm::mat4& value, const std::string& format, Args&&... args) {
    LoadUniformMat4(value, fmt::format(format, std::forward<Args>(args)...));
  }

  /**
   * @attention Uniform loading functions bind Shader!
   */
  virtual void LoadUniformInt(int value, const std::string& name) = 0;

  /**
   * @attention Uniform loading functions bind Shader!
   */
  virtual void LoadUniformFloat(float value, const std::string& name) = 0;

  /**
   * @attention Uniform loading functions bind Shader!
   */
  virtual void LoadUniformFloat2(const glm::vec2& value, const std::string& name) = 0;

  /**
   * @attention Uniform loading functions bind Shader!
   */
  virtual void LoadUniformFloat3(const glm::vec3& value, const std::string& name) = 0;

  /**
   * @attention Uniform loading functions bind Shader!
   */
  virtual void LoadUniformFloat4(const glm::vec4& value, const std::string& name) = 0;

  /**
   * @attention Uniform loading functions bind Shader!
   */
  virtual void LoadUniformMat4(const glm::mat4& value, const std::string& name) = 0;

  /**
   * @attention Uniform loading functions bind Shader!
   */
  virtual void LoadUniformBool(bool value, const std::string& name) = 0;
};

}  // namespace vulture