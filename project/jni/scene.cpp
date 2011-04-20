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

#include <map>
#include <set>
#include <string>
#include <vector>
#include <fstream>
#include "scene.h"
#include "core/cross/client.h"
#include "core/cross/curve.h"
#include "core/cross/pack.h"
#include "core/cross/param_array.h"
#include "core/cross/primitive.h"
#include "core/cross/renderer.h"
#include "core/cross/sampler.h"
#include "core/cross/shape.h"
#include "core/cross/skin.h"
#include "core/cross/transform.h"
#include "core/cross/types.h"
#include "import/cross/collada.h"
#include "materials.h"
#include "primitives.h"
#include "render_graph.h"
#include "shader_builder.h"

namespace o3d_utils {
using namespace o3d;

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
void Scene::AddMissingTexCoordStreams(Element* element) {
  // TODO: We should store that info. The conditioner should either
  // make streams that way or pass on the info so we can do it here.
  if (element->IsA(Primitive::GetApparentClass())) {
    Primitive* primitive = down_cast<Primitive*>(element);
    Material* material = element->material();
    StreamBank* streamBank = primitive->stream_bank();
    std::string lightingType = ShaderBuilder::getColladaLightingType(material);
    if (!lightingType.empty()) {
      int numTexCoordStreamsNeeded =
          ShaderBuilder::getNumTexCoordStreamsNeeded(material);
      // Count the number of TEXCOORD streams the streamBank has.
      const StreamParamVector& streams =
            streamBank->vertex_stream_params();
      const Stream* lastTexCoordStream = NULL;
      int numTexCoordStreams = 0;
      for (int ii = 0; ii < static_cast<int>(streams.size()); ++ii) {
        const Stream* stream = &streams[ii]->stream();
        if (stream->semantic() == Stream::TEXCOORD) {
          lastTexCoordStream = stream;
          ++numTexCoordStreams;
        }
      }
      // Add any missing TEXCOORD streams. It might be more efficient for
      // the GPU to create an effect that doesn't need the extra streams
      // but this is a more generic solution because it means we can reuse
      // the same effect.
      if (lastTexCoordStream) {
        for (int ii = numTexCoordStreams; ii < numTexCoordStreamsNeeded; ++ii) {
          streamBank->SetVertexStream(
            lastTexCoordStream->semantic(),
            lastTexCoordStream->semantic_index() + ii - numTexCoordStreams + 1,
            &lastTexCoordStream->field(),
            lastTexCoordStream->start_index());
        }
      }
    }
  }
};

/**
 * Adds missing tex coord streams to a shape's elements.
 * @param {!o3d.Shape} shape Shape to add missing streams to.
 * @see o3djs.element.addMissingTexCoordStreams
 */
void Scene::AddMissingTexCoordStreams(Shape* shape) {
  const ElementRefArray& elements = shape->GetElementRefs();
  for (size_t ee = 0; ee < elements.size(); ++ee) {
    AddMissingTexCoordStreams(elements[ee]);
  }
};

/**
 * Sets the bounding box and z sort points of a shape's elements.
 * @param {!o3d.Shape} shape Shape to set info on.
 */
void Scene::SetBoundingBoxesAndZSortPoints(Shape* shape) {
  const ElementRefArray& elements = shape->GetElementRefs();
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
void Scene::PrepareShape(Pack* pack, Shape* shape) {
  shape->CreateDrawElements(pack, NULL);
  SetBoundingBoxesAndZSortPoints(shape);
  AddMissingTexCoordStreams(shape);
};

/**
 * Prepares all the shapes in the given pack by setting their boundingBox,
 * zSortPoint and creating DrawElements.
 * @param {!o3d.Pack} pack Pack to manage created objects.
 */
void Scene::PrepareShapes(Pack* pack) {
  std::vector<Shape*> shapes = pack->GetByClass<Shape>();
  for (size_t ss = 0; ss < shapes.size(); ++ss) {
    PrepareShape(pack, shapes[ss]);
  }
};

Scene* Scene::DummyScene(
  Client* client,
  ViewInfo* view_info)
{
  Pack* pack = client->CreatePack();
  pack->set_root(pack->Create<Transform>());
  ParamFloat* time = pack->root()->CreateParam<ParamFloat>("time");

  Shape* shape = pack->Create<Shape>();
  pack->root()->AddShape(shape);

  Primitive* prim = Primitives::CreateSphere(
      pack, 10.0f, 8, 5, NULL);
  // Setup the material with collada parameters so the shader builder will
  // make a shader for us.
  // NOTE: This is NOT the typical way to do this. This is only because
  //    we have a collada specific shader builder in shader_builder.cpp
  //    which is NOT part of O3D.
  Material* material = pack->Create<Material>();
  Sampler* sampler = pack->Create<Sampler>();
  Renderer* renderer =
      pack->service_locator()->GetService<Renderer>();
  sampler->set_texture(renderer->error_texture());
  sampler->set_min_filter(Sampler::NONE);
  sampler->set_mag_filter(Sampler::NONE);
  sampler->set_mip_filter(Sampler::NONE);
  material->CreateParam<ParamString>("collada.lightingType")->set_value(
      "constant");
  material->CreateParam<ParamSampler>("emissiveSampler")->set_value(
      sampler);
  prim->SetOwner(shape);
  prim->set_material(material);
  Materials::PrepareMaterials(pack, view_info, 0);
  PrepareShapes(pack);

  return new Scene(pack, pack->root(), time);
}

Scene* Scene::LoadScene(
    Client* client,
    ViewInfo* view_info,
    const std::string& filename,
    Pack* effect_texture_pack) {
  Pack* pack = client->CreatePack();
  pack->set_root(pack->Create<Transform>());
  ParamFloat* time = pack->root()->CreateParam<ParamFloat>("time");

  Collada::Options options;
  options.up_axis = Vector3(0.0f, 1.0f, 0.0f);
  options.texture_pack = effect_texture_pack;
  options.store_textures_by_basename = effect_texture_pack != NULL;
  if (!Collada::Import(
      pack,
      filename,
      pack->root(),
      time,
      options)) {
    pack->service_locator()->GetService<ObjectManager>()->DestroyPack(pack);
    return 0;
  }

  Materials::PrepareMaterials(pack, view_info, effect_texture_pack);
  PrepareShapes(pack);

  return new Scene(pack, pack->root(), time);
}

Scene* Scene::LoadBinaryScene(
  Client* client,
  ViewInfo* view_info,
  const std::string& filename,
  extra::IExternalResourceProvider& external_resource_provider) {

  Transform* root = 0;
  Pack* pack = client->CreatePack();

  std::ifstream ifs(filename.c_str(), std::ios::in|std::ios::binary);
  if (ifs.is_open()) {
    root = extra::LoadFromBinaryStream(ifs, *pack, external_resource_provider);
    ifs.close();
  }
  else {
    O3D_LOG(ERROR) << "Can't open " << filename;
  }
  if (!root) {
    pack->service_locator()->GetService<ObjectManager>()->DestroyPack(pack);
    return 0;
  }

  pack->set_root(root);
  Materials::PrepareMaterials(pack, view_info, 0);
  PrepareShapes(pack);

  #if 1  // print total polygons.
  {
    int total = 0;
    std::vector<IndexBuffer*> bufs = pack->GetByClass<IndexBuffer>();
    for (size_t ii = 0; ii < bufs.size(); ++ii) {
      int num = bufs[ii]->num_elements() / 3;
      O3D_LOG(INFO) << "IndexBuffer: " << bufs[ii]->name() << " : polys: " << num;
      total += num;
    }
    O3D_LOG(INFO) << "Total Polygons: " << total;
  }
  #endif

  return new Scene(pack, pack->root(), pack->root()->GetParam<ParamFloat>("time"));
}


Scene::Scene(Pack* pack, Transform* root, ParamFloat* time)
    : pack_(pack),
      root_(root),
      time_(time) {
}

Scene::~Scene() {
  root_->SetParent(NULL);
  pack_->Destroy();
}

void Scene::SetParent(Transform* parent) {
  root_->SetParent(parent);
}

void Scene::SetLocalMatrix(const Matrix4& mat) {
  root_->set_local_matrix(mat);
}

void Scene::SetLocalMatrix(const float* mat) {
  root_->set_local_matrix(Matrix4(
      Vector4(mat[0], mat[1], mat[2], mat[3]),
      Vector4(mat[4], mat[5], mat[6], mat[7]),
      Vector4(mat[8], mat[9], mat[10], mat[11]),
      Vector4(mat[12], mat[13], mat[14], mat[15])));
}

void Scene::SetAnimationTime(float timeInSeconds) {
  time_->set_value(timeInSeconds);
}

class Cloner {
 public:
  static Scene* Clone(Client* client, const Scene* src);

 private:
  class CopierBase : public RefCounted {
   public:
    typedef SmartPointer<CopierBase> Ref;

    virtual ~CopierBase() { }

    virtual bool Copy(
        Cloner* cloner,
        const ObjectBase* src,
        ObjectBase* dst) = 0;
  };

  template <typename T>
  class Copier : public CopierBase {
   public:
    virtual bool Copy(
        Cloner* cloner,
        const ObjectBase* src,
        ObjectBase* dst) {
      if (src->IsA(T::GetApparentClass()) && dst->IsA(T::GetApparentClass())) {
        cloner->Copy(static_cast<const T*>(src), static_cast<T*>(dst));
        return true;
      }
      return false;
    }
  };

  Cloner(Client* client)
      : client_(client) {
  }

  Scene* CloneScene(const Scene* src);

  void CheckError() {
    const std::string& error = client_->GetLastError();
    if (!error.empty()) {
      O3D_LOG(INFO) << "============O3D ERROR===================="
                 << error;
      client_->ClearLastError();
    }
  }

  static bool ShouldClone(ObjectBase* obj) {
    // Check by class.
    if (obj->IsA(Curve::GetApparentClass()) ||
        obj->IsA(Material::GetApparentClass()) ||
        obj->IsA(Effect::GetApparentClass()) ||
        obj->IsA(IndexBuffer::GetApparentClass()) ||
        obj->IsA(Skin::GetApparentClass()) ||
        obj->IsA(Texture::GetApparentClass())) {
      return false;
    }
    return true;
  };

  void Copy(const Element* src_element, Element* dst_element) {
    Copy(static_cast<const ParamObject*>(src_element),
         static_cast<ParamObject*>(dst_element));

    Shape* owner = src_element->owner();
    dst_element->SetOwner(owner ? GetNew<Shape>(owner) : NULL);
  }

  void Copy(const DrawElement* src_de, DrawElement* dst_de) {
    Copy(static_cast<const ParamObject*>(src_de),
         static_cast<ParamObject*>(dst_de));

    Element* owner = src_de->owner();
    dst_de->SetOwner(owner ? GetNew<Element>(owner) : NULL);
  }

  void Copy(const NamedObject* src_object, NamedObject* dst_object) {
    Copy(static_cast<const ObjectBase*>(src_object),
         static_cast<ObjectBase*>(dst_object));

    dst_object->set_name(src_object->name() + "_copy");
  }

  void Copy(const ParamObject* src, ParamObject* dst) {
    Copy(static_cast<const NamedObject*>(src),
         static_cast<NamedObject*>(dst));
    const NamedParamRefMap& params = src->params();
    for (NamedParamRefMap::const_iterator it = params.begin();
         it != params.end();
         ++it) {
      Param* src_param = it->second;
      Param* dst_param = dst->GetUntypedParam(src_param->name());
      if (src_param->IsA(RefParamBase::GetApparentClass())) {
        RefParamBase* prb = down_cast<RefParamBase*>(src_param);
        ObjectBase* src_value = prb->value();
        if (src_value) {
          ObjectBase* dst_value = GetNew<ObjectBase>(src_value);
          if (dst_param->IsA(ParamDrawContext::GetApparentClass())) {
            down_cast<ParamDrawContext*>(dst_param)->set_value(
                down_cast<DrawContext*>(dst_value));
          } else
          if (dst_param->IsA(ParamDrawList::GetApparentClass())) {
            down_cast<ParamDrawList*>(dst_param)->set_value(
                down_cast<DrawList*>(dst_value));
          } else
          if (dst_param->IsA(ParamEffect::GetApparentClass())) {
            down_cast<ParamEffect*>(dst_param)->set_value(
                down_cast<Effect*>(dst_value));
          } else
          if (dst_param->IsA(ParamFunction::GetApparentClass())) {
            down_cast<ParamFunction*>(dst_param)->set_value(
                down_cast<Function*>(dst_value));
          } else
          if (dst_param->IsA(ParamMaterial::GetApparentClass())) {
            down_cast<ParamMaterial*>(dst_param)->set_value(
                down_cast<Material*>(dst_value));
          } else
          if (dst_param->IsA(ParamParamArray::GetApparentClass())) {
            down_cast<ParamParamArray*>(dst_param)->set_value(
                down_cast<ParamArray*>(dst_value));
          } else
          if (dst_param->IsA(ParamRenderSurface::GetApparentClass())) {
            down_cast<ParamRenderSurface*>(dst_param)->set_value(
                down_cast<RenderSurface*>(dst_value));
          } else
          if (dst_param->IsA(
              ParamRenderDepthStencilSurface::GetApparentClass())) {
            down_cast<ParamRenderDepthStencilSurface*>(
                dst_param)->set_value(
                    down_cast<RenderDepthStencilSurface*>(dst_value));
          } else
          if (dst_param->IsA(ParamSampler::GetApparentClass())) {
            down_cast<ParamSampler*>(dst_param)->set_value(
                down_cast<Sampler*>(dst_value));
          } else
          if (dst_param->IsA(ParamSkin::GetApparentClass())) {
            down_cast<ParamSkin*>(dst_param)->set_value(
                down_cast<Skin*>(dst_value));
          } else
          if (dst_param->IsA(ParamState::GetApparentClass())) {
            down_cast<ParamState*>(dst_param)->set_value(
                down_cast<State*>(dst_value));
          } else
          if (dst_param->IsA(ParamStreamBank::GetApparentClass())) {
            down_cast<ParamStreamBank*>(dst_param)->set_value(
                down_cast<StreamBank*>(dst_value));
          } else
          if (dst_param->IsA(ParamTexture::GetApparentClass())) {
            down_cast<ParamTexture*>(dst_param)->set_value(
                down_cast<Texture*>(dst_value));
          } else
          if (dst_param->IsA(ParamTransform::GetApparentClass())) {
            down_cast<ParamTransform*>(dst_param)->set_value(
                down_cast<Transform*>(dst_value));
          }
        }
      }
      if (dst_param && src_param->input_connection()) {
        dst_param->Bind(GetNew<Param>(src_param->input_connection()));
      }
    }
  }

  void Copy(const ObjectBase* src_object, ObjectBase* dst_object) {
    // NO-OP
  }

  void Copy(
      const Primitive* src_primitive, Primitive* dst_primitive) {
    Copy(static_cast<const Element*>(src_primitive),
         static_cast<Element*>(dst_primitive));

    dst_primitive->set_number_vertices(src_primitive->number_vertices());
    dst_primitive->set_number_primitives(src_primitive->number_primitives());
    dst_primitive->set_primitive_type(src_primitive->primitive_type());
    dst_primitive->set_start_index(src_primitive->start_index());
    dst_primitive->set_index_buffer(GetNew<IndexBuffer>(
        src_primitive->index_buffer()));
  }

  void Copy(const Shape* src_shape, Shape* dst_shape) {
    Copy(static_cast<const ParamObject*>(src_shape),
         static_cast<ParamObject*>(dst_shape));
  }

  void Copy(
      const Transform* src_transform, Transform* dst_transform) {
    Copy(static_cast<const ParamObject*>(src_transform),
         static_cast<ParamObject*>(dst_transform));

    const ShapeRefArray& src_shapes = src_transform->GetShapeRefs();
    for (size_t ii = 0; ii < src_shapes.size(); ++ii) {
      dst_transform->AddShape(GetNew<Shape>(src_shapes[ii]));
    }

    Transform* parent = src_transform->parent();
    dst_transform->SetParent(parent ? GetNew<Transform>(parent) : NULL);
  }

  void Copy(const SkinEval* src_skin_eval, SkinEval* dst_skin_eval) {
    Copy(static_cast<const VertexSource*>(src_skin_eval),
         static_cast<VertexSource*>(dst_skin_eval));

    const StreamParamVector& vertex_stream_params =
        src_skin_eval->vertex_stream_params();
    for (size_t ii = 0; ii != vertex_stream_params.size(); ++ii) {
      const Stream& stream = vertex_stream_params[ii]->stream();
      dst_skin_eval->SetVertexStream(
          stream.semantic(),
          stream.semantic_index(),
          GetNew<Field>(&stream.field()),
          stream.start_index());
      Param* input = vertex_stream_params[ii]->input_connection();
      if (input != NULL) {
        dst_skin_eval->BindStream(
            GetNew<VertexSource>(input->owner()),
            stream.semantic(),
            stream.semantic_index());
      }
    }
  }

  void Copy(const ParamArray* src_array, ParamArray* dst_array) {
    Copy(static_cast<const NamedObject*>(src_array),
         static_cast<NamedObject*>(dst_array));

    const ParamArray::ParamRefVector& src_params = src_array->params();
    const ParamArray::ParamRefVector& dst_params = dst_array->params();
    for (size_t ii = 0; ii < src_params.size(); ++ii) {
      Param* src_param = src_params[ii];
      Param* input = src_param->input_connection();
      if (input) {
        Param* dst_param = dst_params[ii];
        dst_param->Bind(GetNew<Param>(input));
      }
    }
  }

  void Copy(const StreamBank* src_stream_bank,
            StreamBank* dst_stream_bank) {
    Copy(static_cast<const ParamObject*>(src_stream_bank),
         static_cast<ParamObject*>(dst_stream_bank));
    const StreamParamVector& vertex_stream_params =
        src_stream_bank->vertex_stream_params();
    for (size_t ii = 0; ii != vertex_stream_params.size(); ++ii) {
      const Stream& stream = vertex_stream_params[ii]->stream();
      dst_stream_bank->SetVertexStream(
          stream.semantic(),
          stream.semantic_index(),
          GetNew<Field>(&stream.field()),
          stream.start_index());
      Param* input = vertex_stream_params[ii]->input_connection();
      if (input != NULL) {
        dst_stream_bank->BindStream(
            GetNew<VertexSource>(input->owner()),
            stream.semantic(),
            stream.semantic_index());
      }
    }
  }

  void AddMapping(
      const ObjectBase* old_obj, const ObjectBase* new_obj) {
    O3D_ASSERT(old_obj);
    O3D_ASSERT(new_obj);
    O3D_ASSERT(old_obj->IsA(ObjectBase::GetApparentClass()));
    O3D_ASSERT(new_obj->IsA(ObjectBase::GetApparentClass()));
    old_to_new_map_[old_obj] = const_cast<ObjectBase*>(new_obj);
  }

  template <typename T>
  T* GetNew(const ObjectBase* old) {
    O3D_ASSERT(old) << " was null for type: " << T::GetApparentClass()->name();
    T* value = down_cast<T*>(old_to_new_map_[old]);
    O3D_ASSERT(value == NULL || value->IsA(old->GetClass()));
    return value;
  }

  Client* client_;
  std::map<const ObjectBase*, ObjectBase*> old_to_new_map_;
};

Scene* Cloner::Clone(Client* client, const Scene* src) {
  Cloner cloner(client);
  return cloner.CloneScene(src);
}

Scene* Cloner::CloneScene(const Scene* src) {
  Pack* dst_pack = client_->CreatePack();
  Pack* src_pack = src->pack();

  // First, hand process the stream_banks to find which vertex buffers need to
  // be cloned and which don't
  std::set<Buffer*> buffers_to_clone;
  std::set<StreamBank*> stream_banks_to_clone;
  std::vector<StreamBank*> stream_banks =
      src_pack->GetByClass<StreamBank>();
  for (size_t ii = 0; ii < stream_banks.size(); ++ii) {
    StreamBank* sb = down_cast<StreamBank*>(stream_banks[ii]);
    const StreamParamVector& params = sb->vertex_stream_params();
    bool need_to_clone = false;
    for (size_t jj = 0; jj < params.size(); ++jj) {
      const ParamVertexBufferStream* param = params[jj];
      // If there is an input connection then we need to clone it.
      if (param->input_connection()) {
        const Buffer* buffer = param->stream().field().buffer();
        buffers_to_clone.insert(const_cast<Buffer*>(buffer));
        need_to_clone = true;
      }
    }
    if (!need_to_clone) {
      AddMapping(sb, sb);
    }
  }

  // Clone stuff not on the list.
  std::vector<Buffer*> buffers =
      src_pack->GetByClass<Buffer>();
  std::set<Buffer*> all_buffers(buffers.begin(), buffers.end());
  std::set<Buffer*> buffers_to_keep;

  std::set_difference(
      all_buffers.begin(),
      all_buffers.end(),
      buffers_to_clone.begin(),
      buffers_to_clone.end(),
      std::inserter(buffers_to_keep, buffers_to_keep.begin()));

  for (std::set<Buffer*>::iterator it = buffers_to_keep.begin();
       it != buffers_to_keep.end(); ++it) {
    Buffer* buffer = *it;
    AddMapping(buffer, buffer);
    // The the fields
    const FieldRefArray& fields = buffer->fields();
    for (size_t jj = 0; jj < fields.size(); ++jj) {
      const Field* field = fields[jj];
      AddMapping(field, field);
    }
  }

  // Next we need to look at all primitives and see if they use a streambank
  // that needs to be cloned.
  std::vector<Primitive*> primitives =
      src_pack->GetByClass<Primitive>();
  for (size_t ii = 0; ii < primitives.size(); ++ii) {
    Primitive* p = down_cast<Primitive*>(primitives[ii]);
    if (GetNew<StreamBank>(p->stream_bank()) != NULL) {
      // This primitive does NOT need to be cloned.
      AddMapping(p, p);
      // And neither do it's DrawElements
      const DrawElementRefArray& draw_elements = p->GetDrawElementRefs();
      for (size_t jj = 0; jj < draw_elements.size(); ++jj) {
        DrawElement* de = draw_elements[jj];
        AddMapping(de, de);
      }
    }
  }

  // Finally we need to go through the shapes and see if they use a primitive
  // that needs to be cloned.
  std::vector<Shape*> shapes = src_pack->GetByClass<Shape>();
  for (size_t ii = 0; ii < shapes.size(); ++ii) {
    Shape* s = down_cast<Shape*>(shapes[ii]);
    const ElementRefArray& elements = s->GetElementRefs();
    bool need_to_clone = false;
    for (size_t jj = 0; jj < elements.size(); ++jj) {
      if (GetNew<Element>(elements[jj]) == NULL) {
        need_to_clone = true;
        break;
      }
    }
    if (!need_to_clone) {
      // This Shape does NOT need to be cloned.
      AddMapping(s, s);
    }
  }

  CheckError();
  // Clone all the rest.
  std::vector<ObjectBase*> objects =
      src_pack->GetByClass<ObjectBase>();
  for (size_t ii = 0; ii < objects.size(); ++ii) {
    ObjectBase* src = objects[ii];
    ObjectBase* dst = src;
    if (GetNew<ObjectBase>(src) == NULL && ShouldClone(src)) {
      dst = dst_pack->CreateObjectByClass(src->GetClass());
      if (dst->IsA(ParamObject::GetApparentClass())) {
        down_cast<ParamObject*>(dst)->CopyParams(
            down_cast<ParamObject*>(src));
      }
      if (dst->IsA(Buffer::GetApparentClass())) {
        // Duplicate the fields.
        Buffer* src_buf = down_cast<Buffer*>(src);
        Buffer* dst_buf = down_cast<Buffer*>(dst);
        const FieldRefArray& fields = src_buf->fields();
        for (size_t jj = 0; jj < fields.size(); ++jj) {
          const Field* src_field = fields[jj];
          Field* dst_field = dst_buf->CreateField(
              src_field->GetClass(), src_field->num_components());
          AddMapping(src_field, dst_field);
        }
        dst_buf->AllocateElements(src_buf->num_elements());
        for (size_t jj = 0; jj < fields.size(); ++jj) {
          const Field* src_field = fields[jj];
          Field* dst_field = GetNew<Field>(src_field);
          dst_field->Copy(*src_field);
CheckError();
          AddMapping(src_field, dst_field);
        }
      }
      if (dst->IsA(ParamArray::GetApparentClass())) {
        // Duplicate the params
        ParamArray* src_array = down_cast<ParamArray*>(src);
        ParamArray* dst_array = down_cast<ParamArray*>(dst);
        const ParamArray::ParamRefVector& src_params = src_array->params();
        for (size_t jj = 0; jj < src_params.size(); ++jj) {
          Param* src_param = src_params[jj];
          Param* dst_param =
              dst_array->CreateParamByClass(jj, src_param->GetClass());
          AddMapping(src_param, dst_param);
        }
      }
    }
    AddMapping(src, dst);

    // Map the params.
    if (dst->IsA(ParamObject::GetApparentClass())) {
      ParamObject* src_obj = down_cast<ParamObject*>(src);
      ParamObject* dst_obj = down_cast<ParamObject*>(dst);
      const NamedParamRefMap& params = src_obj->params();
      for (NamedParamRefMap::const_iterator it = params.begin();
           it != params.end(); ++it) {
        const Param* src_param = it->second;
        Param* dst_param = dst_obj->GetUntypedParam(src_param->name());
        AddMapping(src_param, dst_param);
      }
    }
  }

  std::vector<CopierBase::Ref> copiers;
  // TODO(gman): Init this just once.
  // NOTE: The order here is important. Derived types must
  //    come before Base types.
  copiers.push_back(CopierBase::Ref(new Copier<SkinEval>));
  copiers.push_back(CopierBase::Ref(new Copier<StreamBank>));
  copiers.push_back(CopierBase::Ref(new Copier<Primitive>));
  copiers.push_back(CopierBase::Ref(new Copier<Element>));
  copiers.push_back(CopierBase::Ref(new Copier<DrawElement>));
  copiers.push_back(CopierBase::Ref(new Copier<Transform>));
  copiers.push_back(CopierBase::Ref(new Copier<Shape>));
  copiers.push_back(CopierBase::Ref(new Copier<ParamArray>));
  copiers.push_back(CopierBase::Ref(new Copier<ParamObject>));
  copiers.push_back(CopierBase::Ref(new Copier<NamedObject>));
  copiers.push_back(CopierBase::Ref(new Copier<ObjectBase>));

  // Now copy/bind everything.
  for (size_t ii = 0; ii < objects.size(); ++ii) {
    ObjectBase* src = objects[ii];
    ObjectBase* dst = GetNew<ObjectBase>(src);
    if (dst && src != dst) {
      for (size_t jj = 0; jj < copiers.size(); ++jj) {
        if (copiers[jj]->Copy(this, src, dst)) {
          break;
        }
      }
CheckError();
    }
  }
  return new Scene(
      dst_pack,
      GetNew<Transform>(src->root()),
      GetNew<ParamFloat>(src->time_param()));
}

Scene* Scene::Clone(Client* client) const {
  return Cloner::Clone(client, this);
}

}  // namespace o3d_utils

