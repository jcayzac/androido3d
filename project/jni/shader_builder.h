// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef O3D_UTILS_SHADER_BUILDER_H_
#define O3D_UTILS_SHADER_BUILDER_H_

#include <string>
#include "base/cross/types.h"

namespace o3d {

class Effect;
class Material;
class Pack;
class ParamObject;

}  // namespace o3d_utils

namespace o3d_utils {

class ShaderBuilder {
 public:
  virtual ~ShaderBuilder() { };
  static ShaderBuilder* Create();

  static bool isColladaLightingType(const std::string& lightingType);
  static std::string getColladaLightingType(o3d::Material* material);
  static int getNumTexCoordStreamsNeeded(o3d::Material* material);

  static void createUniformParameters(
      o3d::Pack* pack,
      o3d::Effect* effect,
      o3d::ParamObject* paramObject);

  virtual o3d::Effect* createCheckerEffect(o3d::Pack* pack) = 0;
  virtual bool attachStandardShader(
      o3d::Pack* pack,
      o3d::Material* material,
      const o3d::Vector3& lightPos,
      const std::string& effectType) = 0;

};

}  // namespace o3d_utils

#endif  // O3D_UTILS_SHADER_BUILDER_H_


