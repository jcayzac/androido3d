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

#include "base/cross/log.h"
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

#include "camera.h"
#include "debug.h"
#include "image_plane.h"
#include "render_graph.h"
#include "scene.h"

#include <jni.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>


class fake_window_t: public o3d::DisplayWindow {
 public:
  ~fake_window_t() { }
};

struct ImgInfo {
  bool center;
  float x;
  float y;
  float depth;
  const char* filename;
};

static ImgInfo imgs[] = {
  {  true, 699, 387, 1, "/sdcard/androido3d/images/egg.png" },
  { false,  26,  25, 2, "/sdcard/androido3d/images/gaugeback.png" },
  { false,  32,  36, 1, "/sdcard/androido3d/images/1x1white.png" },
  { false, 596,  16, 1, "/sdcard/androido3d/images/radar.png" }
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
  o3d::ServiceLocator service_locator_;
  o3d::Renderer* renderer_;
  o3d::base::scoped_ptr<o3d::EvaluationCounter> evaluation_counter_;
  o3d::base::scoped_ptr<o3d::ClassManager> class_manager_;
  o3d::base::scoped_ptr<o3d::ClientInfoManager> client_info_manager_;
  o3d::base::scoped_ptr<o3d::ObjectManager> object_manager_;
  o3d::base::scoped_ptr<o3d::Profiler> profiler_;
  o3d::base::scoped_ptr<o3d::Features> features_;
  o3d::base::scoped_ptr<o3d::Client> client_;

  o3d::Transform* root_;
  o3d::Transform* hud_root_;
  o3d::Pack* effect_texture_pack_;
  o3d::Pack* pack_;
  o3d_utils::ViewInfo* main_view_;
  o3d_utils::ViewInfo* hud_view_;
  o3d_utils::ImagePlane* images_[o3d_arraysize(imgs)];
  o3d_utils::Scene* scene_;
  o3d::ElapsedTimeTimer timer_;
  float time_;
};

