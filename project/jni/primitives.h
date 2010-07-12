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

#ifndef O3D_UTILS_PRIMITIVES_H_
#define O3D_UTILS_PRIMITIVES_H_

#include <vector>
#include "core/cross/types.h"

namespace o3d {

class Element;
class FloatField;
class Primitive;
class Pack;

}  // namespace o3d.

namespace o3d_utils {

class Primitives {
 public:
  static void SetBoundingBoxAndZSortPoint(o3d::Element* element);

  static void SetFieldFromVector3s(
      o3d::FloatField* field,
      const std::vector<o3d::Vector3>& vectors);

  static void SetFieldFromPoint3s(
      o3d::FloatField* field,
      const std::vector<o3d::Point3>& points);

  static void ApplyMatrix(
      const o3d::Matrix4& mat,
      std::vector<o3d::Vector3>* vectors);

  static void ApplyMatrix(
      const o3d::Matrix4& mat,
      std::vector<o3d::Point3>* points);

  // Creates a plane on the XZ plane.
  static o3d::Primitive* CreatePlane(
      o3d::Pack* pack,
      float width,
      float depth,
      int subdivisionsWidth,
      int subdivisionsDepth,
      o3d::Matrix4* matrix);

  // Creates a sphere.
  static o3d::Primitive* CreateSphere(
      o3d::Pack* pack,
      float radius,
      int subdivisionsAxis,
      int subdivisionsHeight,
      o3d::Matrix4* matrix);
};

}  // namespace o3d_utils

#endif  // O3D_UTILS_PRIMITIVES_H_

