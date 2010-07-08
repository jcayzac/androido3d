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

// OpenGL ES 2.0 code

#include <jni.h>
#include <android/log.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "base/logging.h"
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

#include "render_graph.h"
#include "camera.h"
#include "debug.h"
#include "scene.h"

#define  LOG_TAG    "libo3djni"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

class DisplayWindowAndroid : public o3d::DisplayWindow {
 public:
  ~DisplayWindowAndroid() { }
};

class O3DManager {
 public:
  O3DManager();

  o3d::Client* client() const {
    return client_.get();
  }

  o3d_utils::ViewInfo* main_view() const {
    return main_view_;
  }

  bool Initialize(int width, int height);
  bool ResizeViewport(int width, int height);
  void SetProjection(float width, float height);
  bool Render();
  bool OnContextRestored();
  o3d::Transform* GetRoot();
  o3d_utils::Scene* GetScene();
  void CheckError();

 private:
  DisplayWindowAndroid display_window_;
  o3d::ServiceLocator service_locator_;
  o3d::Renderer* renderer_;
  scoped_ptr<o3d::EvaluationCounter> evaluation_counter_;
  scoped_ptr<o3d::ClassManager> class_manager_;
  scoped_ptr<o3d::ClientInfoManager> client_info_manager_;
  scoped_ptr<o3d::ObjectManager> object_manager_;
  scoped_ptr<o3d::Profiler> profiler_;
  scoped_ptr<o3d::Features> features_;
  scoped_ptr<o3d::Client> client_;

  o3d::Transform* root_;
  o3d::Pack* pack_;
  o3d_utils::ViewInfo* main_view_;
  o3d_utils::Scene* scene_;
  o3d::ElapsedTimeTimer timer_;
  float time_;
};

O3DManager::O3DManager()
    : root_(NULL),
      pack_(NULL),
      main_view_(NULL),
      scene_(NULL),
      time_(0.0f) {
}

bool O3DManager::Initialize(int width, int height) {
  evaluation_counter_.reset(new o3d::EvaluationCounter(&service_locator_));
  class_manager_.reset(new o3d::ClassManager(&service_locator_));
  client_info_manager_.reset(new o3d::ClientInfoManager(&service_locator_));
  object_manager_.reset(new o3d::ObjectManager(&service_locator_));
  profiler_.reset(new o3d::Profiler(&service_locator_));
  features_.reset(new o3d::Features(&service_locator_));

  // create a renderer device based on the current platform
  renderer_ = o3d::Renderer::CreateDefaultRenderer(&service_locator_);

  LOGI("-----------------------------HERE1\n");

  if (renderer_->Init(display_window_, false) != o3d::Renderer::SUCCESS) {
    DLOG(ERROR) << "Window initialization failed!";
    return false;
  }

  LOGI("-----------------------------HERE2\n");

  client_.reset(new o3d::Client(&service_locator_));
  client_->Init();
  pack_ = client_->CreatePack();
  root_ = pack_->Create<o3d::Transform>();
  main_view_ = o3d_utils::ViewInfo::CreateBasicView(
      pack_, root_, client_->render_graph_root());

  LOGI("---Render Graph---(start)---\n");
  DumpRenderNode(client_->render_graph_root(), "");
  LOGI("---Render Graph---(end)---\n");

  scene_ = o3d_utils::Scene::LoadScene(
      client_.get(),
      main_view_,
      "/sdcard/collada/character.zip",
      NULL);
  scene_->SetParent(root_);

  o3d_utils::CameraInfo* camera_info =
      o3d_utils::Camera::getViewAndProjectionFromCameras(
          root_, width, height);
  main_view_->draw_context()->set_view(camera_info->view);

  SetProjection(width, height);

  // Set the light pos on all the materials. We could link a param to
  // all of them so we could just set the one param but I'm lazy.
  o3d::Vector3 light_pos =
      Vectormath::Aos::inverse(camera_info->view).getTranslation();
  std::vector<o3d::Material*> materials =
      scene_->pack()->GetByClass<o3d::Material>();
  for (size_t ii = 0; ii < materials.size(); ++ii) {
    o3d::ParamFloat3* pos =
        materials[ii]->GetParam<o3d::ParamFloat3>("lightWorldPos");
    if (pos) {
      pos->set_value(o3d::Float3(light_pos[0], light_pos[1], light_pos[2]));
    }
  }

  timer_.GetElapsedTimeAndReset();

  CheckError();

  return true;
}

