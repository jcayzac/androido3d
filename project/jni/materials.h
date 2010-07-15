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

#ifndef O3D_UTILS_MATERAILS_H_
#define O3D_UTILS_MATERAILS_H_

#include <string>
#include "core/cross/types.h"

namespace o3d {

class DrawList;
class Element;
class Material;
class Pack;
class Texture;

}  // namespace o3d.

namespace o3d_utils {

class ViewInfo;

class Materials {
 public:
  // Prepares all the materials in a pack assuming they are materials as
  // imported from the collada loader.
  static void PrepareMaterials(
      o3d::Pack* pack, o3d_utils::ViewInfo* view_info, o3d::Pack* effect_pack);

  // Prepares a material assuming it was imported from the collada loader.
  static void PrepareMaterial(
      o3d::Pack* pack, o3d_utils::ViewInfo* view_info, o3d::Material* material,
      std::string opt_effect_type);

  static void AttachStandardEffect(
      o3d::Pack* pack,
      o3d::Material* material,
      o3d_utils::ViewInfo* viewInfo,
      const std::string& effectType);

  static void AttachStandardEffectEx(
      o3d::Pack* pack,
      o3d::Material* material,
      const std::string& effectType);

  o3d::Material* CreateBasicMaterial(
      o3d::Pack* pack,
      o3d_utils::ViewInfo* viewInfo,
      o3d::Float4* opt_color,
      o3d::Texture* opt_texture,
      bool transparent);

  o3d::Material* CreateConstantMaterialEx(
      o3d::Pack* pack,
      o3d::DrawList* drawList,
      o3d::Float4* opt_color,
      o3d::Texture* opt_texture);

  o3d::Material* CreateConstantMaterial(
      o3d::Pack* pack,
      o3d_utils::ViewInfo* viewInfo,
      o3d::Float4* opt_color,
      o3d::Texture* opt_texture,
      bool transparent);

  static o3d::Material* CreateCheckerMaterial(
      o3d::Pack* pack,
      o3d_utils::ViewInfo* viewInfo,
      o3d::Float4* opt_color1 = NULL,
      o3d::Float4* opt_color2 = NULL,
      bool transparent = false,
      float checkSize = 10.0f);

 private:
  static bool hasNonOneAlpha(
      o3d::Material* material, const std::string& name,
      bool* nonOneAlpha);
};

}  // namespace o3d_utils

#endif  // O3D_UTILS_MATERAILS_H_

