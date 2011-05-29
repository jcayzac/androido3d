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

#include "build/build_config.h"

#ifdef __ANDROID__
#include <jni.h>
#include <android/log.h>

#define  LOG_TAG    "libo3djni"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#elif TARGET_OS_IPHONE
#include "iOS/iphoneo3d/log.h"
#endif

#include <string>


#include "core/cross/buffer.h"
#include "core/cross/draw_element.h"
#include "core/cross/effect.h"
#include "core/cross/element.h"
#include "core/cross/material.h"
#include "core/cross/param_array.h"
#include "core/cross/primitive.h"
#include "core/cross/render_node.h"
#include "core/cross/sampler.h"
#include "core/cross/shape.h"
#include "core/cross/skin.h"
#include "core/cross/texture.h"
#include "core/cross/transform.h"

void DumpMultiLineString(const std::string& str) {
  size_t pos = 0;
  int line_num = 1;
  for(;;) {
    size_t start = pos;
    pos = str.find_first_of('\n', pos);
    std::string line = str.substr(start, pos - start);
    DLOG(INFO) << line_num << ": " << line;
    if (pos == std::string::npos) {
      break;
    }
    ++pos;
    ++line_num;
  }
}

void DumpPoint3(const o3d::Point3& v, const char* label) {
  LOGI("%s: %.3f, %.3f, %.3f\n", label, v[0], v[1], v[2]);
}

void DumpVector3(const o3d::Vector3& v, const char* label) {
  LOGI("%s: %.3f, %.3f, %.3f\n", label, v[0], v[1], v[2]);
}

void DumpFloat3(const o3d::Float3& v, const char* label) {
  LOGI("%s: %.3f, %.3f, %.3f\n", label, v[0], v[1], v[2]);
}

void DumpVector4(const o3d::Vector4& v, const char* label) {
  LOGI("%s: %.3f, %.3f, %.3f, %.3f\n", label, v[0], v[1], v[2], v[3]);
}

void DumpFloat4(const o3d::Float4& v, const char* label) {
  LOGI("%s: %.3f, %.3f, %.3f, %.3f\n", label, v[0], v[1], v[2], v[3]);
}

