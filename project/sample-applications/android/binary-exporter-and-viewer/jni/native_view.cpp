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

#include <base/cross/log.h>
#include "native_view.h"
#include <android/log.h>
#include <math.h>
#include <string>
#include <sstream>
#include <fstream>
#include <debug.h>
#include <core/cross/file_resource.h>
#include <extra/cross/bounding_boxes_extra.h>
#define MAX_ANIM_DURATION 10.f

class fake_window_t: public o3d::DisplayWindow {
 public:
  ~fake_window_t() { }
};

NativeView::NativeView(size_t width, size_t height)
: mRenderer(o3d::Renderer::CreateDefaultRenderer(&mServiceLocator))
, mEvaluationCounter(new o3d::EvaluationCounter(&mServiceLocator))
, mClassManager(new o3d::ClassManager(&mServiceLocator))
, mClientInfoManager( new o3d::ClientInfoManager(&mServiceLocator))
, mObjectManager(new o3d::ObjectManager(&mServiceLocator))
, mProfiler(new o3d::Profiler(&mServiceLocator))
, mFeatures(new o3d::Features(&mServiceLocator))
, mClient(new o3d::Client(&mServiceLocator))
, mPack(0)
, mRoot(0)
, mCurrentScene(0)
, mView(0)
, mWidth(width)
, mHeight(height)
, mAngleY(.0f)
{
  O3D_LOG(INFO) << "Dimensions = " << width << "x" << height;
  mRenderer->Init(fake_window_t(), false);
  mClient->Init();
  mPack = mClient->CreatePack();
  mRoot = mPack->Create<o3d::Transform>();
  mView = o3d_utils::ViewInfo::CreateBasicView(mPack, mRoot, mClient->render_graph_root());
  mView->draw_context()->set_projection(o3d_utils::Camera::perspective(.5f, (float)mWidth/(float)mHeight, 10.f, 1000.f));
  mView->clear_buffer()->set_clear_color(o3d::Float4(.5f,1.f,.2f,.5f));
  mTimer.GetElapsedTimeAndReset();
}

NativeView::~NativeView() {
  SetCurrentScene(0);
}

void NativeView::Render(bool animate) {
  float elapsedTimeSinceLastUpdateInSeconds = mTimer.GetElapsedTimeAndReset();
  if (mCurrentScene) {
    if (animate && mCurrentScene->time_param()) {
      // we don't know how much time the animation last!
      // default to MAX_ANIM_DURATION
      float newTime = mCurrentScene->time_param()->value() + elapsedTimeSinceLastUpdateInSeconds;
      mCurrentScene->time_param()->set_value(fmodf(newTime, MAX_ANIM_DURATION));
    }
    mAngleY = fmodf(mAngleY + .02f, M_PI*2.f);
    mRoot->set_local_matrix(
      o3d::Matrix4::rotationY(mAngleY)
    );
  }

  o3d::Float4 color(0.12f,0.12f,0.12f,.0f);

  mView->clear_buffer()->set_clear_color(color);

  mClient->Tick();
  mClient->RenderClient(true);
}

void NativeView::OnResized(size_t width, size_t height) {
  mWidth  = width;
  mHeight = height;
  mRenderer->Resize(mWidth, mHeight);
  o3d::extra::updateBoundingBoxes(*mRoot);

  if (mCurrentScene) {
    o3d_utils::CameraInfo* camera_info = o3d_utils::Camera::getCameraFitToScene(mRoot, mWidth, mHeight);
    mView->draw_context()->set_view(camera_info->view);
    mView->draw_context()->set_projection(camera_info->projection);
    delete camera_info;
  }
}

void NativeView::OnContextRestored() {
  o3d::down_cast<o3d::RendererGLES2*>(mRenderer)->OnContextRestored();
}

bool NativeView::LoadScene(const std::string& path) {
  return LoadScene(path, false);
}

bool NativeView::LoadBinaryScene(const std::string& path) {
  return LoadScene(path, true);
}

bool NativeView::LoadScene(const std::string& path, bool binary) {
  o3d_utils::Scene* newScene = 0;
  if (binary) newScene = o3d_utils::Scene::LoadBinaryScene(&*mClient, mView, path, *this);
  else newScene = o3d_utils::Scene::LoadScene(&*mClient, mView, path, 0);
  if (!newScene) {
    return false;
  }

  SetCurrentScene(newScene);
  return true;
}