bool O3DManager::OnContextRestored() {
  // Reset the timer so we don't have some giant time slice.
  timer_.GetElapsedTimeAndReset();

  // Restore the resources.
  return down_cast<o3d::RendererGLES2*>(renderer_)->OnContextRestored();
}

void O3DManager::SetProjection(float width, float height) {
  main_view_->draw_context()->set_projection(
      o3d_utils::Camera::perspective(
          o3d_utils::degToRad(30.0f), width / height, 10, 1000));
}

bool O3DManager::ResizeViewport(int width, int height) {
  renderer_->Resize(width, height);

  o3d_utils::CameraInfo* camera_info =
      o3d_utils::Camera::getViewAndProjectionFromCameras(
          root_, width, height);

  main_view_->draw_context()->set_view(camera_info->view);
  SetProjection(width, height);
}

o3d::Matrix4 lookAt2(
    const o3d::Point3& eye,
    const o3d::Point3& target,
    const o3d::Vector3& up) {
  o3d::Vector4 vz(Vectormath::Aos::normalize(eye - target));
  o3d::Vector4 vx(Vectormath::Aos::cross(up, vz.getXYZ()));
  o3d::Vector4 vy(Vectormath::Aos::cross(vz.getXYZ(), vx.getXYZ()));
  return o3d::Matrix4(vx, vy, vz, o3d::Vector4(eye));
}

bool O3DManager::Render() {
  float elapsedTimeSinceLastUpdateInSeconds = timer_.GetElapsedTimeAndReset();
  time_ += elapsedTimeSinceLastUpdateInSeconds;

  // Data for animations in 30hz frames. O3D uses seconds so divide by 30.
  //
  // idle1: {startFrame: 0, endFrame: 30},
  // walk: {startFrame: 31, endFrame: 71},
  // jumpStart: {startFrame: 72, endFrame: 87},
  // jumpUp: {startFrame: 87, endFrame: 87},
  // jumpCrest: {startFrame: 87, endFrame: 91},
  // jumpFall: {startFrame: 91, endFrame: 91},
  // jumpLand: {startFrame: 91, endFrame: 110},
  // run: {startFrame: 111, endFrame: 127},
  // idle2: {startFrame: 128, endFrame: 173},
  // idle3: {startFrame: 174, endFrame: 246},
  // idle4: {startFrame: 247, endFrame: 573}};
  static const float kWalkStart = 31.0f / 30.0f;
  static const float kWalkEnd = 71.0f / 30.0f;
  static const float kWalkDuration = kWalkEnd - kWalkStart;
  static const float kIdleStart = 247.0f / 30.0f;
  static const float kIdleEnd = 573.0f / 30.0f;
  static const float kIdleDuration = kIdleEnd - kIdleStart;

  static float moveTimer = 0.0f;
  static float animTimer = 0.0f;
  static bool idleing = false;

  animTimer += elapsedTimeSinceLastUpdateInSeconds;
  if (!idleing) {
    moveTimer += elapsedTimeSinceLastUpdateInSeconds * 0.5;

    // Move and animate the character
    // Move in a circle
    const float kMoveRadius = 10.0f;
    o3d::Point3 position(
        sinf(moveTimer) * kMoveRadius,
        0.0f,
        cosf(moveTimer) * kMoveRadius);
    o3d::Point3 target(
        sinf(moveTimer - 0.1f) * kMoveRadius,
        0.0f,
        cosf(moveTimer - 0.1f) * kMoveRadius);
    o3d::Vector3 up(0.0f, 1.0f, 0.0f);

    o3d::Matrix4 mat(lookAt2(position, target, up));
    scene_->root()->set_local_matrix(mat);

    scene_->SetAnimationTime(kWalkStart + fmodf(animTimer, kWalkDuration));
    if (animTimer >= kWalkDuration * 4) {
      animTimer = 0.0f;
      idleing = true;
    }
  } else {
    scene_->SetAnimationTime(kIdleStart + std::min(animTimer, kIdleDuration));
    if (animTimer >= kIdleDuration) {
      animTimer = 0.0f;
      idleing = false;
    }
  }

  client_->Tick();
  client_->RenderClient(true);
  CheckError();
}

