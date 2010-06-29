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

// Some debugging functions.

#ifndef O3D_UTILS_DEBUG_H_
#define O3D_UTILS_DEBUG_H_

#include <string>
#include <vector>
#include <map>
#include "core/cross/types.h"

namespace o3d {

class RenderNode;

}  // namespace o3d

template <typename T>
class SortHelper {
 public:
  SortHelper(const std::vector<T*>& things) {
    for (size_t ii = 0; ii < things.size(); ++ii) {
      T* thing = things[ii];
      things_[thing->name()] = thing;
    }
  }

  size_t size() const {
    return things_.size();
  }

  T* operator[] (size_t ii) const {
    typename ThingMap::const_iterator it = things_.begin();
    while (ii) {
      --ii;
      ++it;
    }
    return it->second;
  }

 private:
  typedef std::map<std::string, T*> ThingMap;

  ThingMap things_;
};

void DumpRenderGraph(o3d::RenderNode* render_node, const std::string& indent);
void DumpMatrix(const o3d::Matrix4& mat);
bool EndsWith(const std::string& str, const std::string& end);

#endif  // O3D_UTILS_DEBUG_H_

