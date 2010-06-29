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
#include "core/cross/render_node.h"

void DumpRenderGraph(o3d::RenderNode* render_node, const std::string& indent) {
  if (render_node) {
    LOGI("%s%s\n", indent.c_str(), render_node->GetClass()->name());
    const o3d::RenderNodeRefArray& children = render_node->children();
    if (!children.empty()) {
      std::string inner = indent + "    ";
      for (size_t ii = 0; ii < children.size(); ++ii) {
        DumpRenderGraph(children[ii], inner);
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



