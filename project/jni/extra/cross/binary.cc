/*
 * Copyright (C) 2010 Tonchidot Corporation.
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

#include "binary.pb.h"

#include <core/cross/types.h>
#include <core/cross/object_base.h>
#include <core/cross/pack.h>
#include <core/cross/param_array.h>
#include <core/cross/skin.h>
#include <core/cross/curve.h>
#include <core/cross/primitive.h>
#include <core/cross/service_locator.h>
#include <core/cross/service_dependency.h>
#include <core/cross/object_manager.h>
#include <core/cross/class_manager.h>
#include <core/cross/renderer.h>
#include <core/cross/texture.h>
#include <core/cross/draw_context.h>
#include <core/cross/sampler.h>
#include <extra/cross/binary.h>
#include <extra/cross/utils.h>

#include <import/cross/memory_stream.h>
#include <import/cross/destination_buffer.h>
#include <import/cross/collada.h>

#include <google/protobuf/io/lzma_protobuf_stream.h>
#include <google/protobuf/io/gzip_stream.h>

#include <vector>
#include <string>
#include <cctype>
#include <cmath>
#include <float.h>

namespace o3d {
namespace extra {
namespace {

// FourCC for our binary format ("O3DB"),
// here as a little endian integer:
static const uint32_t FOURCC = 0x4244334FU;

// Convert O3D enums to our protocol buffers enums
static inline bool set_primitive_type(binary::Primitive& message, Primitive::PrimitiveType value) {
  binary::Primitive::Type x;
  switch (value) {
    case Primitive::POINTLIST:     x = binary::Primitive::POINT_LIST;     break;
    case Primitive::LINELIST:      x = binary::Primitive::LINE_LIST;      break;
    case Primitive::LINESTRIP:     x = binary::Primitive::LINE_STRIP;     break;
    case Primitive::TRIANGLELIST:  x = binary::Primitive::TRIANGLE_LIST;  break;
    case Primitive::TRIANGLESTRIP: x = binary::Primitive::TRIANGLE_STRIP; break;
    case Primitive::TRIANGLEFAN:   x = binary::Primitive::TRIANGLE_FAN;   break;
    default: return false;
  }
  message.set_primitive_type(x);
  return true;
}

static inline bool set_semantic(binary::VertexSource::Stream& stream, Stream::Semantic value) {
  binary::VertexSource::Stream::Semantic x;
  switch (value) {
    case Stream::POSITION: x = binary::VertexSource::Stream::POSITION; break;
    case Stream::NORMAL:   x = binary::VertexSource::Stream::NORMAL;   break;
    case Stream::TANGENT:  x = binary::VertexSource::Stream::TANGENT;  break;
    case Stream::BINORMAL: x = binary::VertexSource::Stream::BINORMAL; break;
    case Stream::COLOR:    x = binary::VertexSource::Stream::COLOR;    break;
    case Stream::TEXCOORD: x = binary::VertexSource::Stream::TEXCOORD; break;
    default: return false;
  }
  stream.set_semantic(x);
  return true;
}

static inline bool set_infinity(binary::Curve& curve, Curve::Infinity pre, Curve::Infinity post) {
  const Curve::Infinity arg[] = { pre, post };
  binary::Curve::Infinity x[2];
  for (size_t i(0); i<2; ++i) {
    switch (arg[i]) {
      case Curve::CONSTANT:       x[i] = binary::Curve::CONSTANT;       break;
      case Curve::LINEAR:         x[i] = binary::Curve::LINEAR;         break;
      case Curve::CYCLE:          x[i] = binary::Curve::CYCLE;          break;
      case Curve::CYCLE_RELATIVE: x[i] = binary::Curve::CYCLE_RELATIVE; break;
      case Curve::OSCILLATE:      x[i] = binary::Curve::OSCILLATE;      break;
      default: return false;
    }
  }
  curve.set_pre_infinity(x[0]);
  curve.set_post_infinity(x[1]);
  return true;
}

// Convert protocol buffer enums back to O3D enums
static inline bool set_primitive_type(Primitive& primitive, binary::Primitive::Type value) {
  Primitive::PrimitiveType x;
  switch (value) {
    case binary::Primitive::POINT_LIST:     x = Primitive::POINTLIST;     break;
    case binary::Primitive::LINE_LIST:      x = Primitive::LINELIST;      break;
    case binary::Primitive::LINE_STRIP:     x = Primitive::LINESTRIP;     break;
    case binary::Primitive::TRIANGLE_LIST:  x = Primitive::TRIANGLELIST;  break;
    case binary::Primitive::TRIANGLE_STRIP: x = Primitive::TRIANGLESTRIP; break;
    case binary::Primitive::TRIANGLE_FAN:   x = Primitive::TRIANGLEFAN;   break;
    default: return false;
  }
  primitive.set_primitive_type(x);
  return true;
}

static inline bool set_semantic(Stream::Semantic& x, binary::VertexSource::Stream::Semantic value) {
  switch (value) {
    case binary::VertexSource::Stream::POSITION: x = Stream::POSITION; break;
    case binary::VertexSource::Stream::NORMAL:   x = Stream::NORMAL;   break;
    case binary::VertexSource::Stream::TANGENT:  x = Stream::TANGENT;  break;
    case binary::VertexSource::Stream::BINORMAL: x = Stream::BINORMAL; break;
    case binary::VertexSource::Stream::COLOR:    x = Stream::COLOR;    break;
    case binary::VertexSource::Stream::TEXCOORD: x = Stream::TEXCOORD; break;
    default: return false;
  }
  return true;
}

static inline bool set_infinity(Curve& curve, binary::Curve::Infinity pre, binary::Curve::Infinity post) {
  const binary::Curve::Infinity arg[] = { pre, post };
  Curve::Infinity x[2];
  for (size_t i(0); i<2; ++i) {
    switch (arg[i]) {
      case binary::Curve::CONSTANT:       x[i] = Curve::CONSTANT;       break;
      case binary::Curve::LINEAR:         x[i] = Curve::LINEAR;         break;
      case binary::Curve::CYCLE:          x[i] = Curve::CYCLE;          break;
      case binary::Curve::CYCLE_RELATIVE: x[i] = Curve::CYCLE_RELATIVE; break;
      case binary::Curve::OSCILLATE:      x[i] = Curve::OSCILLATE;      break;
      default: return false;
    }
  }
  curve.set_pre_infinity(x[0]);
  curve.set_post_infinity(x[1]);
  return true;
}

// Private class responsible for serializing a scenegraph
class Publisher {
 public:
  Publisher(pb::io::CodedOutputStream& stream)
  : mStream(stream)
  , mServiceLocator(0) { }

  bool operator()(Transform& root) {
    mServiceLocator = root.service_locator();

    // Detach the transform from its parent during the export
    Transform* parent(root.parent());
    root.SetParent(0);
    const bool transform_exported(SendTransform(&root));
    root.SetParent(parent);
    if (!transform_exported) return false;

    binary::AtomHeader atom_header;
    atom_header.set_atom_type(binary::AtomHeader::END_OF_ARCHIVE_ATOM);
    if (pbx::write(atom_header, mStream)) {
      binary::EndOfArchive message;
      message.set_root(root.id());
      if (pbx::write(message, mStream))
        return true;
    }
    O3D_ERROR(mServiceLocator) << "Failed to send end_of_archive";
    return false;
  }
 private:
  // Get an indexed string's index, optionally sending the string
  // on the wire if it doesn't exist yet.
  bool GetStringIndex(const std::string& str, size_t& index) {
    string_db_t::reverse_db_type::const_iterator it(mStringDB.reverse_db.find(str));
    if (it != mStringDB.reverse_db.end()) {
      index = it->second;
      return true;
    }

    // not found, create a new indexed string and send it on the wire
    binary::String message;
    if (!str.empty()) message.set_value(str);
    binary::AtomHeader message_header;
    message_header.set_atom_type(binary::AtomHeader::STRING_ATOM);
    if (pbx::write(message_header, mStream)) {
      if (pbx::write(message, mStream)) {
        // Save the string in the local database
        index = mStringDB.db.size();
        mStringDB.db.push_back(str);
        mStringDB.reverse_db[str] = index;
        return true;
      }
    }
    O3D_ERROR(mServiceLocator) << "Failed to send string";
    return false;
  }

  // Sends ObjectHeader
  bool SendObjectHeader(ObjectBase* o) {
    binary::AtomHeader atom_header;
    // OBJECT_ATOM is the default value, so we don't need to do this:
    // atom_header.set_atom_type(binary::AtomHeader::OBJECT_ATOM);
    // (doing it would add some useless bits to the stream)

    binary::ObjectHeader object_header;
    object_header.set_id(o->id());
    // Set type
    const ObjectBase::Class* object_class(o->GetClass());
    O3D_ASSERT(object_class);
    O3D_ASSERT(object_class->name());
    std::string object_class_name(object_class->name());
    size_t index;
    if (!GetStringIndex(object_class_name, index)) return false;
    object_header.set_type(index);
    // Set name
    NamedObjectBase* named;
    std::string n;
    if (named <<* o) {
      if (!named->name().empty()) {
        n = named->name();
        if (!GetStringIndex(named->name(), index)) return false;
        object_header.set_name(index);
      }
    }

    #ifdef MANIAC_DEBUG
    O3D_LOG(INFO) << "XXX Sending object #" << object_header.id()
      << " [name='" << n << "', class='" << object_class_name << "']";
    #endif

    // Send header
    if (pbx::write(atom_header, mStream)) {
      if (pbx::write(object_header, mStream))
        return true;
    }
    O3D_ERROR(mServiceLocator) << "Failed to send object header";
    return false;
  }
  bool SendObjectParamsIfAny(ObjectBase* o) {
    ParamObject* param_obj;
    if (param_obj <<* o) {
      const NamedParamRefMap& params(param_obj->params());
      NamedParamRefMap::const_iterator it(params.begin());
      while (it != params.end()) {
        std::string name(it->first);
        Param* param((it++)->second.Get());
        O3D_ASSERT(param->owner() == param_obj);
        O3D_ASSERT(param->name().compare(name) == 0);
        if (!SendParam(param)) return false;
      }
    }
    return true;
  }

  // Returns true if this object should be ignored
  bool Ignore(ObjectBase* o) {
    // Ignore NULL objects
    if (!o) return true;

    // Ignore object already visited
    if (mVisitedObjects.find(o->id()) != mVisitedObjects.end())
      return true;

    // Mark object as visited
    mVisitedObjects[o->id()] = ObjectBase::Ref(o);

    Param* param;
    if (param <<* o) {
      // Ignore params that have no output connection,
      // if…
      if (param->output_connections().empty()) {
        // …they're either dynamic or read-only.
        if (param->dynamic() || param->read_only())
          return true;

        // …they're ParamVertexBufferStreams, as they're handled
        // when dealing with SkinEval and StreamBank objects.
        if (is_a<ParamVertexBufferStream>(*param))
          return true;

        // …they're ParamDrawLists. We don't want those.
        if (is_a<ParamDrawList>(*param))
          return true;

        // …they're ParamEffects and their owner is a standard Collada material,
        // as we can rebuild these at runtime.
        if (is_a<ParamEffect>(*param) && is_a<Material>(*param->owner())) {
          ParamString* colladaLightingTypeParam(param->owner()->GetParam<ParamString>(Collada::kLightingTypeParamName));
          if (colladaLightingTypeParam) {
            std::string val(colladaLightingTypeParam->value());
            for (size_t i(0); i<val.size(); ++i) val[i]=::tolower(val[i]);
            if (!(val.compare(Collada::kLightingTypePhong) &&
                  val.compare(Collada::kLightingTypeLambert) &&
                  val.compare(Collada::kLightingTypeBlinn) &&
                  val.compare(Collada::kLightingTypeConstant)))
              return true;
          }
        }
      }
    }
    return false;
  }

  // Sets ignored and returns true if x is ignored, otherwise proceed
  #define CHECK_IGNORE(x) for (bool _x(Ignore(x)); (*(ignored?ignored:&_x)=_x);) return true

  bool SendParam(Param* o, bool* ignored=0, ParamArray* array=0, size_t index=0) {
    CHECK_IGNORE(o);

    binary::Param message;
    if (o->owner()) {
      // We need to send the owner here because 'o' might be an input connection
      // owned by an object not sent yet!
      if (!Send(o->owner())) return false;
      message.set_owner_ref(o->owner()->id());
    }
    else if (array) {
      if (!SendParamArray(array)) return false;
      message.set_owner_ref(array->id());
      message.set_index(index);
    }

    if (o->input_connection()) {
      if (!SendParam(o->input_connection())) return false;
      message.set_input_connection_ref(o->input_connection()->id());
    }
    else do {
      ParamBoolean* as_boolean;
      if (as_boolean <<* o) {
        // it's false by default
        if (as_boolean->value()) message.set_bool_value(true);
        break;
      }

      ParamInteger* as_integer;
      if (as_integer <<* o) {
        // it's 0 by default
        if (as_integer->value()) message.set_integer_value(as_integer->value());
        break;
      }

      ParamString* as_string;
      if (as_string <<* o) {
        size_t index;
        if (!GetStringIndex(as_string->value(), index)) return false;
        message.set_indexed_string_value(index);
        break;
      }

      ParamFloat* as_float;
      if (as_float <<* o) {
        message.add_float_value(as_float->value());
        break;
      }

      ParamFloat2* as_float2;
      if (as_float2 <<* o) {
        Float2 value(as_float2->value());
        message.add_float_value(value[0]);
        message.add_float_value(value[1]);
        break;
      }

      ParamFloat3* as_float3;
      if (as_float3 <<* o) {
        Float3 value(as_float3->value());
        message.add_float_value(value[0]);
        message.add_float_value(value[1]);
        message.add_float_value(value[2]);
        break;
      }

      ParamFloat4* as_float4;
      if (as_float4 <<* o) {
        Float4 value(as_float4->value());
        message.add_float_value(value[0]);
        message.add_float_value(value[1]);
        message.add_float_value(value[2]);
        message.add_float_value(value[3]);
        break;
      }

      ParamMatrix4* as_matrix4;
      if (as_matrix4 <<* o) {
        Matrix4 value(as_matrix4->value());

        if (!matrix4_extra::is_identity(value)) {
          Quat rotation;
          Vector3 position;
          float scale;
          if (matrix4_extra::decompose(value, rotation, position, scale)) {
            message.add_float_value(rotation[0]);
            message.add_float_value(rotation[1]);
            message.add_float_value(rotation[2]);
            message.add_float_value(rotation[3]);

            message.add_float_value(position[0]);
            message.add_float_value(position[1]);
            message.add_float_value(position[2]);

            message.add_float_value(scale);
          }
          else for (size_t i(0); i<4; ++i) {
            message.add_float_value(value[i][0]);
            message.add_float_value(value[i][1]);
            message.add_float_value(value[i][2]);
            message.add_float_value(value[i][3]);
          }
        }
        break;
      }

      // No need to send bounding-boxes data,
      // as these will be recomputed anyway
      ParamBoundingBox* as_bbox;
      if (as_bbox <<* o) break;

      RefParamBase* as_ref;
      if (as_ref <<* o) {
        ObjectBase* value(as_ref->value());
        if (value) {
          if (!Send(value)) return false;
          message.set_object_ref_value(value->id());
        }
        break;
      }

      O3D_ERROR(mServiceLocator) << "Unsupported Param type: " << o->GetClass()->name();
      return false;
    } while (false);

    if (!SendObjectHeader(o)) return false;
    if (!pbx::write(message, mStream)) {
      O3D_ERROR(mServiceLocator) << "Failed to send object";
      return false;
    }
    return true;
  }
  bool SendParamArray(ParamArray* o, bool* ignored = 0) {
    CHECK_IGNORE(o);

    if (!SendObjectHeader(o)) {
      O3D_ERROR(mServiceLocator) << "Failed to send object header";
      return false;
    }

    const ParamArray::ParamRefVector& params(o->params());
    for (size_t i(0); i<params.size(); ++i) {
      Param* param = params[i].Get();
      // Any ignored param would mess up the array indices,
      // so we fail if it happens
      bool ignored;
      if (!SendParam(param, &ignored, o, i)) {
        O3D_ERROR(mServiceLocator) << "Failed to send param";
        return false;
      }
      if (ignored) {
        O3D_ERROR(mServiceLocator) << "Required param was ignored";
        return false;
      }
    }
    return true;
  }
  bool SendEffect(Effect* o, bool* ignored = 0) {
    CHECK_IGNORE(o);

    binary::Effect message;
    if (!o->source().empty()) {
      size_t index;
      if (!GetStringIndex(o->source(), index)) {
        O3D_ERROR(mServiceLocator) << "Failed to index effect's source string";
        return false;
      }
      message.set_source_indexed_string(index);
    }

    if (!SendObjectHeader(o)) {
      O3D_ERROR(mServiceLocator) << "Failed to send object header";
      return false;
    }
    if (!pbx::write(message, mStream)) {
      O3D_ERROR(mServiceLocator) << "Failed to send object";
      return false;
    }
    if (!SendObjectParamsIfAny(o)) {
      O3D_ERROR(mServiceLocator) << "Failed to send object's params";
      return false;
    }
    return true;
  }
  bool SendSkin(Skin* o, bool* ignored = 0) {
    CHECK_IGNORE(o);

    binary::Skin message;

    // Save the influences
    const Skin::InfluencesArray& influences(o->influences());
    for (size_t i(0); i<influences.size(); ++i) {
      binary::Skin::InfluenceArray& ia(*message.add_influence_array());
      for (size_t j(0); j<influences[i].size(); ++j) {
        binary::Skin::InfluenceArray::Influence& inf(*ia.add_influence());
        inf.set_matrix_index(influences[i][j].matrix_index);
        inf.set_weight(influences[i][j].weight);
      }
    }

    // Save the pose matrices
    const Skin::MatrixArray& inverse_bind_pose_matrices(o->inverse_bind_pose_matrices());
    const float* source((const float*)(&inverse_bind_pose_matrices[0]));
    std::copy(source, source+sizeof(Matrix4)*inverse_bind_pose_matrices.size(),
      pb::RepeatedFieldBackInserter(message.mutable_inverse_bind_pose_matrice()));

    if (!SendObjectHeader(o)) {
      O3D_ERROR(mServiceLocator) << "Failed to send object header";
      return false;
    }
    if (!pbx::write(message, mStream)) {
      O3D_ERROR(mServiceLocator) << "Failed to send object";
      return false;
    }
    if (!SendObjectParamsIfAny(o)) {
      O3D_ERROR(mServiceLocator) << "Failed to send object's params";
      return false;
    }
    return true;
  }
  bool SendCurve(Curve* o, bool* ignored = 0) {
    CHECK_IGNORE(o);

    binary::Curve message;
    if (!set_infinity(message, o->pre_infinity(), o->post_infinity())) {
      O3D_ERROR(mServiceLocator) << "Unsupported infinity";
      return false;;
    }
    message.set_use_cache(o->use_cache());
    message.set_sample_rate(o->sample_rate());

    const CurveKeyRefArray& keys(o->keys());
    for (size_t i(0); i<keys.size(); ++i) {
      binary::Curve::Key& key(*message.add_key());

      BezierCurveKey* bezier;
      if (is_a<StepCurveKey>(*keys[i])) key.set_type(binary::Curve::Key::STEP);
      else if (is_a<LinearCurveKey>(*keys[i])) key.set_type(binary::Curve::Key::LINEAR);
      else if (bezier <<* keys[i].Get()) {
        key.set_type(binary::Curve::Key::BEZIER);
        key.add_bezier_tangent(bezier->in_tangent()[0]);
        key.add_bezier_tangent(bezier->in_tangent()[1]);
        key.add_bezier_tangent(bezier->out_tangent()[0]);
        key.add_bezier_tangent(bezier->out_tangent()[1]);
      }
      else {
        O3D_ERROR(mServiceLocator) << "Unsupported CurveKey type: " << keys[i]->GetClass()->name();
        return false;
      }

      key.set_input(keys[i]->input());
      key.set_output(keys[i]->output());
    }

    if (!SendObjectHeader(o)) {
      O3D_ERROR(mServiceLocator) << "Failed to send object header";
      return false;
    }
    if (!pbx::write(message, mStream)) {
      O3D_ERROR(mServiceLocator) << "Failed to send object";
      return false;
    }
    if (!SendObjectParamsIfAny(o)) {
      O3D_ERROR(mServiceLocator) << "Failed to send object's params";
      return false;
    }
    return true;
  }
  bool SendBuffer(Buffer* o, bool* ignored = 0) {
    CHECK_IGNORE(o);

    const bool export_data(!is_a<DestinationBuffer>(*o));

    binary::Buffer message;
    // Buffer.num_elements
    message.set_num_elements(o->num_elements());

    // Buffer.field
    const FieldRefArray& fields(o->fields());
    for (size_t i(0); i<fields.size(); ++i) {
      binary::Buffer::Field& field(*message.add_field());
      // Buffer.field.id
      field.set_id(fields[i]->id());
      mVisitedObjects[fields[i]->id()] = ObjectBase::Ref(fields[i]);
      // Buffer.field.name
      if (!fields[i]->name().empty()) {
        size_t index;
        if (!GetStringIndex(fields[i]->name(), index)) {
          O3D_ERROR(mServiceLocator) << "Failed to index field's name";
          return false;
        }
        field.set_name(index);
      }
      // Buffer.field.num_components
      field.set_num_components(fields[i]->num_components());

      do {
        FloatField* float_field;
        if (float_field <<* fields[i]) {
          // Buffer.field.type
          field.set_type(binary::Buffer::Field::FLOAT);
          // Buffer.field.data
          if (export_data) {
            const size_t count(o->num_elements() * float_field->num_components());

            std::vector<float> tmp(count);
            float_field->GetAsFloats(0, &tmp[0], float_field->num_components(), o->num_elements());

            pb::RepeatedField<float>& data(*field.mutable_value_float());
            data.Reserve(count);
            for (size_t n(0); n<count; ++n) data.AddAlreadyReserved(tmp[n]);
          }
          break;
        }
        UInt32Field* uint32_field;
        if (uint32_field <<* fields[i]) {
          // Buffer.field.type
          field.set_type(binary::Buffer::Field::UINT32);
          // Buffer.field.data
          if (export_data) {
            const size_t count(o->num_elements() * uint32_field->num_components());

            std::vector<uint32_t> tmp(count);
            uint32_field->GetAsUInt32s(0, &tmp[0], uint32_field->num_components(), o->num_elements());

            pb::RepeatedField<uint32_t>& data(*field.mutable_value_uint());
            data.Reserve(count);
            for (size_t n(0); n<count; ++n) data.AddAlreadyReserved(tmp[n]);
          }
          break;
        }
        #ifdef GLES2_BACKEND_NATIVE_GLES2
        UInt16Field* uint16_field;
        if (uint16_field <<* fields[i]) {
          // Buffer.field.type
          field.set_type(binary::Buffer::Field::UINT16);
          // Buffer.field.data
          if (export_data) {
            const size_t count(o->num_elements() * uint16_field->num_components());

            std::vector<uint16_t> tmp(count);
            uint16_field->GetAsUInt16s(0, &tmp[0], uint16_field->num_components(), o->num_elements());

            pb::RepeatedField<uint32_t>& data(*field.mutable_value_uint());
            data.Reserve(count);
            for (size_t n(0); n<count; ++n) data.AddAlreadyReserved(tmp[n]);
          }
          break;
        }
        #endif
        UByteNField* ubyten_field;
        if (ubyten_field <<* fields[i]) {
          // Buffer.field.type
          field.set_type(binary::Buffer::Field::BYTE);
          // Buffer.field.data
          if (export_data) {
            const size_t count(o->num_elements() * ubyten_field->num_components());
            std::string& data(*field.mutable_value_byte());
            data.resize(count);
            ubyten_field->GetAsUByteNs(0, (uint8_t*) &data[0], ubyten_field->num_components(), o->num_elements());
          }
          break;
        }
        O3D_ERROR(mServiceLocator) << "Unsupported field type: " << fields[i]->GetClass()->name();
        return false;
      } while(false);
    }

    if (!SendObjectHeader(o)) {
      O3D_ERROR(mServiceLocator) << "Failed to send object header";
      return false;
    }
    if (!pbx::write(message, mStream)) {
      O3D_ERROR(mServiceLocator) << "Failed to send object";
      return false;
    }
    if (!SendObjectParamsIfAny(o)) {
      O3D_ERROR(mServiceLocator) << "Failed to send object's params";
      return false;
    }
    return true;
  }
  bool SendVertexSource(VertexSource* o, bool* ignored = 0) {
    CHECK_IGNORE(o);

    binary::VertexSource message;

    // How comes vertex_stream_params ain't defined in VertexSource??
    StreamBank* as_stream_bank(as_stream_bank <<* o);
    SkinEval* as_skin_eval(as_skin_eval <<* o);
    StreamParamVector empty_vector;
    const StreamParamVector& stream_param_vector(
      as_stream_bank?as_stream_bank->vertex_stream_params():(
      as_skin_eval?as_skin_eval->vertex_stream_params():
      empty_vector
    ));

    for (size_t i(0); i<stream_param_vector.size(); ++i) {
      const Stream& s(stream_param_vector[i]->stream());
      if (!SendBuffer(s.field().buffer())) {
        O3D_ERROR(mServiceLocator) << "Failed to send buffer";
        return false;
      }

      binary::VertexSource::Stream& stream(*message.add_stream());
      stream.set_field_ref(s.field().id());
      stream.set_start_index(s.start_index());
      if (!set_semantic(stream, s.semantic())) {
        O3D_ERROR(mServiceLocator) << "Unsupported stream semantic";
        return false;
      }
      stream.set_semantic_index(s.semantic_index());
      if (stream_param_vector[i]->input_connection()) {
        ObjectBase* dependency(stream_param_vector[i]->input_connection()->owner());
        if (!Send(dependency)) {
          O3D_ERROR(mServiceLocator) << "Failed to send dependency";
          return false;
        }
        stream.set_bind(dependency->id());
      }
    }

    if (!SendObjectHeader(o)) {
      O3D_ERROR(mServiceLocator) << "Failed to send object header";
      return false;
    }
    if (!pbx::write(message ,mStream)) {
      O3D_ERROR(mServiceLocator) << "Failed to send object";
      return false;
    }
    if (!SendObjectParamsIfAny(o)) {
      O3D_ERROR(mServiceLocator) << "Failed to send object's params";
      return false;
    }
    return true;
  }
  bool SendPrimitive(Primitive* o, bool* ignored = 0) {
    CHECK_IGNORE(o);

    binary::Primitive message;

    // Save the index buffer
    if (o->indexed()) {
      if (!SendBuffer(o->index_buffer())) {
        O3D_ERROR(mServiceLocator) << "Failed to send index buffer";
        return false;
      }
      message.set_index_buffer_ref(o->index_buffer()->id());
    }

    // Save the stream bank
    if (o->stream_bank()) {
      if (!SendVertexSource(o->stream_bank())) {
        O3D_ERROR(mServiceLocator) << "Failed to send streambank";
        return false;
      }
      message.set_stream_bank_ref(o->stream_bank()->id());
    }

    // Save the primitive
    if (!set_primitive_type(message, o->primitive_type())) {
      O3D_ERROR(mServiceLocator) << "Unsupported primitive type";
      return false;
    }
    message.set_number_vertices(o->number_vertices());
    message.set_number_primitives(o->number_primitives());
    message.set_start_index(o->start_index());
    if (o->owner()) {
      if (!SendShape(o->owner())) {
        O3D_ERROR(mServiceLocator) << "Failed to send shape";
        return false;
      }
      message.set_owner_ref(o->owner()->id());
    }

    if (!SendObjectHeader(o)) {
      O3D_ERROR(mServiceLocator) << "Failed to send object header";
      return false;
    }
    if (!pbx::write(message, mStream)) {
      O3D_ERROR(mServiceLocator) << "Failed to send object";
      return false;
    }
    if (!SendObjectParamsIfAny(o)) {
      O3D_ERROR(mServiceLocator) << "Failed to send object's params";
      return false;
    }
    return true;
  }
  bool SendShape(Shape* o, bool* ignored = 0) {
    CHECK_IGNORE(o);

    // Save the Shape
    if (!SendObjectHeader(o)) {
      O3D_ERROR(mServiceLocator) << "Failed to send object header";
      return false;
    }
    if (!SendObjectParamsIfAny(o)) {
      O3D_ERROR(mServiceLocator) << "Failed to send object's params";
      return false;
    }

    // Save the shape's elements, which reference the shape
    ElementRefArray::const_iterator it(o->GetElementRefs().begin());
    while (it != o->GetElementRefs().end()) {
      Element* element((it++)->Get());
      Primitive* primitive;
      if (primitive <<* element) {
        if (!SendPrimitive(primitive)) {
          O3D_ERROR(mServiceLocator) << "Failed to send primitive";
          return false;
        }
      }
      else {
        O3D_ERROR(mServiceLocator) << "Unsupported Element type: " << element->GetClass()->name();
        return false;
      }
    }
    return true;
  }
  bool SendTransform(Transform* o, bool* ignored = 0) {
    CHECK_IGNORE(o);

    binary::Transform message;

    // No parent means the transform is the root transform
    if (o->parent()) {
      if (!SendTransform(o->parent())) {
        O3D_ERROR(mServiceLocator) << "Failed to save transform's parent";
        return false;
      }
      message.set_parent_ref(o->parent()->id());
    }

    // Save the transform
    if (!SendObjectHeader(o)) {
      O3D_ERROR(mServiceLocator) << "Failed to send object header";
      return false;
    }
    if (!pbx::write(message, mStream)) {
      O3D_ERROR(mServiceLocator) << "Failed to send object";
      return false;
    }
    if (!SendObjectParamsIfAny(o)) {
      O3D_ERROR(mServiceLocator) << "Failed to send object's params";
      return false;
    }

    // Save all the shapes needed by this transform
    binary::Attachment attachments;
    attachments.set_object_ref(o->id());
    ShapeRefArray::const_iterator shape_it(o->GetShapeRefs().begin());
    while (shape_it != o->GetShapeRefs().end()) {
      Shape* shape((shape_it++)->Get());
      if (shape) {
        if (!SendShape(shape)) {
          O3D_ERROR(mServiceLocator) << "Failed to send shape";
          return false;
        }
        attachments.add_attachment_ref(shape->id());
      }
    }
    binary::AtomHeader attachments_header;
    attachments_header.set_atom_type(binary::AtomHeader::ATTACHMENT_ATOM);
    if (!pbx::write(attachments_header, mStream)) {
      O3D_ERROR(mServiceLocator) << "Failed to send attachments header";
      return false;
    }
    if (!pbx::write(attachments, mStream)) {
      O3D_ERROR(mServiceLocator) << "Failed to send attachments header";
      return false;
    }

    // Save all the children, who reference this transform
    TransformRefArray::const_iterator transform_it(o->GetChildrenRefs().begin());
    while (transform_it != o->GetChildrenRefs().end()) {
      Transform* transform((transform_it++)->Get());
      if (!SendTransform(transform)) {
        O3D_ERROR(mServiceLocator) << "Failed to send transform";
        return false;
      }
    }
    return true;
  }
  bool SendTexture(Texture* o, bool* ignored = 0) {
    CHECK_IGNORE(o);

    binary::Texture message;

    // o3d.original_uri is a param, so it's sent separately, but
    // since we need it at texture's creation time we send it now
    // as well.
    ParamString* original_uri(o->GetParam<ParamString>(O3D_STRING_CONSTANT("original_uri")));
    if (!original_uri) {
      O3D_ERROR(mServiceLocator) << "No 'o3d.original_uri' parameter found in texture";
      return false;
    }
    size_t index;
    if (!GetStringIndex(original_uri->value(), index)) {
      O3D_ERROR(mServiceLocator) << "Failed to index texture's uri";
      return false;
    }
    message.set_uri(index);

    if (!SendObjectHeader(o)) {
      O3D_ERROR(mServiceLocator) << "Failed to send object header";
      return false;
    }
    if (!pbx::write(message, mStream)) {
      O3D_ERROR(mServiceLocator) << "Failed to send object";
      return false;
    }
    if (!SendObjectParamsIfAny(o)) {
      O3D_ERROR(mServiceLocator) << "Failed to send object's params";
      return false;
    }
    return true;
  }
  bool Send(ObjectBase* o, bool* ignored = 0) {
    // Objects that carry extra information in a PB message
    #define DISPATCH(CLASS) {\
    CLASS* x;\
    if (x <<* o) {\
      if (Send##CLASS(x)) return true;\
      else {\
        O3D_ERROR(mServiceLocator) << "Failed to send " << #CLASS;\
        return false;\
      }\
    }}
    DISPATCH(Param);
    DISPATCH(ParamArray);
    DISPATCH(Effect);
    DISPATCH(Skin);
    DISPATCH(Curve);
    DISPATCH(Buffer);
    DISPATCH(VertexSource);
    DISPATCH(Primitive);
    DISPATCH(Transform);
    DISPATCH(Texture);
    #undef DISPATCH

    CHECK_IGNORE(o);
    if (!SendObjectHeader(o)) {
      O3D_ERROR(mServiceLocator) << "Failed to send object header";
      return false;
    }
    if (!SendObjectParamsIfAny(o)) {
      O3D_ERROR(mServiceLocator) << "Failed to send object's params";
      return false;
    }

    return true;
  }
 private:
  string_db_t mStringDB;
  std::tr1::unordered_map<Id, ObjectBase::Ref> mVisitedObjects;
  pb::io::CodedOutputStream& mStream;
  ServiceLocator* mServiceLocator;
};

class Load {
 public:
  Load(Pack& pack, pb::io::ZeroCopyInputStream& stream, IExternalResourceProvider& erp)
  : mERP(erp), mPack(pack), mStream(stream), mServiceLocator(pack.service_locator()) { }
  Transform* operator()() {
    // Build a map of all the O3D classes
    IClassManager* class_manager(mPack.service_locator()->GetService<IClassManager>());
    std::vector<const ObjectBase::Class*> o3d_classes(class_manager->GetAllClasses());
    for (size_t i(0); i<o3d_classes.size(); ++i) {
      mClassMap[o3d_classes[i]->name()] = o3d_classes[i];
    }
    std::vector<const ObjectBase::Class*>().swap(o3d_classes);

    // Receive loop
    Transform* root(0);
    while (true) {
      binary::AtomHeader atom_header;
      if (!pbx::read(atom_header, mStream)) {
        O3D_ERROR(mServiceLocator) << "Failed to parse atom header";
        return false;
      }

      switch (atom_header.atom_type()) {
        case binary::AtomHeader::END_OF_ARCHIVE_ATOM:
          {
            binary::EndOfArchive eoa;
            if (!pbx::read(eoa, mStream)) {
              O3D_ERROR(mServiceLocator) << "Failed to parse end of archive";
              return false;
            }
            ObjectBase::Ref ref(GetObjectRef(eoa.root()));
            if (!ref) {
              O3D_ERROR(mServiceLocator) << "Couldn't find root transform. Missing dependency?";
              return false;
            }
            if (root <<* ref) return root;
            else {
              O3D_ERROR(mServiceLocator) << "Root ref doesn't reference a Transform object";
              return false;
            }
          }
          return 0;
        case binary::AtomHeader::STRING_ATOM:
          if (!ReceiveString()) {
            O3D_ERROR(mServiceLocator) << "Failed to deserialize a string";
            return false;
          }
          break;
        case binary::AtomHeader::OBJECT_ATOM:
          if (!ReceiveObject()) {
            O3D_ERROR(mServiceLocator) << "Failed to deserialize an object";
            return false;
          }
          break;
        case binary::AtomHeader::ATTACHMENT_ATOM:
          if (!ReceiveAttachments()) {
            O3D_ERROR(mServiceLocator) << "Failed to deserialize attachments";
            return false;
          }
          break;
      }
    }
  }

 private:
  template<typename T, typename V>
  static void set_param_value(T& x, const V& v) {
    if (x.dynamic())
      x.set_dynamic_value(v);
    else if (x.read_only())
      x.set_read_only_value(v);
    else
      x.set_value(v);
  }
  template<typename T>
  static bool try_to_set_typed_param_ref(Param& o, ObjectBase* ref) {
    T* x;
    typename T::WeakPointerType::ClassType* v;
    if (!((x << o) && (v <<* ref))) return false;
    set_param_value(*x, v);
    return true;
  }

  template<typename T>
  bool ReceiveAs(ObjectBase& o, bool& success) {
    T* x;
    if (x << o) {
      success=Receive(*x);
      return true;
    }
    return false;
  }

  ObjectBase::Ref GetObjectRef(uint32_t id) const {
    std::tr1::unordered_map<uint32_t, ObjectBase::Ref>::const_iterator it(mOldIdToNewObject.find(id));
    if (it == mOldIdToNewObject.end()) {
      // Shoud we assert() here?
      return ObjectBase::Ref(0);
    }
    return it->second;
  }

  bool ReceiveString() {
    binary::String indexed_string;
    if (!pbx::read(indexed_string, mStream)) {
      O3D_ERROR(mServiceLocator) << "Failed to parse indexed string message";
      return false;
    }
    mStringDB.db.push_back(indexed_string.value());
    mStringDB.reverse_db[indexed_string.value()] = mStringDB.db.size()-1;
    return true;
  }

  bool ReceiveAttachments() {
    binary::Attachment attachments;
    if (!pbx::read(attachments, mStream)) {
      O3D_ERROR(mServiceLocator) << "Failed to parse attachments";
      return false;
    }
    ObjectBase::Ref object(GetObjectRef(attachments.object_ref()));
    if (!object) {
      O3D_ERROR(mServiceLocator) << "Can't find object referenced by Attachment. Missing dependency? Ref was " << attachments.object_ref();
      return false;
    }

    {
      Transform* transform;
      if (transform <<* object) {
        for(size_t i(0); i<size_t(attachments.attachment_ref_size()); ++i) {
          ObjectBase::Ref attch(GetObjectRef(attachments.attachment_ref(i)));
          if (!attch) {
            O3D_ERROR(mServiceLocator) << "Can't find attachment. Missing dependency? Ref was " << attachments.attachment_ref(i);
            return false;
          }

          Shape* shape;
          if (shape <<* attch) {
            transform->AddShape(shape);
            continue;
          }

          O3D_ERROR(mServiceLocator) << "Unsupported attachment targeting a Transform";
          return false;
        }
        return true;
      }
    }

    O3D_ERROR(mServiceLocator) << "Unsupported recipient class for attachment";
    return false;
  }

  bool ReceiveObject() {
    // Parse object header
    binary::ObjectHeader object_header;
    if (!pbx::read(object_header, mStream)) {
      O3D_ERROR(mServiceLocator) << "Failed to parse object header";
      return false;
    }

    // Get type information
    if (object_header.type() >= mStringDB.db.size()) {
      O3D_ERROR(mServiceLocator) << "Object type out of range";
      return false;
    }
    const std::string& classname(mStringDB.db[object_header.type()]);

    std::tr1::unordered_map<std::string, const ObjectBase::Class*>::iterator it(mClassMap.find(classname));
    if (it == mClassMap.end()) {
      O3D_ERROR(mServiceLocator) << "Cannot find any class";
      return false;
    }

    const ObjectBase::Class* cls(it->second);

    // Get name, if any
    std::string name;
    if (object_header.has_name()) {
      if (object_header.name() >= mStringDB.db.size()) {
        O3D_ERROR(mServiceLocator) << "Object name out of range";
        return false;
      }
      name = mStringDB.db[object_header.name()];
    }

    #ifdef MANIAC_DEBUG
    O3D_LOG(INFO) << "XXX Receiving object #" << object_header.id()
      << " [name='" << name << "', class='" << cls->name() << "']";
    #endif

    // If object is a Param, we need to construct it using CreateParamByClass
    if (ObjectBase::ClassIsA(cls, Param::GetApparentClass())) {
      Param* param(0);
      binary::Param message;
      if (!pbx::read(message, mStream)) {
        O3D_ERROR(mServiceLocator) << "Can't parse Param";
        return false;
      }
      if (message.has_owner_ref()) {
        ObjectBase::Ref owner(GetObjectRef(message.owner_ref()));
        if (!owner) {
          O3D_ERROR(mServiceLocator) << "Can't find param's owner. Missing dependency? Ref was " << message.owner_ref();
          return false;
        }
        ParamArray*  param_array;
        ParamObject* param_object;
        if (param_object <<* owner) {
          // Check if the param already exists (this is the case for
          // all the params created in o3d objects' constructor).
          Param* existant(param_object->GetUntypedParam(name));
          if (existant) {
            std::string classname(cls->name());
            if (classname.compare(existant->GetClass()->name())==0)
              param = existant;
          }
          // Param doesn't exist yet, so let create it
          if (!param) {
            param = param_object->CreateParamByClass(name, cls);
          }
        }
        else if ((param_array <<* owner) && message.has_index()) {
          param = param_array->CreateParamByClass(message.index(), cls);
          if (!name.empty()) param->SetName(name);
        }
        else {
          O3D_ERROR(mServiceLocator) << "Param has an owner, but it's not a ParamObject nor a ParamArray, but a " << owner->GetClass()->name();
          return false;
        }
      }
      else if (param << *mPack.CreateObjectByClass(cls)) {
        // Free Param. Do we really have these??
        if (!name.empty()) param->SetName(name);
      }

      if (!param) {
        O3D_ERROR(mServiceLocator) << "Could not create requested Param";
        return false;
      }

      mOldIdToNewObject[object_header.id()] = ObjectBase::Ref(param);
      if (!ParseParam(*param, message)) {
        O3D_ERROR(mServiceLocator) << "Failed to parse param";
        return false;
      }
    }
    // If it's a texture, we need to create it using CreateTexture
    else if (ObjectBase::ClassIsA(cls, Texture::GetApparentClass())) {
      Texture* texture(0);
      binary::Texture message;
      if (!pbx::read(message, mStream)) {
        O3D_ERROR(mServiceLocator) << "Failed to parse a texture";
        return false;
      }

      if (message.uri() >= mStringDB.db.size()) {
        O3D_ERROR(mServiceLocator) << "Texture uri out of range";
        return false;
      }
      const std::string& uri(mStringDB.db[message.uri()]);

      if (uri.compare("#error")==0) {
        Renderer* renderer(mPack.service_locator()->GetService<Renderer>());
        texture = renderer->error_texture()?renderer->error_texture():renderer->fallback_error_texture();
      }
      else {
        ExternalResource::Ref res(mERP.GetExternalResourceForURI(mPack, uri));
        if (!res) {
          O3D_ERROR(mServiceLocator) << "Failed to fetch data for texture at \"" << uri << "\"";
          return false;
        }

        BitmapRefArray bitmap_refs;
        MemoryReadStream mrs(res->data(), res->size());
        const bool loaded(Bitmap::LoadFromStream(mPack.service_locator(), &mrs, uri, image::UNKNOWN, &bitmap_refs));
        if (!loaded) {
          O3D_ERROR(mServiceLocator) << "Failed to load bitmaps for texture at \"" << uri << "\"";
          return false;
        }

        texture = mPack.CreateTextureFromBitmaps(bitmap_refs, uri, true);
        if (classname.compare(texture->GetClass()->name())) {
          O3D_ERROR(mServiceLocator) << "Texture type mismatch when rebuilding texture";
          return false;
        }
      }

      if (!name.empty()) texture->set_name(name);
      mOldIdToNewObject[object_header.id()] = ObjectBase::Ref(texture);
    }
    else {
      ObjectBase::Ref object(mPack.CreateObjectByClass(cls));
      mOldIdToNewObject[object_header.id()] = object;

      NamedObject* named;
      if (named <<* object) named->set_name(name);

      bool success(true);
      do {
        if (ReceiveAs<Effect      >(*object, success)) break;
        if (ReceiveAs<Skin        >(*object, success)) break;
        if (ReceiveAs<Curve       >(*object, success)) break;
        if (ReceiveAs<Buffer      >(*object, success)) break;
        if (ReceiveAs<VertexSource>(*object, success)) break;
        if (ReceiveAs<Primitive   >(*object, success)) break;
        if (ReceiveAs<Transform   >(*object, success)) break;
      } while (false);

      if (!success) {
        O3D_ERROR(mServiceLocator) << "Failed to deserialize a " << classname;
        return false;
      }
    }
    return true;
  }

  bool ParseParam(Param& o, binary::Param& message) {
    // set input connection
    if (message.has_input_connection_ref()) {
      ObjectBase::Ref input(GetObjectRef(message.input_connection_ref()));
      if (!input) {
        O3D_ERROR(mServiceLocator) << "Can't find param's input connection. Missing dependency?";
        return false;
      }

      Param* param;
      if (param <<* input) {
        o.Bind(param);
        return true;
      }
      else {
        O3D_ERROR(mServiceLocator) << "Param has an input connection, but it's not a Param";
        return false;
      }
    }
    else {
      // assign value
      switch (message.float_value_size()) {
        case 0:
        {
          ParamBoolean* as_boolean;
          if (as_boolean << o) {
            set_param_value(*as_boolean, message.bool_value());
            return true;
          }

          ParamInteger* as_integer;
          if (as_integer << o) {
            set_param_value(*as_integer, message.integer_value());
            return true;
          }

          ParamString* as_string;
          if (as_string << o) {
            if (message.has_indexed_string_value()) {
              if (message.indexed_string_value() >= mStringDB.db.size()) {
                O3D_ERROR(mServiceLocator) << "std::string index out of bounds";
                return false;
              }
              set_param_value(*as_string, mStringDB.db[message.indexed_string_value()]);
            }
            return true;
          }

          ParamBoundingBox* as_bbox;
          if (as_bbox << o) return true;

          ParamMatrix4* as_matrix4;
          if (as_matrix4 << o) {
            // identity matrix
            set_param_value(*as_matrix4, Matrix4::identity());
            return true;
          }

          RefParamBase* as_ref;
          if (as_ref << o) {
            if (message.has_object_ref_value()) {
              ObjectBase::Ref ref(GetObjectRef(message.object_ref_value()));
              if (ref) {
                if (try_to_set_typed_param_ref<ParamDrawContext              >(o, ref.Get())) return true;
                if (try_to_set_typed_param_ref<ParamDrawList                 >(o, ref.Get())) return true;
                if (try_to_set_typed_param_ref<ParamEffect                   >(o, ref.Get())) return true;
                if (try_to_set_typed_param_ref<ParamFunction                 >(o, ref.Get())) return true;
                if (try_to_set_typed_param_ref<ParamMaterial                 >(o, ref.Get())) return true;
                if (try_to_set_typed_param_ref<ParamParamArray               >(o, ref.Get())) return true;
                if (try_to_set_typed_param_ref<ParamRenderSurface            >(o, ref.Get())) return true;
                if (try_to_set_typed_param_ref<ParamRenderDepthStencilSurface>(o, ref.Get())) return true;
                if (try_to_set_typed_param_ref<ParamSampler                  >(o, ref.Get())) return true;
                if (try_to_set_typed_param_ref<ParamSkin                     >(o, ref.Get())) return true;
                if (try_to_set_typed_param_ref<ParamState                    >(o, ref.Get())) return true;
                if (try_to_set_typed_param_ref<ParamStreamBank               >(o, ref.Get())) return true;
                if (try_to_set_typed_param_ref<ParamTexture                  >(o, ref.Get())) return true;
                if (try_to_set_typed_param_ref<ParamTransform                >(o, ref.Get())) return true;
                O3D_ERROR(mServiceLocator) << "Failed to set the value of a " << o.GetClass()->name() << " to point to a " << ref->GetClass()->name();
                return false;
              }
            }
            // This is a NULL ref, which is fine too..
            return true;
          }
          O3D_ERROR(mServiceLocator) << "Unsupported object reference for " << o.GetClass()->name() << "(" << o.name() << ")";
          return false;
        }
        case 1:
        {
          ParamFloat* as_float;
          if (as_float << o) {
            set_param_value(*as_float, message.float_value(0));
            return true;
          }
          break;
        }
        case 2:
        {
          ParamFloat2* as_float2;
          if (as_float2 << o) {
            Float2 v;
            v[0] = message.float_value(0);
            v[1] = message.float_value(1);
            set_param_value(*as_float2, v);
            return true;
          }
          break;
        }
        case 3:
        {
          ParamFloat3* as_float3;
          if (as_float3 << o) {
            Float3 v;
            v[0] = message.float_value(0);
            v[1] = message.float_value(1);
            v[2] = message.float_value(2);
            set_param_value(*as_float3, v);
            return true;
          }
          break;
        }
        case 4:
        {
          ParamFloat4* as_float4;
          if (as_float4 << o) {
            Float4 v;
            v[0] = message.float_value(0);
            v[1] = message.float_value(1);
            v[2] = message.float_value(2);
            v[3] = message.float_value(3);
            set_param_value(*as_float4, v);
            return true;
          }
          break;
        }
        case 8:
        {
          ParamMatrix4* as_matrix4;
          if (as_matrix4 << o) {
            // Decomposed rigid-body transformation matrix
            const Quat rotation(message.float_value(0),
              message.float_value(1),
              message.float_value(2),
              message.float_value(3));
            const Vector3 position(message.float_value(4),
              message.float_value(5),
              message.float_value(6));
            const float scale(message.float_value(7));
            set_param_value(*as_matrix4, matrix4_extra::recompose(rotation, position, scale));
            return true;
          }
          break;
        }
        case 16:
        {
          ParamMatrix4* as_matrix4;
          if (as_matrix4 << o) {
            Matrix4 v;
            for (size_t col(0); col<4; ++col) {
              Vector4 w(message.float_value(col*4+0),
                message.float_value(col*4+1),
                message.float_value(col*4+2),
                message.float_value(col*4+3));
              v.setCol(col, w);
            }
            set_param_value(*as_matrix4, v);
            return true;
          }
          break;
        }
      }
      O3D_ERROR(mServiceLocator) << "Unsupported param of type " << o.GetClass()->name();
      return false;
    }
  }

  bool Receive(Effect& o) {
    binary::Effect message;
    if (!pbx::read(message, mStream)) {
      O3D_ERROR(mServiceLocator) << "Failed to parse an effect";
      return false;
    }

    if (message.has_source_indexed_string()) {
      if (message.source_indexed_string() >= mStringDB.db.size()) {
        O3D_ERROR(mServiceLocator) << "Effect source string out of range";
        return false;
      }
      if (!o.LoadFromFXString(mStringDB.db[message.source_indexed_string()])) {
        O3D_ERROR(mServiceLocator) << "Failed to load effect";
        return false;
      }
    }
    return true;
  }

  bool Receive(Skin& o) {

    binary::Skin message;
    if (!pbx::read(message, mStream)) {
      O3D_ERROR(mServiceLocator) << "Failed to parse a skin";
      return false;
    }

    if (message.inverse_bind_pose_matrice_size()%sizeof(Matrix4)) {
      O3D_ERROR(mServiceLocator) << "Corrupt data";
      return false;
    }
    const size_t num_matrices(message.inverse_bind_pose_matrice_size()/sizeof(Matrix4));

    const Matrix4* inverse_bind_pose_matrice((const Matrix4*) message.inverse_bind_pose_matrice().data());
    for (size_t i(0); i<num_matrices; ++i) {
      o.SetInverseBindPoseMatrix(i, inverse_bind_pose_matrice[i]);
    }

    Skin::Influences influences;
    for (size_t i(0); i<(size_t)message.influence_array_size(); ++i) {
      const binary::Skin::InfluenceArray& influence_array(message.influence_array(i));
      influences.clear();
      for (size_t j(0); j<(size_t)influence_array.influence_size(); ++j) {
        const binary::Skin::InfluenceArray::Influence& x(influence_array.influence(j));
        influences.push_back(Skin::Influence(x.matrix_index(), x.weight()));
      }
      o.SetVertexInfluences(i, influences);
    }
    return true;
  }

  bool Receive(Curve& o) {
    binary::Curve message;
    if (!pbx::read(message, mStream)) {
      O3D_ERROR(mServiceLocator) << "Failed to parse a curve";
      return false;
    }

    for (size_t i(0); i<(size_t)message.key_size(); ++i) {
      const binary::Curve::Key& k(message.key(i));
      const ObjectBase::Class* key_class(0);
      switch (k.type()) {
        case binary::Curve::Key::STEP:
          key_class = StepCurveKey::GetApparentClass();
          break;
        case binary::Curve::Key::LINEAR:
          key_class = LinearCurveKey::GetApparentClass();
          break;
        case binary::Curve::Key::BEZIER:
          key_class = BezierCurveKey::GetApparentClass();
          break;
        default:
          O3D_ERROR(mServiceLocator) << "Unsupported curve key type";
          return false;
      }
      CurveKey* key(o.CreateKeyByClass(key_class));
      if (!key) {
        O3D_ERROR(mServiceLocator) << "Failed to create a curve key";
        return false;
      }
      key->SetInput(k.input());
      key->SetOutput(k.output());
      BezierCurveKey* bezier;
      if (bezier <<* key) {
        if (k.bezier_tangent_size() != 4) {
          O3D_ERROR(mServiceLocator) << "Wrong number of tangents found for Bezier curve key";
          return false;
        }
        Float2 in(k.bezier_tangent(0), k.bezier_tangent(1));
        Float2 out(k.bezier_tangent(2), k.bezier_tangent(3));
        bezier->SetInTangent(in);
        bezier->SetOutTangent(out);
      }
    }
    if (!set_infinity(o, message.pre_infinity(), message.post_infinity())) {
      O3D_ERROR(mServiceLocator) << "Unsupported infinity";
      return false;
    }
    o.set_use_cache(message.use_cache());
    o.SetSampleRate(message.sample_rate());

    return true;
  }

  bool Receive(Buffer& o) {
    const bool is_an_index_buffer(is_a<IndexBuffer>(o));
    bool has_data(false);

    const FieldRefArray& fields(o.fields());
    while (!fields.empty())
      o.RemoveField(fields[fields.size()-1]);

    binary::Buffer message;
    if (!pbx::read(message, mStream)) {
      O3D_ERROR(mServiceLocator) << "Failed to parse a buffer";
      return false;
    }

    // First pass recreates the fields, but can't set their data yet
    // since we haven't allocated any memory for it yet‚Ä¶
    for (size_t i(0); i<(size_t)message.field_size(); ++i) {
      const binary::Buffer::Field& field_desc(message.field(i));
      binary::Buffer::Field::Type field_type(field_desc.type());

      #ifdef GLES2_BACKEND_NATIVE_GLES2
      // Index buffers are 16-bit
      if (is_an_index_buffer && (field_type==binary::Buffer::Field::UINT32))
        field_type = binary::Buffer::Field::UINT16;
      #else
      // UIn16Field exists only if GLES2_BACKEND_NATIVE_GLES2 is defined
      if (field_type == binary::Buffer::Field::UINT16)
        field_type = binary::Buffer::Field::UINT32;
      #endif

      Field* field(0);
      switch (field_type) {
        case binary::Buffer::Field::FLOAT:
          field = o.CreateTypedField<FloatField>(field_desc.num_components());
          has_data |= field_desc.value_float_size();
          break;
        case binary::Buffer::Field::UINT32:
          field = o.CreateTypedField<UInt32Field>(field_desc.num_components());
          has_data |= field_desc.value_uint_size();
          break;
        #ifdef GLES2_BACKEND_NATIVE_GLES2
        case binary::Buffer::Field::UINT16:
          field = o.CreateTypedField<UInt16Field>(field_desc.num_components());
          has_data |= field_desc.value_uint_size();
          break;
        #endif
        case binary::Buffer::Field::BYTE:
          field = o.CreateTypedField<UByteNField>(field_desc.num_components());
          has_data |= field_desc.has_value_byte();
          break;
        default:
          break;
      }

      if (!field) {
        O3D_ERROR(mServiceLocator) << "Failed to deserialize a field";
        return false;
      }
      if (field_desc.has_name()) {
        if (field_desc.name() >= mStringDB.db.size()) {
          O3D_ERROR(mServiceLocator) << "Field name out of range";
          return false;
        }
        field->set_name(mStringDB.db[field_desc.name()]);
      }
      mOldIdToNewObject[field_desc.id()] = ObjectBase::Ref(field);
    }

    // Allocate the memory
    if (!o.AllocateElements(message.num_elements())) {
      O3D_ERROR(mServiceLocator) << "Couldn't allocate " << message.num_elements() << " elements for buffer";
      return false;
    }

    // 2nd pass recreates the data (it any)
    if (has_data) {
      BufferLockHelper lock(&o);
      lock.GetData(Buffer::WRITE_ONLY);
      for (size_t i(0); i<(size_t)fields.size(); ++i) {
        const binary::Buffer::Field& field_desc(message.field(i));
        Field& field = *fields[i];
        do {
          FloatField* float_field;
          if (float_field << field) {
            if ((size_t)field_desc.value_float_size() != (size_t)field.num_components() * o.num_elements()) {
              O3D_ERROR(mServiceLocator) << "Field's data size mismatchs";
              return false;
            }
            float_field->SetFromFloats(field_desc.value_float().data(), field.num_components(), 0, o.num_elements());
            break;
          }
          UInt32Field* uint32_field;
          if (uint32_field << field) {
            if ((size_t)field_desc.value_uint_size() != (size_t)field.num_components() * o.num_elements()) {
              O3D_ERROR(mServiceLocator) << "Field's data size mismatchs";
              return false;
            }
            uint32_field->SetFromUInt32s(field_desc.value_uint().data(), field.num_components(), 0, o.num_elements());
            break;
          }
          UByteNField* ubyten_field;
          if (ubyten_field << field) {
            const std::string& value_byte(field_desc.value_byte());
            if (value_byte.size() != field.num_components() * o.num_elements()) {
              O3D_ERROR(mServiceLocator) << "Field's data size mismatchs";
              return false;
            }
            ubyten_field->SetFromUByteNs((const uint8_t*) &value_byte[0], field.num_components(), 0, o.num_elements());
            break;
          }
          #ifdef GLES2_BACKEND_NATIVE_GLES2
          UInt16Field* uint16_field;
          if (uint16_field << field) {
            if ((size_t)field_desc.value_uint_size() != (size_t)field.num_components() * o.num_elements()) {
              O3D_ERROR(mServiceLocator) << "Field's data size mismatchs";
              return false;
            }
            uint16_field->SetFromUInt32s(field_desc.value_uint().data(), field.num_components(), 0, o.num_elements());
            break;
          }
          #endif
          O3D_ERROR(mServiceLocator) << "Unknown Field type";
          return false;
        } while(false);
      }
    }
    return true;
  }

  template<typename T>
  bool ReceiveVertexSource(T& o) {
    binary::VertexSource message;
    if (!pbx::read(message, mStream)) {
      O3D_ERROR(mServiceLocator) << "Failed to parse a vertex source";
      return false;
    }

    for (size_t i(0); i<(size_t)message.stream_size(); ++i) {
      const binary::VertexSource::Stream& stream(message.stream(i));
      Stream::Semantic semantic;
      if (!set_semantic(semantic, stream.semantic())) {
        O3D_ERROR(mServiceLocator) << "Unsupported stream semantic";
        return false;
      }

      ObjectBase::Ref ref(GetObjectRef(stream.field_ref()));
      if (!ref) {
        O3D_ERROR(mServiceLocator) << "Couldn't find field reference. Missing dependency?";
        return false;
      }

      Field* field;
      if (field <<* ref)
        o.SetVertexStream(semantic, stream.semantic_index(), field, stream.start_index());
      else {
        O3D_ERROR(mServiceLocator) << "Field ref doesn't reference a Field object";
        return false;
      }

      if (stream.has_bind()) {
        ObjectBase::Ref ref2(GetObjectRef(stream.bind()));
        if (!ref2) {
          O3D_ERROR(mServiceLocator) << "Missing dependency for stream";
          return false;
        }
        VertexSource* source;
        if (source <<* ref2) {
          ParamVertexBufferStream* dst_param(o.GetVertexStreamParam(semantic, stream.semantic_index()));
          ParamVertexBufferStream* src_param(source->GetVertexStreamParam(semantic, stream.semantic_index()));
          if (!dst_param || !src_param) {
            O3D_ERROR(mServiceLocator) << "Stream param not found";
            return false;
          }
          dst_param->Bind(src_param);
        }
      }
    }
    return true;
  }

  bool Receive(VertexSource& o) {
    SkinEval* as_skin_eval;
    StreamBank* as_stream_bank;
    if (as_skin_eval << o) return ReceiveVertexSource(*as_skin_eval);
    else if (as_stream_bank << o) return ReceiveVertexSource(*as_stream_bank);
    else {
      O3D_ERROR(mServiceLocator) << "Unsupported vertex source type";
      return false;
    }
  }

  bool Receive(Primitive& o) {
    binary::Primitive message;
    if (!pbx::read(message, mStream)) {
      O3D_ERROR(mServiceLocator) << "Failed to parse a primitive";
      return false;
    }
    if (!set_primitive_type(o, message.primitive_type())) {
      O3D_ERROR(mServiceLocator) << "Unsupported primitive type";
      return false;
    }
    o.set_number_vertices(message.number_vertices());
    o.set_number_primitives(message.number_primitives());
    o.set_start_index(message.start_index());

    if (message.has_index_buffer_ref()) {
      ObjectBase::Ref ref(GetObjectRef(message.index_buffer_ref()));
      if (!ref) {
        O3D_ERROR(mServiceLocator) << "Couldn't find primitive's index buffer";
        return false;
      }
      IndexBuffer* index_buffer;
      if (index_buffer <<* ref) o.set_index_buffer(index_buffer);
      else {
        O3D_ERROR(mServiceLocator) << "Impostor posed as an IndexBuffer, but we didn't fall for it";
        return false;
      }
    }

    if (message.has_stream_bank_ref()) {
      ObjectBase::Ref ref(GetObjectRef(message.stream_bank_ref()));
      if (!ref) {
        O3D_ERROR(mServiceLocator) << "Couldn't find primitive's stream bank";
        return false;
      }
      StreamBank* stream_bank;
      if (stream_bank <<* ref) o.set_stream_bank(stream_bank);
      else {
        O3D_ERROR(mServiceLocator) << "Impostor posed as a StreamBank, but we didn't fall for it";
        return false;
      }
    }

    ObjectBase::Ref ref(GetObjectRef(message.owner_ref()));
    if (!ref) {
      O3D_ERROR(mServiceLocator) << "Couldn't find primitive's owner";
      return false;
    }

    Shape* shape;
    if (shape <<* ref) o.SetOwner(shape);
    else {
      O3D_ERROR(mServiceLocator) << "The object owning the primitive is not a Shape";
      return false;
    }

    return true;
  }

  bool Receive(Transform& o) {
    binary::Transform message;
    if (!pbx::read(message, mStream)) {
      O3D_ERROR(mServiceLocator) << "Failed to parse a transform";
      return false;
    }

    if (message.has_parent_ref()) {
      ObjectBase::Ref ref(GetObjectRef(message.parent_ref()));
      if (!ref) {
        O3D_ERROR(mServiceLocator) << "Cannot resolve Transform's parent ref. Missing dependency? [Ref was " << message.parent_ref() << "]";
        return false;
      }
      Transform* parent;
      if (parent <<* ref) o.SetParent(parent);
      else {
        O3D_ERROR(mServiceLocator) << "Transform has a parent, but it's not a Transform!";
        return false;
      }
    }
    return true;
  }

 private:
  IExternalResourceProvider& mERP;
  string_db_t mStringDB;
  std::tr1::unordered_map<std::string, const ObjectBase::Class*> mClassMap;
  std::tr1::unordered_map<uint32_t, ObjectBase::Ref> mOldIdToNewObject;
  Pack& mPack;
  pb::io::ZeroCopyInputStream& mStream;
  ServiceLocator* mServiceLocator;
};

} // anonymous namespace

Transform* LoadFromBinaryStream(std::istream& stream, Pack& pack, IExternalResourceProvider& erp) {
  pbx::log_handler lh;
  Transform* root(0);
  if (stream.good()) {
    stream.seekg (0, std::ios::beg);
    pb::io::IstreamInputStream low_level_stream(&stream);

    // Read FourCC
    bool magic_ok(false);
    do {
      pb::io::CodedInputStream tmp(&low_level_stream);
      uint32_t magic;
      if (!tmp.ReadLittleEndian32(&magic)) break;
      if (magic != FOURCC) break;
      magic_ok = true;
    } while(false);

    if (magic_ok) {
      binary::StreamHeader header;
      pb::io::ZeroCopyInputStream* decompressed_stream(0);
      if (pbx::read(header, low_level_stream)) {
        switch (header.compression()) {
        case binary::StreamHeader::COMPRESSION_NONE:
          decompressed_stream = &low_level_stream;
          break;
        case binary::StreamHeader::COMPRESSION_GZIP:
          decompressed_stream = new pb::io::GzipInputStream(&low_level_stream);
          break;
        case binary::StreamHeader::COMPRESSION_LZMA:
          decompressed_stream = new pb::io::LzmaInputStream(&low_level_stream, true);
          break;
        default:
          O3D_ASSERT(false);
        }

        if (decompressed_stream) {
          Load load(pack, *decompressed_stream, erp);
          root=load();

          if (decompressed_stream != &low_level_stream)
            delete decompressed_stream;
        }
      }
    }
    if (!root) stream.setstate(std::ios_base::failbit);
  }
  return root;
}

bool SaveToBinaryStream(std::ostream& stream, Transform& root, TCompressionAlgorithm compression) {
  pbx::log_handler lh;
  if (stream.good()) {
    pb::io::ZeroCopyOutputStream* low_level_stream(new pb::io::OstreamOutputStream(&stream));
    if (low_level_stream) {
      pb::io::ZeroCopyOutputStream* compressed_stream(0);

      binary::StreamHeader header;
      switch(compression) {
      case COMPRESSION_NONE:
        compressed_stream = low_level_stream;
        break;
      case COMPRESSION_GZIP:
        {
          header.set_compression(binary::StreamHeader::COMPRESSION_GZIP);
          pb::io::GzipOutputStream::Options options;
          options.compression_level = 9;
          compressed_stream = new pb::io::GzipOutputStream(low_level_stream, options);
        }
        break;
      case COMPRESSION_LZMA:
        {
          header.set_compression(binary::StreamHeader::COMPRESSION_LZMA);
          compressed_stream = new pb::io::LzmaOutputStream(low_level_stream);
        }
        break;
      default:
        O3D_ASSERT(false);
      }

      bool ok(false);
      if (compressed_stream) {
        // First, write header to uncompressed stream
        do {
          pb::io::CodedOutputStream header_stream(low_level_stream);
          header_stream.WriteLittleEndian32(FOURCC);
          ok = pbx::write(header, header_stream);
        } while(false);

        // Next, serialize our model in compressed stream
        if (ok) {
          pb::io::CodedOutputStream model_stream(compressed_stream);
          Publisher publish(model_stream);
          ok = publish(root);
        }

        if (compressed_stream != low_level_stream)
          delete compressed_stream;
      }

      delete low_level_stream;
      if (ok) return true;
    }
  }
  stream.setstate(std::ios_base::failbit);
  return false;
}

} // namespace extra
} // namespace o3d

/* vim: set sw=2 ts=2 sts=2 expandtab ff=unix: */
