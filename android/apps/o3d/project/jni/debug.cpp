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

#include <jni.h>
#include <android/log.h>
#include <string>

#define  LOG_TAG    "libo3djni"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#include "core/cross/buffer.h"
#include "core/cross/draw_element.h"
#include "core/cross/effect.h"
#include "core/cross/element.h"
#include "core/cross/material.h"
#include "core/cross/render_node.h"
#include "core/cross/sampler.h"
#include "core/cross/shape.h"
#include "core/cross/texture.h"
#include "core/cross/transform.h"

void DumpParams(const o3d::ParamObject* obj, const std::string& indent) {
  if (obj) {
    const o3d::NamedParamRefMap& params = obj->params();
    for (o3d::NamedParamRefMap::const_iterator it = params.begin();
         it != params.end();
         ++it) {
      o3d::Param* param = it->second;
      o3d::Param* input = param->input_connection();
      std::string value;
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
      }
      if (input) {
        LOGI("%s:Param: %s [%s] <- %s\n", indent.c_str(), param->name().c_str(),
             param->GetClass()->name(), input->name().c_str());
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
    LOGI("%sElement: %s\n", indent.c_str(), element->name().c_str());
    DumpParams(element, indent + "   ");
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
    const o3d::ElementRefArray& elements = shape->GetElementRefs();
    if (!elements.empty()) {
      std::string inner = indent + "    ";
      for (size_t ii = 0; ii < elements.size(); ++ii) {
        DumpElement(elements[ii], inner);
      }
    }
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



