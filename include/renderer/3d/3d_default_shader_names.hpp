/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file 3d_default_shader_names.hpp
 * @date 2022-05-07
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

#include <cstdint>

namespace vulture {

constexpr uint32_t kMaxUniformNameLength = 128;

//================================================================
//! @defgroup 3D_SHADER_DEFAULT_ATTRIB_NAMES
//! Default shader attributes' names for 3D.
//!
//! @addtogroup 3D_SHADER_DEFAULT_ATTRIB_NAMES
//! @{

constexpr const char* kAttribNameMSPosition = "in_ms_position";
constexpr const char* kAttribNameMSNormal   = "in_ms_normal";
constexpr const char* kAttribNameUV         = "in_uv";

//! @}
//================================================================

//================================================================
//! @defgroup 3D_SHADER_DEFAULT_UNIFORM_NAMES
//! Default shader uniforms' names for 3D.
//!
//! @addtogroup 3D_SHADER_DEFAULT_UNIFORM_NAMES
//! @{

constexpr const char* kUniformNameProjectionView   = "u_projection_view";
constexpr const char* kUniformNameModel            = "u_model";

constexpr const char* kUniformNameWSCamera         = "u_ws_camera";

constexpr const char* kUniformNamePointLights      = "u_point_lights";
constexpr const char* kUniformNamePointLightsCount = "u_point_lights_count";

constexpr const char* kUniformNameMaterial         = "u_material";

//! @}
//================================================================

//================================================================
//! @defgroup 3D_SHADER_DEFAULT_STRUCT_MEMBER_NAMES
//! Default shader struct members' names for 3D.
//!
//! @addtogroup 3D_SHADER_DEFAULT_STRUCT_MEMBER_NAMES
//! @{

constexpr const char* kStructMemberNameWSPosition           = "ws_position";

constexpr const char* kStructMemberNameAmbientColor         = "color_ambient";
constexpr const char* kStructMemberNameDiffuseColor         = "color_diffuse";
constexpr const char* kStructMemberNameSpecularColor        = "color_specular";
constexpr const char* kStructMemberNameSpecularExponent     = "color_specular";

constexpr const char* kStructMemberNameAttenuationLinear    = "attenuation_linear";
constexpr const char* kStructMemberNameAttenuationQuadratic = "attenuation_quadratic";

//! @}
//================================================================

}  // namespace vulture