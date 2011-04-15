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

#ifndef O3D_UTILS_SCENE_H_
#define O3D_UTILS_SCENE_H_

#include <string>
#include <set>
#include "core/cross/types.h"
#include "extra/cross/binary.h"

namespace o3d {

class Client;
class Element;
class Material;
class Pack;
class ParamFloat;
class Shape;
class Transform;

}  // namespace o3d.

namespace o3d_utils {

class ViewInfo;

class StringList {
 public:
  StringList(const char** strings, size_t num_strings);
  bool IsInList(const std::string& check);

 private:
  std::set<std::string> strings_;
};

// Manages a scene loaded from collada/o3d.
class Scene {
 public:
  ~Scene();

  // Material pack can be NULL
  // Parameters:
  //   client: The o3d client object.
  //   view_info: The view to use with material
  //   filename: The path to the file to load
  //   effect_texture_pack: The pack to get textures and effects from or to
  //      put new textures and effects into. Pass NULL to have textures and
  //      effects put in the pack that will get created for this scene.
  static Scene* LoadScene(
      o3d::Client* client,
      o3d_utils::ViewInfo* view_info,
      const std::string& filename,
      o3d::Pack* effect_texture_pack);

  static Scene* LoadBinaryScene(
      o3d::Client* client,
      o3d_utils::ViewInfo* view_info,
      const std::string& filename,
      o3d::extra::IExternalResourceProvider& external_resource_provider);

  static Scene* DummyScene(
      o3d::Client* client,
      o3d_utils::ViewInfo* view_info);

  o3d::Pack* pack() const {
    return pack_;
  }

  o3d::Transform* root() const {
    return root_;
  }

  o3d::ParamFloat* time_param() const {
    return time_;
  }

  // TODO(gman): Need to pass in effect/material pack :-(
  Scene* Clone(o3d::Client* client) const;

  void SetParent(o3d::Transform* parent);
  void SetLocalMatrix(const o3d::Matrix4& mat);
  void SetLocalMatrix(const float* mat);
  void SetAnimationTime(float timeInSeconds);

  static void AddMissingTexCoordStreams(o3d::Element* element);
  static void AddMissingTexCoordStreams(o3d::Shape* shape);
  static void SetBoundingBoxesAndZSortPoints(o3d::Shape* shape);
  static void PrepareShape(o3d::Pack* pack, o3d::Shape* shape);
  static void PrepareShapes(o3d::Pack* pack);

 private:
  friend class Cloner;

  Scene(o3d::Pack* pack, o3d::Transform* root, o3d::ParamFloat* time);

  o3d::Pack* pack_;
  o3d::Transform* root_;
  o3d::ParamFloat* time_;
};

}  // namespace o3d_utils

#endif  // O3D_UTILS_SCENE_H_