void DumpParams(const o3d::ParamObject* obj, const std::string& indent) {
  if (obj) {
    const o3d::NamedParamRefMap& params = obj->params();
    for (o3d::NamedParamRefMap::const_iterator it = params.begin();
         it != params.end();
         ++it) {
      o3d::Param* param = it->second;
      o3d::Param* input = param->input_connection();
      std::string value = "--na--";
      char buf[256];
      if (param->IsA(o3d::ParamFloat::GetApparentClass())) {
        float v = static_cast<o3d::ParamFloat*>(param)->value();
        sprintf(buf, "%.3f", v);
        value = buf;
      } else if (param->IsA(o3d::ParamFloat2::GetApparentClass())) {
        o3d::Float2 v = static_cast<o3d::ParamFloat2*>(param)->value();
        sprintf(buf, "%.3f, %.3f", v[0], v[1]);
        value = buf;
      } else if (param->IsA(o3d::ParamFloat3::GetApparentClass())) {
        o3d::Float3 v = static_cast<o3d::ParamFloat3*>(param)->value();
        sprintf(buf, "%.3f, %.3f, %.3f", v[0], v[1], v[2]);
        value = buf;
      } else if (param->IsA(o3d::ParamFloat4::GetApparentClass())) {
        o3d::Float4 v = static_cast<o3d::ParamFloat4*>(param)->value();
        sprintf(buf, "%.3f, %.3f, %.3f, %.3f", v[0], v[1], v[2], v[3]);
        value = buf;
      } else if (param->IsA(o3d::ParamMatrix4::GetApparentClass())) {
        o3d::Matrix4 v = static_cast<o3d::ParamMatrix4*>(param)->value();
        value = "";
        for (size_t ii = 0; ii < 4; ++ii) {
          sprintf(buf, "[%.3f, %.3f, %.3f, %.3f]",
                  v[ii][0], v[ii][1], v[ii][2], v[ii][3]);
          value += buf;
        }
      } else if (param->IsA(o3d::ParamBoundingBox::GetApparentClass())) {
        o3d::BoundingBox v =
            static_cast<o3d::ParamBoundingBox*>(param)->value();
        sprintf(buf, "%s [%.3f, %.3f, %.3f], [%.3f, %.3f, %.3f]",
                v.valid() ? "valid" : "**invalid**",
                v.min_extent()[0], v.min_extent()[1],
                v.min_extent()[2],v.max_extent()[0], v.max_extent()[1], v.max_extent()[2]);
        value = buf;
      } else if (param->IsA(o3d::ParamInteger::GetApparentClass())) {
        int v = static_cast<o3d::ParamInteger*>(param)->value();
        sprintf(buf, "%d", v);
        value = buf;
      } else if (param->IsA(o3d::ParamBoolean::GetApparentClass())) {
        bool v = static_cast<o3d::ParamBoolean*>(param)->value();
        value = v ? "true" : "false";
      } else if (param->IsA(o3d::ParamString::GetApparentClass())) {
        value = static_cast<o3d::ParamString*>(param)->value();
      } else if (param->IsA(o3d::ParamMaterial::GetApparentClass())) {
        o3d::Material* v = static_cast<o3d::ParamMaterial*>(param)->value();
        value = v ? v->name() : "NULL";
      } else if (param->IsA(o3d::ParamEffect::GetApparentClass())) {
        o3d::Effect* v = static_cast<o3d::ParamEffect*>(param)->value();
        value = v ? v->name() : "NULL";
      } else if (param->IsA(o3d::ParamTexture::GetApparentClass())) {
        o3d::Texture* v = static_cast<o3d::ParamTexture*>(param)->value();
        value = v ? v->name() : "NULL";
      } else if (param->IsA(o3d::ParamSampler::GetApparentClass())) {
        o3d::Sampler* v = static_cast<o3d::ParamSampler*>(param)->value();
        value = v ? v->name() : "NULL";
      } else if (param->IsA(o3d::ParamSkin::GetApparentClass())) {
        o3d::Skin* v = static_cast<o3d::ParamSkin*>(param)->value();
        value = v ? v->name() : "NULL";
      } else if (param->IsA(o3d::ParamStreamBank::GetApparentClass())) {
        o3d::StreamBank* v = static_cast<o3d::ParamStreamBank*>(param)->value();
        value = v ? v->name() : "NULL";
      } else if (param->IsA(o3d::ParamParamArray::GetApparentClass())) {
        o3d::ParamArray* v = static_cast<o3d::ParamParamArray*>(param)->value();
        value = v ? v->name() : "NULL";
      }
      if (input) {
        LOGI("%s:Param: %s [%s] <- %s.%s\n", indent.c_str(),
             param->name().c_str(), param->GetClass()->name(),
             input->owner()->name().c_str(), input->name().c_str());
      } else {
        LOGI("%s:Param: %s [%s] = %s\n", indent.c_str(), param->name().c_str(),
             param->GetClass()->name(), value.c_str());
      }
    }
  }
}

void DumpRenderNode(
    const o3d::RenderNode* render_node, const std::string& indent) {
  if (render_node) {
    LOGI("%s%s\n", indent.c_str(), render_node->GetClass()->name());
    DumpParams(render_node, indent + "   ");
    const o3d::RenderNodeRefArray& children = render_node->children();
    if (!children.empty()) {
      std::string inner = indent + "    ";
      for (size_t ii = 0; ii < children.size(); ++ii) {
        DumpRenderNode(children[ii], inner);
      }
    }
  }
}

void DumpDrawElement(
    const o3d::DrawElement* drawelement, const std::string& indent) {
  if (drawelement) {
    LOGI("%sDrawElement: %s\n", indent.c_str(), drawelement->name().c_str());
    DumpParams(drawelement, indent + "   ");
  }
}

