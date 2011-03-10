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

#include "native_view.h"
#include <android/log.h>
#include <math.h>
#include <string>
#include <sstream>
#include <fstream>
#include <debug.h>
#include <extra/cross/file_resource.h>
#include <extra/cross/bounding_boxes_extra.h>
#define MAX_ANIM_DURATION 10.f

class fake_window_t: public o3d::DisplayWindow {
public:
  ~fake_window_t() { }
};

class nano_logger_t {
 public:
  nano_logger_t(android_LogPriority pri): mPriority(pri) { }
  ~nano_logger_t() {
    __android_log_print(mPriority, "O3DConditioner", mStream.str().c_str());
  }
  std::stringstream& operator()() { return mStream; }
 private:
  android_LogPriority mPriority;
  std::stringstream   mStream;
};
#define LOG_XXX(x, xxx) { nano_logger_t log(xxx); log() << "[" << __FILE__ << ":" << __LINE__ << "] (" << __PRETTY_FUNCTION__ << ")\n" << x; }
#define LOG_ERROR(x) LOG_XXX(x, ANDROID_LOG_ERROR)
#define LOG_INFO(x)  LOG_XXX(x, ANDROID_LOG_INFO)

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
  LOG_INFO("Dimensions = " << width << "x" << height);
  mRenderer->Init(fake_window_t(), false);
  mClient->Init();
  mPack = mClient->CreatePack();
  mRoot = mPack->Create<o3d::Transform>();
  mView = o3d_utils::ViewInfo::CreateBasicView(mPack, mRoot, mClient->render_graph_root());
  mView->draw_context()->set_projection(o3d_utils::Camera::perspective(.5f, (float)mWidth/(float)mHeight, 10.f, 1000.f));
  mView->clear_buffer()->set_clear_color(o3d::Float4(.5f,1.f,.2f,.5f));
  mTimer.GetElapsedTimeAndReset();
/*
  o3d::State* state(mView->z_ordered_draw_pass_info()->state());
  state->GetStateParam<o3d::ParamBoolean>(o3d::State::kAlphaBlendEnableParamName)->set_value(true);
  state->GetStateParam<o3d::ParamBoolean>(o3d::State::kDitherEnableParamName)->set_value(true);

  state = mView->performance_draw_pass_info()->state();
  state->GetStateParam<o3d::ParamBoolean>(o3d::State::kDitherEnableParamName)->set_value(true);
  state->GetStateParam<o3d::ParamInteger>(o3d::State::kCullModeParamName)->set_value(o3d::State::CULL_CW);
  
  state->GetStateParam<o3d::ParamBoolean>(o3d::State::kZEnableParamName)->set_value(true);
  state->GetStateParam<o3d::ParamBoolean>(o3d::State::kZWriteEnableParamName)->set_value(true);
  state->GetStateParam<o3d::ParamInteger>(o3d::State::kZComparisonFunctionParamName)->set_value(o3d::State::CMP_LESS);
*/
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
  LOG_INFO("Entering");
  LOG_INFO("Dimensions = " << width << "x" << height);
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

  LOG_INFO("Leaving");
}

void NativeView::OnContextRestored() {
  LOG_INFO("Entering");
  down_cast<o3d::RendererGLES2*>(mRenderer)->OnContextRestored();
  LOG_INFO("Leaving");
}

bool NativeView::LoadScene(const std::string& path) {
  return LoadScene(path, false);
}

bool NativeView::LoadBinaryScene(const std::string& path) {
  return LoadScene(path, true);
}

bool NativeView::LoadScene(const std::string& path, bool binary) {
  LOG_INFO("Entering");
  LOG_INFO("Path = " << path);
  o3d_utils::Scene* newScene = 0;
  if (binary) newScene = o3d_utils::Scene::LoadBinaryScene(&*mClient, mView, path, *this);
  else newScene = o3d_utils::Scene::LoadScene(&*mClient, mView, path, 0);
  if (!newScene) {
    LOG_INFO("Leaving");
    return false;
  }

  SetCurrentScene(newScene);
  return true;
}

