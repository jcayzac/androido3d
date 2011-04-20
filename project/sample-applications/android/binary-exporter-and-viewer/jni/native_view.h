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

#pragma once

#include "core/cross/service_locator.h"
#include "core/cross/evaluation_counter.h"
#include "core/cross/client.h"
#include "core/cross/client_info.h"
#include "core/cross/class_manager.h"
#include "core/cross/display_window.h"
#include "core/cross/features.h"
#include "core/cross/object_manager.h"
#include "core/cross/profiler.h"
#include "core/cross/renderer.h"
#include "core/cross/renderer_platform.h"
#include "core/cross/skin.h"
#include "core/cross/timer.h"
#include "core/cross/transform.h"
#include "core/cross/types.h"
#include <extra/cross/binary.h>
#include <scene.h>
#include <camera.h>
#include <render_graph.h>

class NativeView: public o3d::extra::IExternalResourceProvider {
 public:
  NativeView(size_t width, size_t height);
  ~NativeView();
  void Render(bool animate=true);
  void OnResized(size_t width, size_t height);
  void OnContextRestored();
  bool LoadScene(const std::string& path);
  bool LoadBinaryScene(const std::string& path);
  bool ExportScene(const std::string& path);

 private:
  bool LoadScene(const std::string& path, bool is_binary);
  void SetCurrentScene(o3d_utils::Scene* newScene);

 private:
  // From o3d::extra::IExternalResourceProvider
  o3d::ExternalResource::Ref GetExternalResourceForURI(o3d::Pack& pack, const std::string& uri);

 private:
  o3d::ServiceLocator                           mServiceLocator;
  o3d::Renderer*                                mRenderer;
  o3d::base::scoped_ptr<o3d::EvaluationCounter> mEvaluationCounter;
  o3d::base::scoped_ptr<o3d::ClassManager>      mClassManager;
  o3d::base::scoped_ptr<o3d::ClientInfoManager> mClientInfoManager;
  o3d::base::scoped_ptr<o3d::ObjectManager>     mObjectManager;
  o3d::base::scoped_ptr<o3d::Profiler>          mProfiler;
  o3d::base::scoped_ptr<o3d::Features>          mFeatures;
  o3d::base::scoped_ptr<o3d::Client>            mClient;
  o3d::Pack*                                    mPack;
  o3d::Transform*                               mRoot;
  o3d_utils::Scene*                             mCurrentScene;
  o3d_utils::ViewInfo*                          mView;
  o3d::ElapsedTimeTimer                         mTimer;
  size_t                                        mWidth;
  size_t                                        mHeight;
  float                                         mAngleY;
};

/* vim: set sw=2 ts=2 sts=2 expandtab ff=unix: */
