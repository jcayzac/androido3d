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

// OpenGL ES 2.0 code

#include <map>
#include <set>
#include <string>
#include <vector>
#include "scene.h"
#include "core/cross/client.h"
#include "core/cross/curve.h"
#include "core/cross/pack.h"
#include "core/cross/param_array.h"
#include "core/cross/primitive.h"
#include "core/cross/sampler.h"
#include "core/cross/shape.h"
#include "core/cross/skin.h"
#include "core/cross/transform.h"
#include "core/cross/types.h"
#include "import/cross/collada.h"
#include "shader_builder.h"
#include "primitives.h"
#include "render_graph.h"

namespace o3d_utils {

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
bool Scene::hasNonOneAlpha(
    o3d::Material* material, const std::string& name,
    bool* nonOneAlpha) {
  bool found = false;
  *nonOneAlpha = false;
  o3d::Texture* texture = NULL;
  o3d::ParamSampler* samplerParam =
      material->GetParam<o3d::ParamSampler>(name + "Sampler");
  if (samplerParam) {
    found = true;
    o3d::Sampler* sampler = samplerParam->value();
    if (sampler) {
      texture = sampler->texture();
    }
  } else {
    o3d::ParamTexture* textureParam =
        material->GetParam<o3d::ParamTexture>(name + "Texture");
    if (textureParam) {
      found = true;
      texture = textureParam->value();
    }
  }

  if (texture && !texture->alpha_is_one()) {
    *nonOneAlpha = true;
  }

  if (!found) {
    o3d::ParamFloat4* colorParam = material->GetParam<o3d::ParamFloat4>(name);
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
void Scene::PrepareMaterial(
    o3d::Pack* pack, o3d_utils::ViewInfo* view_info, o3d::Material* material,
    std::string opt_effect_type) {
  // Assume we want the performance list
  o3d::DrawList* draw_list =
      view_info->performance_draw_pass_info()->draw_list();
  // First check if we have a tag telling us that it is or is not
  // transparent
  if (!material->draw_list()) {
    o3d::ParamBoolean* param = material->GetParam<o3d::ParamBoolean>(
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

void Scene::AttachStandardEffect(o3d::Pack* pack,
                                 o3d::Material* material,
                                 o3d_utils::ViewInfo* viewInfo,
                                 const std::string& effectType) {
  if (!material->effect()) {
    scoped_ptr<ShaderBuilder> shader_builder(ShaderBuilder::Create());
    o3d::Vector3 light_pos =
        Vectormath::Aos::inverse(
            viewInfo->draw_context()->view()).getTranslation();
    if (!shader_builder->attachStandardShader(
        pack, material, light_pos, effectType)) {
       NOTREACHED() << "Could not attach a standard effect";
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
void Scene::PrepareMaterials(
  o3d::Pack* pack, o3d_utils::ViewInfo* view_info, o3d::Pack* effect_pack) {
  std::vector<o3d::Material*> materials = pack->GetByClass<o3d::Material>();
  for (size_t ii = 0; ii < materials.size(); ++ii) {
    PrepareMaterial(effect_pack ? effect_pack : pack,
                    view_info,
                    materials[ii], "");
  }
}

/**
 * Adds missing texture coordinate streams to a primitive.
 *
 * This is very application specific but if it's a primitive
 * and if it uses a collada material the material builder
 * assumes 1 TEXCOORD stream per texture. In other words if you have
 * both a specular texture AND a diffuse texture the builder assumes
 * you have 2 TEXCOORD streams. This assumption is often false.
 *
 * To work around this we check how many streams the material
 * expects and if there are not enough UVs streams we duplicate the
 * last TEXCOORD stream until there are, making a BIG assumption that
 * that will work.
 *
 * The problem is maybe you have 4 textures and each of them share
 * texture coordinates. There is information in the collada file about
 * what stream to connect each texture to.
 *
 * @param {!o3d.Element} element Element to add streams to.
 */
void Scene::AddMissingTexCoordStreams(o3d::Element* element) {
  // TODO: We should store that info. The conditioner should either
  // make streams that way or pass on the info so we can do it here.
  if (element->IsA(o3d::Primitive::GetApparentClass())) {
    o3d::Primitive* primitive = down_cast<o3d::Primitive*>(element);
    o3d::Material* material = element->material();
    o3d::StreamBank* streamBank = primitive->stream_bank();
    std::string lightingType = ShaderBuilder::getColladaLightingType(material);
    if (!lightingType.empty()) {
      int numTexCoordStreamsNeeded =
          ShaderBuilder::getNumTexCoordStreamsNeeded(material);
      // Count the number of TEXCOORD streams the streamBank has.
      const o3d::StreamParamVector& streams =
            streamBank->vertex_stream_params();
      const o3d::Stream* lastTexCoordStream = NULL;
      int numTexCoordStreams = 0;
      for (int ii = 0; ii < static_cast<int>(streams.size()); ++ii) {
        const o3d::Stream* stream = &streams[ii]->stream();
        if (stream->semantic() == o3d::Stream::TEXCOORD) {
          lastTexCoordStream = stream;
          ++numTexCoordStreams;
        }
      }
      // Add any missing TEXCOORD streams. It might be more efficient for
      // the GPU to create an effect that doesn't need the extra streams
      // but this is a more generic solution because it means we can reuse
      // the same effect.
      for (int ii = numTexCoordStreams;
           ii < numTexCoordStreamsNeeded;
           ++ii) {
        streamBank->SetVertexStream(
            lastTexCoordStream->semantic(),
            lastTexCoordStream->semantic_index() + ii - numTexCoordStreams + 1,
            &lastTexCoordStream->field(),
            lastTexCoordStream->start_index());
      }
    }
  }
};

/**
 * Adds missing tex coord streams to a shape's elements.
 * @param {!o3d.Shape} shape Shape to add missing streams to.
 * @see o3djs.element.addMissingTexCoordStreams
 */
void Scene::AddMissingTexCoordStreams(o3d::Shape* shape) {
  const o3d::ElementRefArray& elements = shape->GetElementRefs();
  for (size_t ee = 0; ee < elements.size(); ++ee) {
    AddMissingTexCoordStreams(elements[ee]);
  }
};

/**
 * Sets the bounding box and z sort points of a shape's elements.
 * @param {!o3d.Shape} shape Shape to set info on.
 */
void Scene::SetBoundingBoxesAndZSortPoints(o3d::Shape* shape) {
  const o3d::ElementRefArray& elements = shape->GetElementRefs();
  for (size_t ee = 0; ee < elements.size(); ++ee) {
    Primitives::SetBoundingBoxAndZSortPoint(elements[ee]);
  }
};

/**
 * Prepares a shape by setting its boundingBox, zSortPoint and creating
 * DrawElements.
 * @param {!o3d.Pack} pack Pack to manage created objects.
 * @param {!o3d.Shape} shape Shape to prepare.
 */
void Scene::PrepareShape(o3d::Pack* pack, o3d::Shape* shape) {
  shape->CreateDrawElements(pack, NULL);
  SetBoundingBoxesAndZSortPoints(shape);
  AddMissingTexCoordStreams(shape);
};

/**
 * Prepares all the shapes in the given pack by setting their boundingBox,
 * zSortPoint and creating DrawElements.
 * @param {!o3d.Pack} pack Pack to manage created objects.
 */
void Scene::PrepareShapes(o3d::Pack* pack) {
  std::vector<o3d::Shape*> shapes = pack->GetByClass<o3d::Shape>();
  for (size_t ss = 0; ss < shapes.size(); ++ss) {
    PrepareShape(pack, shapes[ss]);
  }
};

Scene* Scene::LoadScene(
    o3d::Client* client,
    o3d_utils::ViewInfo* view_info,
    const std::string& filename,
    o3d::Pack* effect_texture_pack) {
  o3d::Pack* pack = client->CreatePack();
  o3d::Transform* root = pack->Create<o3d::Transform>();
  o3d::ParamFloat* time = root->CreateParam<o3d::ParamFloat>("time");

  o3d::Collada::Options options;
  options.up_axis = o3d::Vector3(0.0f, 1.0f, 0.0f);
  options.texture_pack = effect_texture_pack;
  options.store_textures_by_basename = effect_texture_pack != NULL;
  if (!o3d::Collada::Import(
      pack,
      filename,
      root,
      time,
      options)) {
    pack->Destroy();
    return NULL;
  }

  PrepareMaterials(pack, view_info, effect_texture_pack);
  PrepareShapes(pack);

  #if 1  // print total polygons.
  {
    int total = 0;
    std::vector<o3d::IndexBuffer*> bufs = pack->GetByClass<o3d::IndexBuffer>();
    for (size_t ii = 0; ii < bufs.size(); ++ii) {
      int num = bufs[ii]->num_elements() / 3;
      DLOG(INFO) << "IndexBuffer: " << bufs[ii]->name() << " : polys: " << num;
      total += num;
    }
    DLOG(INFO) << "Total Polygons: " << total;
  }
  #endif

  return new Scene(pack, root, time);
}

Scene::Scene(o3d::Pack* pack, o3d::Transform* root, o3d::ParamFloat* time)
    : pack_(pack),
      root_(root),
      time_(time) {
}

Scene::~Scene() {
  root_->SetParent(NULL);
  pack_->Destroy();
}

void Scene::SetParent(o3d::Transform* parent) {
  root_->SetParent(parent);
}

void Scene::SetLocalMatrix(const o3d::Matrix4& mat) {
  root_->set_local_matrix(mat);
}

void Scene::SetLocalMatrix(const float* mat) {
  root_->set_local_matrix(o3d::Matrix4(
      o3d::Vector4(mat[0], mat[1], mat[2], mat[3]),
      o3d::Vector4(mat[4], mat[5], mat[6], mat[7]),
      o3d::Vector4(mat[8], mat[9], mat[10], mat[11]),
      o3d::Vector4(mat[12], mat[13], mat[14], mat[15])));
}

void Scene::SetAnimationTime(float timeInSeconds) {
  time_->set_value(timeInSeconds);
}

class Cloner {
 public:
  static Scene* Clone(o3d::Client* client, const Scene* src);

 private:
  class CopierBase : public o3d::RefCounted {
   public:
    typedef o3d::SmartPointer<CopierBase> Ref;

    virtual ~CopierBase() { }

    virtual bool Copy(
        Cloner* cloner,
        const o3d::ObjectBase* src,
        o3d::ObjectBase* dst) = 0;
  };

  template <typename T>
  class Copier : public CopierBase {
   public:
    virtual bool Copy(
        Cloner* cloner,
        const o3d::ObjectBase* src,
        o3d::ObjectBase* dst) {
//DLOG(INFO) << "Checking if object is: " << T::GetApparentClass()->name();
      if (src->IsA(T::GetApparentClass()) && dst->IsA(T::GetApparentClass())) {
//DLOG(INFO) << "Copying as: " <<  T::GetApparentClass()->name();
        cloner->Copy(static_cast<const T*>(src), static_cast<T*>(dst));
        return true;
      }
      return false;
    }
  };

  Cloner(o3d::Client* client)
      : client_(client) {
  }

  Scene* CloneScene(const Scene* src);

  void CheckError() {
    const std::string& error = client_->GetLastError();
    if (!error.empty()) {
      DLOG(INFO) << "============O3D ERROR===================="
                 << error;
      client_->ClearLastError();
    }
  }

  static bool ShouldClone(o3d::ObjectBase* obj) {
    // Check by class.
    if (obj->IsA(o3d::Curve::GetApparentClass()) ||
        obj->IsA(o3d::Material::GetApparentClass()) ||
        obj->IsA(o3d::Effect::GetApparentClass()) ||
        obj->IsA(o3d::IndexBuffer::GetApparentClass()) ||
        obj->IsA(o3d::Skin::GetApparentClass()) ||
        obj->IsA(o3d::Texture::GetApparentClass())) {
      return false;
    }
    return true;
  };

  void Copy(const o3d::Element* src_element, o3d::Element* dst_element) {
//DLOG(INFO) << "Copy Element";
    Copy(static_cast<const o3d::ParamObject*>(src_element),
         static_cast<o3d::ParamObject*>(dst_element));

    o3d::Shape* owner = src_element->owner();
    dst_element->SetOwner(owner ? GetNew<o3d::Shape>(owner) : NULL);
  }

  void Copy(const o3d::DrawElement* src_de, o3d::DrawElement* dst_de) {
//DLOG(INFO) << "Copy DrawElement";
    Copy(static_cast<const o3d::ParamObject*>(src_de),
         static_cast<o3d::ParamObject*>(dst_de));

    o3d::Element* owner = src_de->owner();
    dst_de->SetOwner(owner ? GetNew<o3d::Element>(owner) : NULL);
  }

  void Copy(const o3d::NamedObject* src_object, o3d::NamedObject* dst_object) {
//DLOG(INFO) << "Copy NamedObject";
    Copy(static_cast<const o3d::ObjectBase*>(src_object),
         static_cast<o3d::ObjectBase*>(dst_object));

    dst_object->set_name(src_object->name() + "_copy");
  }

  void Copy(const o3d::ParamObject* src, o3d::ParamObject* dst) {
//DLOG(INFO) << "Copy ParamObject";
    Copy(static_cast<const o3d::NamedObject*>(src),
         static_cast<o3d::NamedObject*>(dst));
    const o3d::NamedParamRefMap& params = src->params();
    for (o3d::NamedParamRefMap::const_iterator it = params.begin();
         it != params.end();
         ++it) {
      o3d::Param* src_param = it->second;
      o3d::Param* dst_param = dst->GetUntypedParam(src_param->name());
      if (src_param->IsA(o3d::RefParamBase::GetApparentClass())) {
        o3d::RefParamBase* prb = down_cast<o3d::RefParamBase*>(src_param);
        o3d::ObjectBase* src_value = prb->value();
        if (src_value) {
          o3d::ObjectBase* dst_value = GetNew<o3d::ObjectBase>(src_value);
          if (dst_param->IsA(o3d::ParamDrawContext::GetApparentClass())) {
            //DLOG(INFO) << "Copying ParamDrawContext";
            down_cast<o3d::ParamDrawContext*>(dst_param)->set_value(
                down_cast<o3d::DrawContext*>(dst_value));
          } else
          if (dst_param->IsA(o3d::ParamDrawList::GetApparentClass())) {
            //DLOG(INFO) << "Copying ParamDrawList";
            down_cast<o3d::ParamDrawList*>(dst_param)->set_value(
                down_cast<o3d::DrawList*>(dst_value));
          } else
          if (dst_param->IsA(o3d::ParamEffect::GetApparentClass())) {
            //DLOG(INFO) << "Copying ParamEffect";
            down_cast<o3d::ParamEffect*>(dst_param)->set_value(
                down_cast<o3d::Effect*>(dst_value));
          } else
          if (dst_param->IsA(o3d::ParamFunction::GetApparentClass())) {
            //DLOG(INFO) << "Copying ParamFunction";
            down_cast<o3d::ParamFunction*>(dst_param)->set_value(
                down_cast<o3d::Function*>(dst_value));
          } else
          if (dst_param->IsA(o3d::ParamMaterial::GetApparentClass())) {
            //DLOG(INFO) << "Copying ParamMaterial";
            down_cast<o3d::ParamMaterial*>(dst_param)->set_value(
                down_cast<o3d::Material*>(dst_value));
          } else
          if (dst_param->IsA(o3d::ParamParamArray::GetApparentClass())) {
            //DLOG(INFO) << "Copying ParamParamArray";
            o3d::ParamArray* spa = down_cast<o3d::ParamArray*>(src_value);
            o3d::ParamArray* dpa = down_cast<o3d::ParamArray*>(dst_value);
            DCHECK(spa);
            DCHECK(dpa);
            //DLOG(INFO) << "Src ParamArray: "
            //    << src->name() << ":" << src->GetClass()->name()
            //    << src_param->name() << ":"
            //    << spa->name();
            //DLOG(INFO) << "Dst ParamArray: "
            //    << dst->name() << ":" << dst->GetClass()->name()
            //    << dst_param->name() << ":"
            //    << (dpa ? dpa->name().c_str() : "*NULL*");
            down_cast<o3d::ParamParamArray*>(dst_param)->set_value(
                down_cast<o3d::ParamArray*>(dst_value));
          } else
          if (dst_param->IsA(o3d::ParamRenderSurface::GetApparentClass())) {
            //DLOG(INFO) << "Copying ParamRenderSurface";
            down_cast<o3d::ParamRenderSurface*>(dst_param)->set_value(
                down_cast<o3d::RenderSurface*>(dst_value));
          } else
          if (dst_param->IsA(
              o3d::ParamRenderDepthStencilSurface::GetApparentClass())) {
            //DLOG(INFO) << "Copying ParamRenderDepthSampler";
            down_cast<o3d::ParamRenderDepthStencilSurface*>(
                dst_param)->set_value(
                    down_cast<o3d::RenderDepthStencilSurface*>(dst_value));
          } else
          if (dst_param->IsA(o3d::ParamSampler::GetApparentClass())) {
            //DLOG(INFO) << "Copying ParamSampler";
            down_cast<o3d::ParamSampler*>(dst_param)->set_value(
                down_cast<o3d::Sampler*>(dst_value));
          } else
          if (dst_param->IsA(o3d::ParamSkin::GetApparentClass())) {
            //DLOG(INFO) << "Copying ParamSkin";
            down_cast<o3d::ParamSkin*>(dst_param)->set_value(
                down_cast<o3d::Skin*>(dst_value));
          } else
          if (dst_param->IsA(o3d::ParamState::GetApparentClass())) {
            //DLOG(INFO) << "Copying ParamState";
            down_cast<o3d::ParamState*>(dst_param)->set_value(
                down_cast<o3d::State*>(dst_value));
          } else
          if (dst_param->IsA(o3d::ParamStreamBank::GetApparentClass())) {
            //DLOG(INFO) << "Copying ParamStreamBank";
            down_cast<o3d::ParamStreamBank*>(dst_param)->set_value(
                down_cast<o3d::StreamBank*>(dst_value));
          } else
          if (dst_param->IsA(o3d::ParamTexture::GetApparentClass())) {
            //DLOG(INFO) << "Copying ParamTexture";
            down_cast<o3d::ParamTexture*>(dst_param)->set_value(
                down_cast<o3d::Texture*>(dst_value));
          } else
          if (dst_param->IsA(o3d::ParamTransform::GetApparentClass())) {
            //DLOG(INFO) << "Copying ParamTransform";
            down_cast<o3d::ParamTransform*>(dst_param)->set_value(
                down_cast<o3d::Transform*>(dst_value));
          }
        }
      }
      if (dst_param && src_param->input_connection()) {
        dst_param->Bind(GetNew<o3d::Param>(src_param->input_connection()));
      }
    }
  }

  void Copy(const o3d::ObjectBase* src_object, o3d::ObjectBase* dst_object) {
//DLOG(INFO) << "Copy ObjectBase";
  }

  void Copy(
      const o3d::Primitive* src_primitive, o3d::Primitive* dst_primitive) {
//DLOG(INFO) << "Copy Primitive";
    Copy(static_cast<const o3d::Element*>(src_primitive),
         static_cast<o3d::Element*>(dst_primitive));

    dst_primitive->set_number_vertices(src_primitive->number_vertices());
    dst_primitive->set_number_primitives(src_primitive->number_primitives());
    dst_primitive->set_primitive_type(src_primitive->primitive_type());
    dst_primitive->set_start_index(src_primitive->start_index());
    dst_primitive->set_index_buffer(GetNew<o3d::IndexBuffer>(
        src_primitive->index_buffer()));
  }

  void Copy(const o3d::Shape* src_shape, o3d::Shape* dst_shape) {
//DLOG(INFO) << "Copy Shape";
    Copy(static_cast<const o3d::ParamObject*>(src_shape),
         static_cast<o3d::ParamObject*>(dst_shape));
  }

  void Copy(
      const o3d::Transform* src_transform, o3d::Transform* dst_transform) {
//DLOG(INFO) << "Copy Transform";
    Copy(static_cast<const o3d::ParamObject*>(src_transform),
         static_cast<o3d::ParamObject*>(dst_transform));

    const o3d::ShapeRefArray& src_shapes = src_transform->GetShapeRefs();
    for (size_t ii = 0; ii < src_shapes.size(); ++ii) {
      dst_transform->AddShape(GetNew<o3d::Shape>(src_shapes[ii]));
    }

    o3d::Transform* parent = src_transform->parent();
    dst_transform->SetParent(parent ? GetNew<o3d::Transform>(parent) : NULL);
  }

  void Copy(const o3d::SkinEval* src_skin_eval, o3d::SkinEval* dst_skin_eval) {
//DLOG(INFO) << "Copy SkinEval";
    Copy(static_cast<const o3d::VertexSource*>(src_skin_eval),
         static_cast<o3d::VertexSource*>(dst_skin_eval));

    const o3d::StreamParamVector& vertex_stream_params =
        src_skin_eval->vertex_stream_params();
    for (size_t ii = 0; ii != vertex_stream_params.size(); ++ii) {
      const o3d::Stream& stream = vertex_stream_params[ii]->stream();
      dst_skin_eval->SetVertexStream(
          stream.semantic(),
          stream.semantic_index(),
          GetNew<o3d::Field>(&stream.field()),
          stream.start_index());
      o3d::Param* input = vertex_stream_params[ii]->input_connection();
      if (input != NULL) {
        dst_skin_eval->BindStream(
            GetNew<o3d::VertexSource>(input->owner()),
            stream.semantic(),
            stream.semantic_index());
      }
    }
  }

  void Copy(const o3d::ParamArray* src_array, o3d::ParamArray* dst_array) {
//DLOG(INFO) << "Copy ParamArray";
    Copy(static_cast<const o3d::NamedObject*>(src_array),
         static_cast<o3d::NamedObject*>(dst_array));

    const o3d::ParamArray::ParamRefVector& src_params = src_array->params();
    const o3d::ParamArray::ParamRefVector& dst_params = dst_array->params();
    for (size_t ii = 0; ii < src_params.size(); ++ii) {
      o3d::Param* src_param = src_params[ii];
      o3d::Param* input = src_param->input_connection();
      if (input) {
        o3d::Param* dst_param = dst_params[ii];
        dst_param->Bind(GetNew<o3d::Param>(input));
      }
    }
  }

  void Copy(const o3d::StreamBank* src_stream_bank,
            o3d::StreamBank* dst_stream_bank) {
//DLOG(INFO) << "Copy StreamBank";
    Copy(static_cast<const o3d::ParamObject*>(src_stream_bank),
         static_cast<o3d::ParamObject*>(dst_stream_bank));
    const o3d::StreamParamVector& vertex_stream_params =
        src_stream_bank->vertex_stream_params();
    for (size_t ii = 0; ii != vertex_stream_params.size(); ++ii) {
      const o3d::Stream& stream = vertex_stream_params[ii]->stream();
      dst_stream_bank->SetVertexStream(
          stream.semantic(),
          stream.semantic_index(),
          GetNew<o3d::Field>(&stream.field()),
          stream.start_index());
      o3d::Param* input = vertex_stream_params[ii]->input_connection();
      if (input != NULL) {
        dst_stream_bank->BindStream(
            GetNew<o3d::VertexSource>(input->owner()),
            stream.semantic(),
            stream.semantic_index());
      }
    }
  }

  void AddMapping(
      const o3d::ObjectBase* old_obj, const o3d::ObjectBase* new_obj) {
    DCHECK(old_obj);
    DCHECK(new_obj);
    DCHECK(old_obj->IsA(o3d::ObjectBase::GetApparentClass()));
    DCHECK(new_obj->IsA(o3d::ObjectBase::GetApparentClass()));
    old_to_new_map_[old_obj] = const_cast<o3d::ObjectBase*>(new_obj);
  }

  template <typename T>
  T* GetNew(const o3d::ObjectBase* old) {
    DCHECK(old) << " was null for type: " << T::GetApparentClass()->name();
    T* value = down_cast<T*>(old_to_new_map_[old]);
    DCHECK(value == NULL || value->IsA(old->GetClass()));
    return value;
  }

  o3d::Client* client_;
  std::map<const o3d::ObjectBase*, o3d::ObjectBase*> old_to_new_map_;
};

Scene* Cloner::Clone(o3d::Client* client, const Scene* src) {
  Cloner cloner(client);
  return cloner.CloneScene(src);
}

Scene* Cloner::CloneScene(const Scene* src) {
  o3d::Pack* dst_pack = client_->CreatePack();
  o3d::Pack* src_pack = src->pack();

//DLOG(INFO) << "Walk Stream Banks";
  // First, hand process the stream_banks to find which vertex buffers need to
  // be cloned and which don't
  std::set<o3d::Buffer*> buffers_to_clone;
  std::set<o3d::StreamBank*> stream_banks_to_clone;
  std::vector<o3d::StreamBank*> stream_banks =
      src_pack->GetByClass<o3d::StreamBank>();
  for (size_t ii = 0; ii < stream_banks.size(); ++ii) {
    o3d::StreamBank* sb = down_cast<o3d::StreamBank*>(stream_banks[ii]);
    const o3d::StreamParamVector& params = sb->vertex_stream_params();
    bool need_to_clone = false;
    for (size_t jj = 0; jj < params.size(); ++jj) {
      const o3d::ParamVertexBufferStream* param = params[jj];
      // If there is an input connection then we need to clone it.
      if (param->input_connection()) {
        const o3d::Buffer* buffer = param->stream().field().buffer();
        buffers_to_clone.insert(const_cast<o3d::Buffer*>(buffer));
        need_to_clone = true;
      }
    }
    if (!need_to_clone) {
      AddMapping(sb, sb);
    }
  }

//DLOG(INFO) << "Compute buffers to keep";
  // Clone stuff not on the list.
  std::vector<o3d::Buffer*> buffers =
      src_pack->GetByClass<o3d::Buffer>();
  std::set<o3d::Buffer*> all_buffers(buffers.begin(), buffers.end());
  std::set<o3d::Buffer*> buffers_to_keep;

  std::set_difference(
      all_buffers.begin(),
      all_buffers.end(),
      buffers_to_clone.begin(),
      buffers_to_clone.end(),
      std::inserter(buffers_to_keep, buffers_to_keep.begin()));

  for (std::set<o3d::Buffer*>::iterator it = buffers_to_keep.begin();
       it != buffers_to_keep.end(); ++it) {
    o3d::Buffer* buffer = *it;
    AddMapping(buffer, buffer);
    // The the fields
    const o3d::FieldRefArray& fields = buffer->fields();
    for (size_t jj = 0; jj < fields.size(); ++jj) {
      const o3d::Field* field = fields[jj];
      AddMapping(field, field);
    }
  }

//DLOG(INFO) << "Walk Primitives";
  // Next we need to look at all primitives and see if they use a streambank
  // that needs to be cloned.
  std::vector<o3d::Primitive*> primitives =
      src_pack->GetByClass<o3d::Primitive>();
  for (size_t ii = 0; ii < primitives.size(); ++ii) {
    o3d::Primitive* p = down_cast<o3d::Primitive*>(primitives[ii]);
    if (GetNew<o3d::StreamBank>(p->stream_bank()) != NULL) {
      // This primitive does NOT need to be cloned.
      AddMapping(p, p);
      // And neither do it's DrawElements
      const o3d::DrawElementRefArray& draw_elements = p->GetDrawElementRefs();
      for (size_t jj = 0; jj < draw_elements.size(); ++jj) {
        o3d::DrawElement* de = draw_elements[jj];
        AddMapping(de, de);
      }
    }
  }

//DLOG(INFO) << "Walk Shapes";
  // Finally we need to go through the shapes and see if they use a primitive
  // that needs to be cloned.
  std::vector<o3d::Shape*> shapes = src_pack->GetByClass<o3d::Shape>();
  for (size_t ii = 0; ii < shapes.size(); ++ii) {
    o3d::Shape* s = down_cast<o3d::Shape*>(shapes[ii]);
    const o3d::ElementRefArray& elements = s->GetElementRefs();
    bool need_to_clone = false;
    for (size_t jj = 0; jj < elements.size(); ++jj) {
      if (GetNew<o3d::Element>(elements[jj]) == NULL) {
        need_to_clone = true;
        break;
      }
    }
    if (!need_to_clone) {
      // This Shape does NOT need to be cloned.
      AddMapping(s, s);
    }
  }

//DLOG(INFO) << "Clone Stuff";
  CheckError();
  // Clone all the rest.
  std::vector<o3d::ObjectBase*> objects =
      src_pack->GetByClass<o3d::ObjectBase>();
  for (size_t ii = 0; ii < objects.size(); ++ii) {
    o3d::ObjectBase* src = objects[ii];
    o3d::ObjectBase* dst = src;
    if (GetNew<o3d::ObjectBase>(src) == NULL && ShouldClone(src)) {
      dst = dst_pack->CreateObjectByClass(src->GetClass());
      if (dst->IsA(o3d::ParamObject::GetApparentClass())) {
        down_cast<o3d::ParamObject*>(dst)->CopyParams(
            down_cast<o3d::ParamObject*>(src));
      }
      if (dst->IsA(o3d::Buffer::GetApparentClass())) {
        // Duplicate the fields.
        o3d::Buffer* src_buf = down_cast<o3d::Buffer*>(src);
        o3d::Buffer* dst_buf = down_cast<o3d::Buffer*>(dst);
//DLOG(INFO) << "Create Fields";
        const o3d::FieldRefArray& fields = src_buf->fields();
        for (size_t jj = 0; jj < fields.size(); ++jj) {
          const o3d::Field* src_field = fields[jj];
          o3d::Field* dst_field = dst_buf->CreateField(
              src_field->GetClass(), src_field->num_components());
          AddMapping(src_field, dst_field);
        }
        dst_buf->AllocateElements(src_buf->num_elements());
//DLOG(INFO) << "Copy Fields";
        for (size_t jj = 0; jj < fields.size(); ++jj) {
          const o3d::Field* src_field = fields[jj];
          o3d::Field* dst_field = GetNew<o3d::Field>(src_field);
          dst_field->Copy(*src_field);
CheckError();
          AddMapping(src_field, dst_field);
        }
      }
      if (dst->IsA(o3d::ParamArray::GetApparentClass())) {
        // Duplicate the params
        o3d::ParamArray* src_array = down_cast<o3d::ParamArray*>(src);
        o3d::ParamArray* dst_array = down_cast<o3d::ParamArray*>(dst);
        const o3d::ParamArray::ParamRefVector& src_params = src_array->params();
        for (size_t jj = 0; jj < src_params.size(); ++jj) {
          o3d::Param* src_param = src_params[jj];
          o3d::Param* dst_param =
              dst_array->CreateParamByClass(jj, src_param->GetClass());
          AddMapping(src_param, dst_param);
        }
      }
    }
    AddMapping(src, dst);

    // Map the params.
    if (dst->IsA(o3d::ParamObject::GetApparentClass())) {
      o3d::ParamObject* src_obj = down_cast<o3d::ParamObject*>(src);
      o3d::ParamObject* dst_obj = down_cast<o3d::ParamObject*>(dst);
      const o3d::NamedParamRefMap& params = src_obj->params();
      for (o3d::NamedParamRefMap::const_iterator it = params.begin();
           it != params.end(); ++it) {
        const o3d::Param* src_param = it->second;
        o3d::Param* dst_param = dst_obj->GetUntypedParam(src_param->name());
        AddMapping(src_param, dst_param);
      }
    }
  }

//DLOG(INFO) << "Setup Copier";
  std::vector<CopierBase::Ref> copiers;
  // TODO(gman): Init this just once.
  // NOTE: The order here is important. Derived types must
  //    come before Base types.
  copiers.push_back(CopierBase::Ref(new Copier<o3d::SkinEval>));
  copiers.push_back(CopierBase::Ref(new Copier<o3d::StreamBank>));
  copiers.push_back(CopierBase::Ref(new Copier<o3d::Primitive>));
  copiers.push_back(CopierBase::Ref(new Copier<o3d::Element>));
  copiers.push_back(CopierBase::Ref(new Copier<o3d::DrawElement>));
  copiers.push_back(CopierBase::Ref(new Copier<o3d::Transform>));
  copiers.push_back(CopierBase::Ref(new Copier<o3d::Shape>));
  copiers.push_back(CopierBase::Ref(new Copier<o3d::ParamArray>));
  copiers.push_back(CopierBase::Ref(new Copier<o3d::ParamObject>));
  copiers.push_back(CopierBase::Ref(new Copier<o3d::NamedObject>));
  copiers.push_back(CopierBase::Ref(new Copier<o3d::ObjectBase>));

  // Now copy/bind everything.
//DLOG(INFO) << "Copy Stuff";
  for (size_t ii = 0; ii < objects.size(); ++ii) {
    o3d::ObjectBase* src = objects[ii];
    o3d::ObjectBase* dst = GetNew<o3d::ObjectBase>(src);
    if (dst && src != dst) {
//      DLOG(INFO) << "Copying a: " << src->GetClass()->name();
//      DLOG(INFO) << "To a : " << dst->GetClass()->name();
//      if (src->IsA(o3d::NamedObject::GetApparentClass())) {
//        DLOG(INFO) << "Copying: "
//                   << down_cast<o3d::NamedObject*>(src)->name();
//      }
      for (size_t jj = 0; jj < copiers.size(); ++jj) {
        if (copiers[jj]->Copy(this, src, dst)) {
          break;
        }
      }
CheckError();
    }
  }

//DLOG(INFO) << "Done Copying";

  return new Scene(
      dst_pack,
      GetNew<o3d::Transform>(src->root()),
      GetNew<o3d::ParamFloat>(src->time_param()));
}

Scene* Scene::Clone(o3d::Client* client) const {
  return Cloner::Clone(client, this);
}

}  // namespace o3d_utils

