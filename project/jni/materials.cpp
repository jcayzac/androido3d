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

#include "materials.h"
#include <string>
#include <vector>
#include "core/cross/effect.h"
#include "core/cross/material.h"
#include "core/cross/pack.h"
#include "core/cross/sampler.h"
#include "core/cross/types.h"
#include "render_graph.h"
#include "shader_builder.h"

namespace o3d_utils {
using namespace o3d;

/**
 * Checks a material's params by name to see if it possibly has non 1.0 alpha.
 * Given a name, checks for a ParamTexture called 'nameTexture' and if that
 * fails, checks for a ParamFloat4 'name'.
 * @private
 * @param {!o3d.Material} material Materal to check.
 * @param {string} name name of color params to check.
 * @return {{found: boolean, nonOneAlpha: boolean}} found is true if one of
 *     the params was found, nonOneAlpha is true if that param had non 1.0
 *     alpha.
 */
bool Materials::hasNonOneAlpha(
    Material* material, const std::string& name,
    bool* nonOneAlpha) {
  bool found = false;
  *nonOneAlpha = false;
  Texture* texture = NULL;
  ParamSampler* samplerParam =
      material->GetParam<ParamSampler>(name + "Sampler");
  if (samplerParam) {
    found = true;
    Sampler* sampler = samplerParam->value();
    if (sampler) {
      texture = sampler->texture();
    }
  } else {
    ParamTexture* textureParam =
        material->GetParam<ParamTexture>(name + "Texture");
    if (textureParam) {
      found = true;
      texture = textureParam->value();
    }
  }

  if (texture && !texture->alpha_is_one()) {
    *nonOneAlpha = true;
  }

  if (!found) {
    ParamFloat4* colorParam = material->GetParam<ParamFloat4>(name);
    if (colorParam) {
      found = true;
      // TODO: this check does not work. We need to check for the
      // <transparency> and <transparent> elements or something.
      // if (colorParam.value[3] < 1) {
      //   nonOneAlpha = true;
      // }
    }
  }
  return found;
};

/**
 * Prepares a material by setting their drawList and possibly creating
 * an standard effect if one does not already exist.
 *
 * This function is very specific to our sample importer. It expects that if
 * no Effect exists on a material that certain extra Params have been created
 * on the Material to give us instructions on what to Effects to create.
 *
 * @param {!o3d.Pack} pack Pack to manage created objects.
 * @param {!o3djs.rendergraph.ViewInfo} viewInfo as returned from
 *     o3djs.rendergraph.createView.
 * @param {!o3d.Material} material to prepare.
 * @param {string} opt_effectType type of effect to create ('phong',
 *     'lambert', 'constant').
 *
 * @see o3djs.material.attachStandardEffect
 */
void Materials::PrepareMaterial(
    Pack* pack, o3d_utils::ViewInfo* view_info, Material* material,
    std::string opt_effect_type) {
  // Assume we want the performance list
  DrawList* draw_list =
      view_info->performance_draw_pass_info()->draw_list();
  // First check if we have a tag telling us that it is or is not
  // transparent
  if (!material->draw_list()) {
    ParamBoolean* param = material->GetParam<ParamBoolean>(
        "collada.transparent");
    if (param) {
      material->set_draw_list(param->value() ?
          view_info->z_ordered_draw_pass_info()->draw_list() :
          view_info->performance_draw_pass_info()->draw_list());
    }
  }

  // If the material has no effect, try to build shaders for it.
  if (!material->effect()) {
    // If the user didn't pass an effect type in see if one was stored there
    // by our importer.
    if (opt_effect_type.empty()) {
      // Retrieve the lightingType parameter from the material, if any.
      std::string lightingType =
          ShaderBuilder::getColladaLightingType(material);
      if (!lightingType.empty()) {
        opt_effect_type = lightingType;
      }
    }
    if (!opt_effect_type.empty()) {
      AttachStandardEffect(pack,
                           material,
                           view_info,
                           opt_effect_type);
      // For collada common profile stuff guess what drawList to use. Note: We
      // can only do this for collada common profile stuff because we supply
      // the shaders and therefore now the inputs and how they are used.
      // For other shaders you've got to do this stuff yourself. On top of
      // that this is a total guess. Just because a texture has no alpha
      // it does not follow that you don't want it in the zOrderedDrawList.
      // That is application specific. Here we are just making a guess and
      // hoping that it covers most cases.
      if (!material->draw_list()) {
        // Check the common profile params.
        bool nonOneAlpha = false;
        bool found = hasNonOneAlpha(material, "diffuse", &nonOneAlpha);
        if (!found) {
          found = hasNonOneAlpha(material, "emissive", &nonOneAlpha);
        }
        if (nonOneAlpha) {
          draw_list = view_info->z_ordered_draw_pass_info()->draw_list();
        }
      }
    }
  }

  if (!material->draw_list()) {
    material->set_draw_list(draw_list);
  }
}

/**
 * Builds a standard effect for a given material.
 * If the material already has an effect, none is created.
 * @param {!o3d.Pack} pack Pack to manage created objects.
 * @param {!o3d.Material} material The material for which to create an
 *     effect.
 * @param {string} effectType Type of effect to create ('phong', 'lambert',
 *     'constant').
 *
 * @see o3djs.effect.attachStandardShader
 */
void Materials::AttachStandardEffectEx(
    Pack* pack,
    Material* material,
    const std::string& effectType) {
  if (!material->effect()) {
    ::o3d::base::scoped_ptr<ShaderBuilder> shader_builder(ShaderBuilder::Create());
    if (!shader_builder->attachStandardShader(
        pack,
        material,
        Vector3(0.0f, 0.0f, 0.0f),
        effectType)) {
      O3D_NEVER_REACHED() << "'Could not attach a standard effect";
    }
  }
};


void Materials::AttachStandardEffect(
    Pack* pack,
    Material* material,
    o3d_utils::ViewInfo* viewInfo,
    const std::string& effectType) {
  if (!material->effect()) {
    ::o3d::base::scoped_ptr<ShaderBuilder> shader_builder(ShaderBuilder::Create());
    Vector3 light_pos =
        Vectormath::Aos::inverse(
            viewInfo->draw_context()->view()).getTranslation();
    if (!shader_builder->attachStandardShader(
        pack, material, light_pos, effectType)) {
       O3D_NEVER_REACHED() << "Could not attach a standard effect";
    }
  }
}

/**
 * Prepares all the materials in the given pack by setting their drawList and
 * if they don't have an Effect, creating one for them.
 *
 * This function is very specific to our sample importer. It expects that if
 * no Effect exists on a material that certain extra Params have been created
 * on the Material to give us instructions on what to Effects to create.
 *
 * @param {!o3d.Pack} pack Pack to prepare.
 * @param {!o3djs.rendergraph.ViewInfo} viewInfo as returned from
 *     o3djs.rendergraph.createView.
 * @param {!o3d.Pack} opt_effectPack Pack to create effects in. If this
 *     is not specifed the pack to prepare above will be used.
 *
 * @see o3djs.material.prepareMaterial
 */
void Materials::PrepareMaterials(
  Pack* pack, o3d_utils::ViewInfo* view_info, Pack* effect_pack) {
  std::vector<Material*> materials = pack->GetByClass<Material>();
  for (size_t ii = 0; ii < materials.size(); ++ii) {
    PrepareMaterial(effect_pack ? effect_pack : pack,
                    view_info,
                    materials[ii], "");
  }
}

/**
 * This function creates a basic material for when you just want to get
 * something on the screen quickly without having to manually setup shaders.
 * You can call this function something like.
 *
 * <pre>
 * &lt;html&gt;&lt;body&gt;
 * &lt;script type="text/javascript" src="o3djs/all.js"&gt;
 * &lt;/script&gt;
 * &lt;script&gt;
 * window.onload = init;
 *
 * function init() {
 *   o3djs.base.makeClients(initStep2);
 * }
 *
 * function initStep2(clientElements) {
 *   var clientElement = clientElements[0];
 *   var client = clientElement.client;
 *   var pack = client.createPack();
 *   var viewInfo = o3djs.rendergraph.createBasicView(
 *       pack,
 *       client.root,
 *       client.renderGraphRoot);
 *   var material = o3djs.material.createBasicMaterial(
 *       pack,
 *       viewInfo,
 *       [1, 0, 0, 1]);  // red
 *   var shape = o3djs.primitives.createCube(pack, material, 10);
 *   var transform = pack.createObject('Transform');
 *   transform.parent = client.root;
 *   transform.addShape(shape);
 *   o3djs.camera.fitContextToScene(client.root,
 *                                  client.width,
 *                                  client.height,
 *                                  viewInfo.drawContext);
 * }
 * &lt;/script&gt;
 * &lt;div id="o3d" style="width: 600px; height: 600px"&gt;&lt;/div&gt;
 * &lt;/body&gt;&lt;/html&gt;
 * </pre>
 *
 * @param {!o3d.Pack} pack Pack to manage created objects.
 * @param {!o3djs.rendergraph.ViewInfo} viewInfo as returned from
 *     o3djs.rendergraph.createBasicView.
 * @param {(!o3djs.math.Vector4|!o3d.Texture)} colorOrTexture Either a color in
 *     the format [r, g, b, a] or an O3D texture.
 * @param {boolean} opt_transparent Whether or not the material is transparent.
 *     Defaults to false.
 * @return {!o3d.Material} The created material.
 */
Material* Materials::CreateBasicMaterial(
    Pack* pack,
    ViewInfo* viewInfo,
    Float4* opt_color,
    Texture* opt_texture,
    bool transparent) {
  O3D_ASSERT(pack);
  O3D_ASSERT(viewInfo);
  O3D_ASSERT(opt_color != NULL || opt_texture != NULL);

  Material* material = pack->Create<Material>();
  material->set_draw_list(transparent ?
      viewInfo->z_ordered_draw_pass_info()->draw_list() :
      viewInfo->performance_draw_pass_info()->draw_list());

  // If it has a length assume it's a color, otherwise assume it's a texture.
  if (opt_color) {
    material->CreateParam<ParamFloat4>("diffuse")->set_value(*opt_color);
  } else {
    ParamSampler* paramSampler =
          material->CreateParam<ParamSampler>("diffuseSampler");
    Sampler* sampler = pack->Create<Sampler>();
    paramSampler->set_value(sampler);
    sampler->set_texture(opt_texture);
  }

  // Create some suitable defaults for the material to save the user having
  // to know all this stuff right off the bat.
  material->CreateParam<ParamFloat4>("emissive")->set_value(
      Float4(0.0f, 0.0f, 0.0f, 1.0f));
  material->CreateParam<ParamFloat4>("ambient")->set_value(
      Float4(0.0f, 0.0f, 0.0f, 1.0f));
  material->CreateParam<ParamFloat4>("specular")->set_value(
      Float4(1.0f, 1.0f, 1.0f, 1.0f));
  material->CreateParam<ParamFloat>("shininess")->set_value(50.0f);
  material->CreateParam<ParamFloat>("specularFactor")->set_value(1.0f);
  material->CreateParam<ParamFloat4>("lightColor")->set_value(
      Float4(1.0f, 1.0f, 1.0f, 1.0f));
  ParamFloat3* lightPositionParam =
      material->CreateParam<ParamFloat3>("lightWorldPos");

  AttachStandardEffect(pack, material, viewInfo, "phong");

  // We have to set the light position after calling attachStandardEffect
  // because attachStandardEffect sets it based on the view.
  lightPositionParam->set_value(Float3(1000.0f, 2000.0f, 3000.0f));

  return material;
};

/**
 * This function creates a constant material. No lighting. It is especially
 * useful for debugging shapes and 2d UI elements.
 *
 * @param {!o3d.Pack} pack Pack to manage created objects.
 * @param {!o3d.DrawList} drawList The DrawList for the material.
 * @param {(!o3djs.math.Vector4|!o3d.Texture)} colorOrTexture Either a color in
 *     the format [r, g, b, a] or an O3D texture.
 * @return {!o3d.Material} The created material.
 */
Material* Materials::CreateConstantMaterialEx(
    Pack* pack,
    DrawList* drawList,
    Float4* opt_color,
    Texture* opt_texture) {
  O3D_ASSERT(pack);
  O3D_ASSERT(drawList);
  O3D_ASSERT(opt_color != NULL || opt_texture != NULL);
  Material* material = pack->Create<Material>();
  material->set_draw_list(drawList);

  // If it has a length assume it's a color, otherwise assume it's a texture.
  if (opt_color) {
    material->CreateParam<ParamFloat4>("emissive")->set_value(*opt_color);
  } else {
    ParamSampler* paramSampler =
          material->CreateParam<ParamSampler>("emissiveSampler");
    Sampler* sampler = pack->Create<Sampler>();
    paramSampler->set_value(sampler);
    sampler->set_texture(opt_texture);
  }

  AttachStandardEffectEx(pack, material, "constant");

  return material;
}

/**
 * This function creates a constant material. No lighting. It is especially
 * useful for debugging shapes and 2d UI elements.
 *
 * @param {!o3d.Pack} pack Pack to manage created objects.
 * @param {!o3djs.rendergraph.ViewInfo} viewInfo as returned from
 *     o3djs.rendergraph.createBasicView.
 * @param {(!o3djs.math.Vector4|!o3d.Texture)} colorOrTexture Either a color in
 *     the format [r, g, b, a] or an O3D texture.
 * @param {boolean} opt_transparent Whether or not the material is transparent.
 *     Defaults to false.
 * @return {!o3d.Material} The created material.
 */
Material* Materials::CreateConstantMaterial(
    Pack* pack,
    o3d_utils::ViewInfo* viewInfo,
    Float4* opt_color,
    Texture* opt_texture,
    bool transparent) {
  O3D_ASSERT(pack);
  O3D_ASSERT(viewInfo);
  O3D_ASSERT(opt_color != NULL || opt_texture != NULL);
  return CreateConstantMaterialEx(
      pack,
      transparent ? viewInfo->z_ordered_draw_pass_info()->draw_list() :
                    viewInfo->performance_draw_pass_info()->draw_list(),
      opt_color,
      opt_texture);
}

/**
 * This function creates 2 color procedureal texture material.
 *
 * @see o3djs.material.createBasicMaterial
 *
 * @param {!o3d.Pack} pack Pack to manage created objects.
 * @param {!o3djs.rendergraph.ViewInfo} viewInfo as returned from
 *     o3djs.rendergraph.createBasicView.
 * @param {!o3djs.math.Vector4} opt_color1 a color in the format [r, g, b, a].
 *     Defaults to a medium blue-green.
 * @param {!o3djs.math.Vector4} opt_color2 a color in the format [r, g, b, a].
 *     Defaults to a light blue-green.
 * @param {boolean} opt_transparent Whether or not the material is transparent.
 *     Defaults to false.
 * @param {number} opt_checkSize Defaults to 10 units.
 * @return {!o3d.Material} The created material.
 */
Material* Materials::CreateCheckerMaterial(
  Pack* pack,
  ViewInfo* viewInfo,
  Float4* opt_color1,
  Float4* opt_color2,
  bool transparent,
  float checkSize) {
  Float4 color1(0.4f, 0.5f, 0.5f, 1.0f);
  Float4 color2(0.6f, 0.8f, 0.8f, 1.0f);
  if (opt_color1) {
    color1 = *opt_color1;
  }
  if (opt_color2) {
    color2 = *opt_color2;
  }

  ::o3d::base::scoped_ptr<ShaderBuilder> builder_(ShaderBuilder::Create());
  Effect* effect = builder_->createCheckerEffect(pack);
  Material* material = pack->Create<Material>();;
  material->set_effect(effect);
  material->set_draw_list(transparent ?
      viewInfo->z_ordered_draw_pass_info()->draw_list() :
      viewInfo->performance_draw_pass_info()->draw_list());
  ShaderBuilder::createUniformParameters(pack, effect, material);

  material->GetParam<ParamFloat4>("color1")->set_value(color1);
  material->GetParam<ParamFloat4>("color2")->set_value(color2);
  material->GetParam<ParamFloat>("checkSize")->set_value(checkSize);

  return material;
};

}  // namespace o3d_utils