void DumpElement(const o3d::Element* element, const std::string& indent) {
  if (element) {
    const char* pre = indent.c_str();
    LOGI("%sElement: %s\n", indent.c_str(), element->name().c_str());
    DumpParams(element, indent + "   ");
    if (element->IsA(o3d::Primitive::GetApparentClass())) {
      const o3d::Primitive* prim = down_cast<const o3d::Primitive*>(element);
      LOGI("%s  num_primitives: %d\n", pre, prim->number_primitives());
      LOGI("%s  num_vertices: %d\n", pre, prim->number_vertices());
      LOGI("%s  prim_type: %d\n", pre, prim->primitive_type());
      LOGI("%s  start index: %d\n", pre, prim->start_index());
      LOGI("%s  indexbuffer: %s\n", pre, prim->index_buffer()->name().c_str());
      o3d::StreamBank* sb = prim->stream_bank();
      const o3d::StreamParamVector& params = sb->vertex_stream_params();
      for (size_t jj = 0; jj < params.size(); ++jj) {
        const o3d::ParamVertexBufferStream* param = params[jj];
        const o3d::Stream& stream = param->stream();
        const o3d::Field& field = stream.field();
        const o3d::Buffer* buffer = field.buffer();
        LOGI("%s    stream: s:%d si:%d start:%d numv:%d buf:%s:%s\n",
             pre, stream.semantic(),
             stream.semantic_index(), stream.start_index(),
             stream.GetMaxVertices(),
             buffer->name().c_str(),
             buffer->GetClass()->name());
        unsigned num = std::min(buffer->num_elements(), 5u);
        float floats[5 * 4];
        field.GetAsFloats(0, floats, field.num_components(), num);
        for (unsigned elem = 0; elem < num; ++elem) {
          float* v = &floats[elem * field.num_components()];
          switch (field.num_components()) {
          case 1:
            LOGI("%s     %d: %.3f\n", pre, elem, v[0]);
            break;
          case 2:
            LOGI("%s     %d: %.3f, %.3f\n", pre, elem, v[0], v[1]);
            break;
          case 3:
            LOGI("%s     %d: %.3f, %.3f, %.3f\n", pre, elem, v[0], v[1], v[2]);
            break;
          case 4:
            LOGI("%s     %d: %.3f, %.3f, %.3f, %.3f\n", pre, elem,
                 v[0], v[1], v[2], v[3]);
            break;
          }
        }

        const o3d::Param* input = param->input_connection();
        if (input) {
          const o3d::ParamObject* owner = input->owner();
          LOGI("%s      input: %s:%s:%s:%s\n", pre,
               owner->name().c_str(), owner->GetClass()->name(),
               input->name().c_str(), input->GetClass()->name());
          if (owner->IsA(o3d::SkinEval::GetApparentClass())) {
            const o3d::SkinEval* se = down_cast<const o3d::SkinEval*>(owner);
            LOGI("%s        se skin: %s\n", pre, se->skin()->name().c_str());
            LOGI("%s        se mats: %s\n", pre,se->matrices()->name().c_str());
            const o3d::ParamArray* pa = se->matrices();
            LOGI("%s        pa size: %d\n", pre, pa->size());
            for (size_t pp = 0; pp < pa->size(); ++pp) {
              o3d::Param* mp = pa->GetUntypedParam(pp);
              o3d::Param* inp = mp->input_connection();
              LOGI("%s        %d: <- %s:%s\n",
                   pre, pp, inp ? inp->owner()->name().c_str() : "-",
                   inp ? inp->name().c_str() : "-");
            }
            LOGI("%s      -skineval-\n", pre);
            DumpParams(se, indent + "    ");
          }
        }
      }
    }
    const o3d::DrawElementRefArray& draw_elements =
        element->GetDrawElementRefs();
    if (!draw_elements.empty()) {
      std::string inner = indent + "    ";
      for (size_t ii = 0; ii < draw_elements.size(); ++ii) {
        DumpDrawElement(draw_elements[ii], inner);
      }
    }
  }
}

void DumpShape(const o3d::Shape* shape, const std::string& indent) {
  if (shape) {
    LOGI("%sShape: %s\n", indent.c_str(), shape->name().c_str());
//    const o3d::ElementRefArray& elements = shape->GetElementRefs();
//    if (!elements.empty()) {
//      std::string inner = indent + "    ";
//      for (size_t ii = 0; ii < elements.size(); ++ii) {
//        DumpElement(elements[ii], inner);
//      }
//    }
  }
}

void DumpTransform(const o3d::Transform* transform, const std::string& indent) {
  if (transform) {
    LOGI("%sTransform: %s\n", indent.c_str(), transform->name().c_str());
    DumpParams(transform, indent + "    ");
    const o3d::TransformRefArray& children = transform->GetChildrenRefs();
    if (!children.empty()) {
      std::string inner = indent + "    ";
      for (size_t ii = 0; ii < children.size(); ++ii) {
        DumpTransform(children[ii], inner);
      }
    }
    const o3d::ShapeRefArray& shapes = transform->GetShapeRefs();
    if (!shapes.empty()) {
      std::string inner = indent + "    ";
      for (size_t ii = 0; ii < shapes.size(); ++ii) {
        DumpShape(shapes[ii], inner);
      }
    }
  }
}

void DumpMatrix(const o3d::Matrix4& mat) {
  for (int ii = 0; ii < 4; ++ii) {
    LOGI("   %.3f, %.3f, %.3f, %.3f\n",
         mat[ii][0], mat[ii][1], mat[ii][2], mat[ii][3]);
  }
}

bool EndsWith(const std::string& str, const std::string& end) {
  return str.size() >= end.size() &&
         str.substr(str.size() - end.size()).compare(end) == 0;
}



