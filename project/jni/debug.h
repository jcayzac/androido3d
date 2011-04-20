/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Some debugging functions.

#ifndef O3D_UTILS_DEBUG_H_
#define O3D_UTILS_DEBUG_H_

#include <string>
#include <vector>
#include <map>
#include "core/cross/types.h"

namespace o3d {
class RenderNode;
}  // namespace o3d

namespace o3d_utils {
void DumpMultiLineString(const std::string& str);
void DumpPoint3(const o3d::Point3& v, const char* label);
void DumpVector3(const o3d::Vector3& v, const char* label);
void DumpFloat3(const o3d::Float3& v, const char* label);
void DumpVector4(const o3d::Vector4& v, const char* label);
void DumpFloat4(const o3d::Float4& v, const char* label);
void DumpParams(const o3d::ParamObject* obj, const std::string& indent);
void DumpRenderNode(
    const o3d::RenderNode* render_node, const std::string& indent);
void DumpDrawElement(
    const o3d::DrawElement* drawelement, const std::string& indent);
void DumpElement(const o3d::Element* element, const std::string& indent);
void DumpShape(const o3d::Shape* shape, const std::string& indent);
void DumpTransform(const o3d::Transform* transform, const std::string& indent);
void DumpMatrix(const o3d::Matrix4& mat);
bool EndsWith(const std::string& str, const std::string& end);
}  // namespace o3d_utils

#endif  // O3D_UTILS_DEBUG_H_

