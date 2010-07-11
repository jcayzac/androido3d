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

#include "image_plane.h"
#include <vector>
#include "core/cross/param.h"
#include "core/cross/primitive.h"
#include "core/cross/sampler.h"
#include "core/cross/texture.h"
#include "core/cross/transform.h"
#include "primitives.h"
#include "render_graph.h"

namespace o3d_utils {

static const char* const kImagePlaneShader =
  "uniform mat4 worldViewProjection;\n"
  "\n"
  "attribute vec4 position;\n"
  "attribute vec2 texCoord0;\n"
  "varying vec2 v_texCoord;\n"
  "\n"
  "void main() {\n"
  "  gl_Position = worldViewProjection * position;\n"
  "  v_texCoord = texCoord0;\n"
  "}\n"
  "\n"
  "// #o3d SplitMarker\n"
  "\n"
  "uniform vec4 colorMult;\n"
  "uniform sampler2D texSampler0;\n"
  "varying vec2 v_texCoord;\n"
  "\n"
  "void main() {\n"
  "  gl_FragColor = texture2D(texSampler0, v_texCoord) * colorMult;\n"
  "}\n"
  "// #o3d MatrixLoadOrder RowMajor\n"
  "";

static const char* const kImagePlaneShapeName = "__ImagePlaneShape";
static const char* const kImagePlanePrimitiveName = "__ImagePlanePrimitive";
static const char* const kImagePlaneEffectName = "__ImagePlaneEffect";
static const char* const kImagePlaneMaterialName = "__ImagePlaneMaterial";

ImagePlane* ImagePlane::Create(
    o3d::Pack* plane_pack,
    o3d::Pack* texture_pack,
    o3d_utils::ViewInfo* view_info,
    const std::string& filename,
    bool origin_at_center) {
  o3d::Texture2D* texture = GetTexture(texture_pack, filename);
  if (!texture) {
    return NULL;
  }
  ImagePlane* img = new ImagePlane();
  img->Init(texture, plane_pack, view_info, filename, origin_at_center);
  return img;
}

ImagePlane::ImagePlane()
    : color_mult_param_(NULL),
      transform_(NULL),
      scale_transform_(NULL) {
}

o3d::Texture2D* ImagePlane::GetTexture(
    o3d::Pack* pack, const std::string& filename) {
  // Get Texture from texture pack.
  std::vector<o3d::Texture2D*> textures = pack->Get<o3d::Texture2D>(filename);
  if (!textures.empty()) {
    return textures[0];
  }

  o3d::Texture* texture = pack->CreateTextureFromFile(
      filename, filename, o3d::image::UNKNOWN, true);
  DCHECK(texture->IsA(o3d::Texture2D::GetApparentClass()));
  texture->set_name(filename);
  return down_cast<o3d::Texture2D*>(texture);
}

o3d::Shape* ImagePlane::GetImagePlaneShape(
    o3d::Pack* pack, ViewInfo* view_info) {
  // Let's assume if we find the shape then everything else is already created
  // and conversely if it's not found then nothing is created.
  std::vector<o3d::Shape*> shapes = pack->Get<o3d::Shape>(kImagePlaneShapeName);
  if (!shapes.empty()) {
    return shapes[0];
  }

  // create the effect.
  o3d::Effect* effect = pack->Create<o3d::Effect>();
  effect->set_name(kImagePlaneEffectName);
  bool success = effect->LoadFromFXString(kImagePlaneShader);
  DCHECK(success);

  // Create the material.
  o3d::Material* material = pack->Create<o3d::Material>();
  material->set_name(kImagePlaneMaterialName);
  material->set_draw_list(
      view_info->z_ordered_draw_pass_info()->draw_list());
  material->set_effect(effect);
  effect->CreateUniformParameters(effect);

  // Create the plane.
  o3d::Matrix4 mat(
    o3d::Vector4(1.0f, 0.0f, 0.0f, 0.0f),
    o3d::Vector4(0.0f, 0.0f, 1.0f, 0.0f),
    o3d::Vector4(0.0f,-1.0f, 0.0f, 0.0f),
    o3d::Vector4(0.0f, 0.0f, 0.0f, 1.0f));
  o3d::Primitive* prim = Primitives::CreatePlane(
      pack, 1, 1, 1, 1, &mat);
  prim->set_name(kImagePlanePrimitiveName);
  prim->set_material(material);

  // Create the shape.
  o3d::Shape* shape = pack->Create<o3d::Shape>();
  prim->SetOwner(shape);
  return shape;
}

bool ImagePlane::Init(
    o3d::Texture2D* texture,
    o3d::Pack* pack,
    ViewInfo* view_info,
    const std::string& filename,
    bool origin_at_center) {
  DCHECK(texture);
  DCHECK(pack);
  DCHECK(view_info);

  o3d::Shape* shape = GetImagePlaneShape(pack, view_info);
  shape->set_name(kImagePlaneShapeName);

  // create a transform for positioning
  transform_ = pack->Create<o3d::Transform>();
  transform_->set_name(filename);

  // create a transform for scaling to the size of the image just so
  // we don't have to manage that manually in the transform above.
  scale_transform_ = pack->Create<o3d::Transform>();
  scale_transform_->set_name(filename + "_scale");
  scale_transform_->SetParent(transform_);

  // setup the sampler for the texture
  sampler_ = pack->Create<o3d::Sampler>();
  sampler_->set_address_mode_u(o3d::Sampler::CLAMP);
  sampler_->set_address_mode_v(o3d::Sampler::CLAMP);
  sampler_->set_texture(texture);

  // NOTE: We may need to set the min filter to NEAREST on some hardware
  // or better yet, we should just make O3D do that if it needs to.

  sampler_param_ =
      scale_transform_->CreateParam<o3d::ParamSampler>("texSampler0");
  sampler_param_->set_value(sampler_);

  // Setup the color param
  color_mult_param_ =
      scale_transform_->CreateParam<o3d::ParamFloat4>("colorMult");
  color_mult_param_->set_value(o3d::Float4(1.0f, 1.0f, 1.0f, 1.0f));

  // Setup the scale.
  scale_transform_->AddShape(shape);
  o3d::Matrix4 mat(o3d::Matrix4::identity());
  if (!origin_at_center) {
    mat = mat * o3d::Matrix4::translation(
        o3d::Vector3(texture->width() / 2.0f,
                     texture->height() / 2.0f, 0));
  }
  mat = mat * o3d::Matrix4::scale(
        o3d::Vector3(texture->width(), -texture->height(), 1));
  scale_transform_->set_local_matrix(mat);

  return true;
}

}  // namespace o3d_utils