o3d::Transform* O3DManager::GetRoot() {
  return root_;
}

o3d_utils::Scene* O3DManager::GetScene() {
  return scene_;
}

void O3DManager::CheckError() {
  const std::string& error = client_->GetLastError();
  if (!error.empty()) {
    LOGI("================O3D ERROR====================\n%s", error.c_str());
    client_->ClearLastError();
  }
};

static O3DManager* g_mgr = NULL;

extern "C" {
    JNIEXPORT void JNICALL Java_com_android_o3djni_O3DJNILib_init(JNIEnv * env, jobject obj,  jint width, jint height);
    JNIEXPORT void JNICALL Java_com_android_o3djni_O3DJNILib_surfaceCreated(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_android_o3djni_O3DJNILib_step(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_android_o3djni_O3DJNILib_onKeyDown(JNIEnv * env, jobject obj,  jint keycode);
    JNIEXPORT void JNICALL Java_com_android_o3djni_O3DJNILib_onKeyUp(JNIEnv * env, jobject obj,  jint keycode);
    JNIEXPORT void JNICALL Java_com_android_o3djni_O3DJNILib_onTouch(JNIEnv * env, jobject obj,
        jint x, jint y, jfloat directionX, jfloat directionY);
    JNIEXPORT void JNICALL Java_com_android_o3djni_O3DJNILib_onRoll(JNIEnv * env, jobject obj,
    		jfloat directionX, jfloat directionY);
};

JNIEXPORT void JNICALL Java_com_android_o3djni_O3DJNILib_init(JNIEnv * env, jobject obj,  jint width, jint height) {
  LOGI("init %dx%d", width, height);
  glViewport(0, 0, width, height);

  if (g_mgr != NULL) {
    g_mgr->ResizeViewport(width, height);
  } else {
    g_mgr = new O3DManager();
    g_mgr->Initialize(width, height);
  }
}

JNIEXPORT void JNICALL Java_com_android_o3djni_O3DJNILib_surfaceCreated(JNIEnv * env, jobject obj) {
  // Called when the EGL surface is created, such as on boot and after a context loss.
  LOGI("surfaceCreated\n");
  if (g_mgr) {
    LOGI("Restoring Resources\n");
    if (!g_mgr->OnContextRestored()) {
      LOGI("Failed to restore resources");
      g_mgr->CheckError();
    }
  }
}

JNIEXPORT void JNICALL Java_com_android_o3djni_O3DJNILib_step(JNIEnv * env, jobject obj) {
  g_mgr->Render();
}

JNIEXPORT void JNICALL Java_com_android_o3djni_O3DJNILib_onKeyDown(JNIEnv * env, jobject obj,  jint keycode) {
  LOG(INFO) << "onKeyDown: " << keycode;
}

JNIEXPORT void JNICALL Java_com_android_o3djni_O3DJNILib_onKeyUp(JNIEnv * env, jobject obj,  jint keycode) {
  LOG(INFO) << "onKeyUp: " << keycode;
}

JNIEXPORT void JNICALL Java_com_android_o3djni_O3DJNILib_onTouch(JNIEnv * env, jobject obj,
    jint x, jint y, jfloat directionX, jfloat directionY) {
  LOG(INFO) << "onTouch: (" << x << ", " << y << ")";
}

JNIEXPORT void JNICALL Java_com_android_o3djni_O3DJNILib_onRoll(JNIEnv * env, jobject obj,
		jfloat directionX, jfloat directionY) {
	LOG(INFO) << "onRoll: (" << directionX << ", " << directionY << ")";
}