O3DManager::O3DManager()
    : root_(NULL),
      hud_root_(NULL),
      pack_(NULL),
      main_view_(NULL),
      hud_view_(NULL),
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

  if (renderer_->Init(fake_window_t(), false) != o3d::Renderer::SUCCESS) {
    O3D_LOG(ERROR) << "Window initialization failed!";
    return false;
  }

  client_.reset(new o3d::Client(&service_locator_));
  client_->Init();
  pack_ = client_->CreatePack();
  effect_texture_pack_ = client_->CreatePack();
  root_ = pack_->Create<o3d::Transform>();
  main_view_ = o3d_utils::ViewInfo::CreateBasicView(
      pack_, root_, client_->render_graph_root());

  // Create a second view for the hud.
  hud_root_ = pack_->Create<o3d::Transform>();
  hud_view_ = o3d_utils::ViewInfo::CreateBasicView(
      pack_, hud_root_, client_->render_graph_root());

  // Make sure the hud gets drawn after the 3d stuff
  hud_view_->root()->set_priority(main_view_->root()->priority() + 1);

  // Turn off clearing the color for the hud since that would erase the 3d
  // parts but leave clearing the depth and stencil so the HUD is unaffected
  // by anything done by the 3d parts.
  hud_view_->clear_buffer()->set_clear_color_flag(false);

  // Set culling to none so we can flip images using rotation or negative scale.
  o3d::State* state = hud_view_->z_ordered_draw_pass_info()->state();
  state->GetStateParam<o3d::ParamInteger>(
        o3d::State::kCullModeParamName)->set_value(o3d::State::CULL_NONE);
  state->GetStateParam<o3d::ParamBoolean>(
        o3d::State::kZWriteEnableParamName)->set_value(false);

  hud_view_->draw_context()->set_view(o3d::Matrix4::lookAt(
      o3d::Point3(0.0f, 0.0f, 30.0f),
      o3d::Point3(0.0f, 0.0f, 0.0f),
      o3d::Vector3(0.0f, 1.0f, 0.0f)));

  for (size_t ii = 0; ii < o3d_arraysize(images_); ++ii) {
    const ImgInfo& info = imgs[ii];
    images_[ii] = o3d_utils::ImagePlane::Create(
        pack_, pack_, hud_view_, info.filename, info.center);
    images_[ii]->transform()->SetParent(hud_root_);
    images_[ii]->transform()->set_local_matrix(o3d::Matrix4::translation(
        o3d::Vector3(info.x, info.y, info.depth)));
  }

  images_[0]->transform()->GetChildrenRefs()[0]->GetShapeRefs()[0]->
      GetElementRefs()[0]->set_cull(false);

  scene_ = o3d_utils::Scene::LoadScene(
      client_.get(),
      main_view_,
      "/sdcard/androido3d/collada/character.zip",
      effect_texture_pack_);
  if (!scene_)
    scene_ = o3d_utils::Scene::DummyScene(client_.get(), main_view_);
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
  std::vector<o3d::Pack*> packs;
  packs.push_back(scene_->pack());
  for (size_t ii = 0; ii < packs.size(); ++ii) {
    std::vector<o3d::Material*> materials =
        packs[ii]->GetByClass<o3d::Material>();
    for (size_t ii = 0; ii < materials.size(); ++ii) {
      o3d::ParamFloat3* pos =
          materials[ii]->GetParam<o3d::ParamFloat3>("lightWorldPos");
      if (pos) {
        pos->set_value(o3d::Float3(light_pos[0], light_pos[1], light_pos[2]));
      }
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
  return o3d::down_cast<o3d::RendererGLES2*>(renderer_)->OnContextRestored();
}

void O3DManager::SetProjection(float width, float height) {
  main_view_->draw_context()->set_projection(
      o3d_utils::Camera::perspective(
          o3d_utils::degToRad(30.0f), width / height, 10, 1000));

  hud_view_->draw_context()->set_projection(
      o3d::Matrix4::orthographic(
          .0f,
          width,
          height,
          .0f,
          .001f,
          1000.f));
}

bool O3DManager::ResizeViewport(int width, int height) {
  renderer_->Resize(width, height);

  o3d_utils::CameraInfo* camera_info =
      o3d_utils::Camera::getViewAndProjectionFromCameras(
          root_, width, height);

  main_view_->draw_context()->set_view(camera_info->view);
  SetProjection(width, height);
  return true;
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

    o3d::Matrix4 mat(o3d::Matrix4::lookAt(position, target, up));
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

  // Move the meter in the gauge.
  images_[2]->SetColorMult(o3d::Float4(
      (sinf(time_ * 4.0f) + 1.0f) * 0.5f, 0.0f, 0.0f, 1.0f));
  images_[2]->transform()->set_local_matrix(
     o3d::Matrix4::translation(
         o3d::Vector3(imgs[2].x, imgs[2].y, imgs[2].depth)) *
     o3d::Matrix4::scale(o3d::Vector3(fmodf(time_ / 2, 1.0f) * 384, 80, 1)));

  // Spin the egg.
  images_[0]->transform()->set_local_matrix(
     o3d::Matrix4::translation(
       o3d::Vector3(imgs[0].x, imgs[0].y, imgs[0].depth)) *
     o3d::Matrix4::rotationZ(time_));

  // Fade the radar in/out.
  images_[3]->SetColorMult(
      o3d::Float4(1.0f, 1.0f, 1.0f, (sinf(time_ * 2) + 1) * 0.5));

  client_->Tick();
  client_->RenderClient(true);
  CheckError();

  return true;
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
    O3D_LOG(ERROR) << error;
    client_->ClearLastError();
  }
};

static O3DManager* g_mgr = NULL;




#define JNIFUNC(x) extern "C" JNIEXPORT void JNICALL Java_com_android_o3djni_O3DJNILib_##x

JNIFUNC(init) (JNIEnv * env, jobject obj,  jint width, jint height) {
  glViewport(0, 0, width, height);
  if (g_mgr) g_mgr->ResizeViewport(width, height);
  else {
    g_mgr = new O3DManager();
    g_mgr->Initialize(width, height);
  }
}

JNIFUNC(surfaceCreated) (JNIEnv * env, jobject obj) {
  // Called when the EGL surface is created, such as on boot and after a context loss.
  if (g_mgr) {
    O3D_LOG(INFO) << "Restoring Resources";
    if (!g_mgr->OnContextRestored()) {
      O3D_LOG(ERROR) << "Failed to restore resources";
      g_mgr->CheckError();
    }
  }
}

JNIFUNC(step) (JNIEnv * env, jobject obj) {
  g_mgr->Render();
}

JNIFUNC(onKeyDown) (JNIEnv * env, jobject obj,  jint keycode) {
  O3D_LOG(INFO) << "onKeyDown: " << keycode;
}

JNIFUNC(onKeyUp) (JNIEnv * env, jobject obj,  jint keycode) {
  O3D_LOG(INFO) << "onKeyUp: " << keycode;
}

JNIFUNC(onTouch) (JNIEnv * env, jobject obj, jint x, jint y, jfloat directionX, jfloat directionY) {
  O3D_LOG(INFO) << "onTouch: (" << x << ", " << y << ")";
}

JNIFUNC(onRoll) (JNIEnv * env, jobject obj, jfloat directionX, jfloat directionY) {
	O3D_LOG(INFO) << "onRoll: (" << directionX << ", " << directionY << ")";
}