bool NativeView::ExportScene(const std::string& path) {
  if (!mCurrentScene) {
    O3D_LOG(ERROR) << "Nothing to export";
    return false;
  }
  std::ofstream ofs(path.c_str(), std::ios::binary|std::ios::trunc);
  if (!ofs.is_open()) {
    O3D_LOG(ERROR) << "Can't open [" << path << "] for writing";
    return false;
  }
  // Reset everything so that time-bound params are in a proper state
  o3d::extra::updateBoundingBoxes(*mRoot);
  mCurrentScene->time_param()->set_value(.0f);
  // Render (without animating), so that all params get reinitialized
  Render(false);

  const bool success(o3d::extra::SaveToBinaryStream(ofs, *mCurrentScene->root()));
  ofs.close();
  if (!success) {
    O3D_LOG(ERROR) << "Couldn't export the model";
  }
  return success;
}

void NativeView::SetCurrentScene(o3d_utils::Scene* newScene) {
  o3d::TransformArray children(mRoot->GetChildren());
  o3d::TransformArray::iterator end(children.end());
  for (o3d::TransformArray::iterator iter(children.begin()); iter != end; ++iter)
     (*iter)->SetParent(0);

  if (mCurrentScene) {
    mCurrentScene->SetParent(0);
    delete mCurrentScene;
  }

  mCurrentScene = newScene;

  if (mCurrentScene) {
    mCurrentScene->SetParent(mRoot);
    mTimer.GetElapsedTimeAndReset();

    o3d::extra::updateBoundingBoxes(*mRoot);

    o3d_utils::CameraInfo* camera_info = o3d_utils::Camera::getCameraFitToScene(mRoot, mWidth, mHeight);
    mView->draw_context()->set_view(camera_info->view);
    mView->draw_context()->set_projection(camera_info->projection);
    o3d::Vector3 light_pos = Vectormath::Aos::inverse(camera_info->view).getTranslation();
    std::vector<o3d::Material*> materials = mCurrentScene->pack()->GetByClass<o3d::Material>();
    for (size_t ii = 0; ii < materials.size(); ++ii) {
      o3d::ParamFloat3* pos = materials[ii]->GetParam<o3d::ParamFloat3>("lightWorldPos");
      if (pos) pos->set_value(o3d::Float3(light_pos[0], light_pos[1], light_pos[2]));
    }
    delete camera_info;
  }
}

o3d::ExternalResource::Ref NativeView::GetExternalResourceForURI(o3d::Pack& pack, const std::string& uri) {
  o3d::ExternalResource::Ref res(new o3d::FileResource(uri));
  if (!res->data())
    return o3d::ExternalResource::Ref();
  return res;
}

/////////////////////////////////////// JNI ////////////////////////////////////////

#include <jni.h>
#define JNIFUNC(x, y) extern "C" JNIEXPORT x JNICALL Java_com_tonchidot_O3DConditioner_NativeView_##y

JNIFUNC(jlong, createPeer) (JNIEnv* env, jclass, jlong width, jlong height) {
  NativeView* peer = new NativeView((size_t) width, (size_t) height);
  return (jlong) peer;
}

JNIFUNC(void, destroyPeer) (JNIEnv* env, jclass, jlong handle) {
  delete reinterpret_cast<NativeView*>(handle);
}

JNIFUNC(void, render) (JNIEnv* env, jclass, jlong handle) {
  reinterpret_cast<NativeView*>(handle)->Render();
}

JNIFUNC(void, onResized) (JNIEnv* env, jclass, jlong handle, jlong width, jlong height) {
  reinterpret_cast<NativeView*>(handle)->OnResized((size_t) width, (size_t) height);
}

JNIFUNC(void, onContextRestored) (JNIEnv* env, jclass, jlong handle) {
  reinterpret_cast<NativeView*>(handle)->OnContextRestored();
}

JNIFUNC(jboolean, loadScene) (JNIEnv* env, jclass, jlong handle, jstring path) {
  const char *path_utf8 = env->GetStringUTFChars(path, 0);
  const bool result(reinterpret_cast<NativeView*>(handle)->LoadScene(path_utf8));
  env->ReleaseStringUTFChars(path, path_utf8);
  return (jboolean) result;
}

JNIFUNC(jboolean, loadBinaryScene) (JNIEnv* env, jclass, jlong handle, jstring path) {
  const char *path_utf8 = env->GetStringUTFChars(path, 0);
  const bool result(reinterpret_cast<NativeView*>(handle)->LoadBinaryScene(path_utf8));
  env->ReleaseStringUTFChars(path, path_utf8);
  return (jboolean) result;
}

JNIFUNC(jboolean, exportScene) (JNIEnv* env, jclass, jlong handle, jstring path) {
  const char *path_utf8 = env->GetStringUTFChars(path, 0);
  const bool result(reinterpret_cast<NativeView*>(handle)->ExportScene(path_utf8));
  env->ReleaseStringUTFChars(path, path_utf8);
  return (jboolean) result;
}

/* vim: set sw=2 ts=2 sts=2 expandtab ff=unix: */