bool NativeView::ExportScene(const std::string& path) {
  LOG_INFO("Entering");
  LOG_INFO("Path = " << path);
  if (!mCurrentScene) {
    LOG_ERROR("Nothing to export");
    return false;
  }
  std::ofstream ofs(path.c_str(), std::ios::binary|std::ios::trunc);
  if (!ofs.is_open()) {
    LOG_ERROR("Can't open [" << path << "] for writing");
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
    LOG_ERROR("Couldn't export the model");
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

o3d::extra::ExternalResource::Ref NativeView::GetExternalResourceForURI(o3d::Pack& pack, const std::string& uri) {
  LOG_INFO("Requesting resource for URI = [" << uri << "]");
  o3d::extra::ExternalResource::Ref res(new o3d::extra::FileResource(uri));
  if (!res->data())
    return o3d::extra::ExternalResource::Ref();
  return res;
}

/////////////////////////////////////// JNI ////////////////////////////////////////

#include <jni.h>
#define JNIFUNC(x, y) extern "C" JNIEXPORT x JNICALL Java_com_tonchidot_O3DConditioner_NativeView_##y

JNIFUNC(jlong, createPeer) (JNIEnv* env, jclass, jlong width, jlong height) {
  LOG_INFO("Entering");
  NativeView* peer = new NativeView((size_t) width, (size_t) height);
  LOG_INFO("Leaving");
  return (jlong) peer;
}

JNIFUNC(void, destroyPeer) (JNIEnv* env, jclass, jlong handle) {
  LOG_INFO("Entering");
  delete reinterpret_cast<NativeView*>(handle);
  LOG_INFO("Leaving");
}

JNIFUNC(void, render) (JNIEnv* env, jclass, jlong handle) {
  reinterpret_cast<NativeView*>(handle)->Render();
}

JNIFUNC(void, onResized) (JNIEnv* env, jclass, jlong handle, jlong width, jlong height) {
  LOG_INFO("Entering");
  reinterpret_cast<NativeView*>(handle)->OnResized((size_t) width, (size_t) height);
  LOG_INFO("Leaving");
}

JNIFUNC(void, onContextRestored) (JNIEnv* env, jclass, jlong handle) {
  LOG_INFO("Entering");
  reinterpret_cast<NativeView*>(handle)->OnContextRestored();
  LOG_INFO("Leaving");
}

JNIFUNC(jboolean, loadScene) (JNIEnv* env, jclass, jlong handle, jstring path) {
  LOG_INFO("Entering");
  const char *path_utf8 = env->GetStringUTFChars(path, 0);
  const bool result(reinterpret_cast<NativeView*>(handle)->LoadScene(path_utf8));
  env->ReleaseStringUTFChars(path, path_utf8);
  LOG_INFO("Leaving (" << result << ")");
  return (jboolean) result;
}

JNIFUNC(jboolean, loadBinaryScene) (JNIEnv* env, jclass, jlong handle, jstring path) {
  LOG_INFO("Entering");
  const char *path_utf8 = env->GetStringUTFChars(path, 0);
  const bool result(reinterpret_cast<NativeView*>(handle)->LoadBinaryScene(path_utf8));
  env->ReleaseStringUTFChars(path, path_utf8);
  LOG_INFO("Leaving (" << result << ")");
  return (jboolean) result;
}

JNIFUNC(jboolean, exportScene) (JNIEnv* env, jclass, jlong handle, jstring path) {
  LOG_INFO("Entering");
  const char *path_utf8 = env->GetStringUTFChars(path, 0);
  const bool result(reinterpret_cast<NativeView*>(handle)->ExportScene(path_utf8));
  env->ReleaseStringUTFChars(path, path_utf8);
  LOG_INFO("Leaving (" << result << ")");
  return (jboolean) result;
}

/* vim: set sw=2 ts=2 sts=2 expandtab ff=unix: */
