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
#include "core/cross/timer.h"
#include "core/cross/transform.h"
#include "core/cross/types.h"

#include "render_graph.h"
#include "camera.h"
#include "debug.h"
#include "scene.h"


// game stuff
#include "AnimationComponent.h"
#include "GameObject.h"
#include "GameObjectSystem.h"
#include "MainLoop.h"
#include "MathUtils.h"
#include "MetaRegistry.h"
#include "MovementComponent.h"
#include "PlayerAnimationComponent.h"
#include "PlayerMotionComponent.h"
#include "ProfileSystem.h"
#include "RenderComponent.h"
#include "SystemRegistry.h"
#include "TimeSystemPosix.h"
#include "Vector3.h"

#include "meta_interface.h"

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

  bool Initialize(int width, int height);
  bool ResizeViewport(int width, int height);
  void SetProjection(float width, float height);
  bool Render();
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
  DumpRenderGraph(client_->render_graph_root(), "");
  LOGI("---Render Graph---(end)---\n");

  scene_ = o3d_utils::Scene::LoadScene(
      client_.get(),
      main_view_,
//      "/sdcard/collada/seven_shapes.zip",
//      "/sdcard/collada/cube.zip",
//      "/sdcard/collada/kitty_151_idle_stand05_cff1.zip",
      "/sdcard/collada/character.zip",
      NULL);
  scene_->SetParent(root_);

  o3d_utils::CameraInfo* camera_info =
      o3d_utils::Camera::getViewAndProjectionFromCameras(
          root_, width, height);

  main_view_->draw_context()->set_view(camera_info->view);
  //main_view_->draw_context()->set_projection(camera_info->projection);
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

void O3DManager::SetProjection(float width, float height) {
  main_view_->draw_context()->set_projection(
      o3d_utils::Camera::perspective(
          o3d_utils::degToRad(30.0f), width / height, 10, 1000));
}

// I have no idea if this is right.
bool O3DManager::ResizeViewport(int width, int height) {
  renderer_->Resize(width, height);

  o3d_utils::CameraInfo* camera_info =
      o3d_utils::Camera::getViewAndProjectionFromCameras(
          root_, width, height);

  main_view_->draw_context()->set_view(camera_info->view);
  //main_view_->draw_context()->set_projection(camera_info->projection);
  SetProjection(width, height);
}

bool O3DManager::Render() {
  time_ += timer_.GetElapsedTimeAndReset();
  /*if (time_ > 249.0f / 30.0f) {  // end of kitty anim
    time_ = 0.0f;
  }
  scene_->SetAnimationTime(time_);*/

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
static ObjectHandle<MainLoop> g_mainLoop = NULL;
static ObjectHandle<GameObject> g_object = NULL;

void startUpGame() {
  g_mainLoop = MainLoop::factory();

  TimeSystemPosix* pTimeSystem = TimeSystemPosix::factory();
	pTimeSystem->startup();
	g_mainLoop->setTimeSystem(pTimeSystem);

	SystemRegistry::getSystemRegistry()->addSystem(g_mainLoop);
	SystemRegistry::getSystemRegistry()->addSystem(pTimeSystem);

  ProfileSystem* pProfiler = ProfileSystem::factory();
	SystemRegistry::getSystemRegistry()->addSystem(pProfiler);
	g_mainLoop->addSystem(pProfiler);

	GameObjectSystem* pGameObjectSystem = GameObjectSystem::factory();
	SystemRegistry::getSystemRegistry()->addSystem(pGameObjectSystem);
	g_mainLoop->addSystem(pGameObjectSystem);

	// Make a game object!
	GameObject* object = new GameObject();
	RenderComponent* render = RenderComponent::factory();
	MovementComponent* movement = MovementComponent::factory();
	AnimationComponent* animation = AnimationComponent::factory();
	PlayerAnimationComponent* playerAnim = PlayerAnimationComponent::factory();
	PlayerMotionComponent* playerMotion = PlayerMotionComponent::factory();

	object->add(render);
	object->add(movement);
	object->add(animation);
	object->add(playerAnim);
	object->add(playerMotion);

	/* idle1: {startFrame: 0, endFrame: 30},
  walk: {startFrame: 31, endFrame: 71},
  jumpStart: {startFrame: 72, endFrame: 87},
  jumpUp: {startFrame: 87, endFrame: 87},
  jumpCrest: {startFrame: 87, endFrame: 91},
  jumpFall: {startFrame: 91, endFrame: 91},
  jumpLand: {startFrame: 91, endFrame: 110},
  run: {startFrame: 111, endFrame: 127},
  idle2: {startFrame: 128, endFrame: 173},
  idle3: {startFrame: 174, endFrame: 246},
  idle4: {startFrame: 247, endFrame: 573}}; */

	AnimationComponent::AnimationRecord* walkAnimation = AnimationComponent::AnimationRecord::factory();
	walkAnimation->setStartFrame(31);
	walkAnimation->setEndFrame(71);
	walkAnimation->setFramesPerSecond(30);
	walkAnimation->setLooping(true);

	const int walk = animation->addAnimation(walkAnimation);

	AnimationComponent::AnimationRecord* idle1Animation = AnimationComponent::AnimationRecord::factory();
	idle1Animation->setStartFrame(0);
	idle1Animation->setEndFrame(30);
	idle1Animation->setFramesPerSecond(30);
	idle1Animation->setLooping(true);

	const int idle1 = animation->addAnimation(idle1Animation);

	AnimationComponent::AnimationRecord* runAnimation = AnimationComponent::AnimationRecord::factory();
	runAnimation->setStartFrame(111);
	runAnimation->setEndFrame(127);
	runAnimation->setFramesPerSecond(30);
	runAnimation->setLooping(true);

	const int run = animation->addAnimation(runAnimation);

	animation->playAnimation(idle1);

	playerAnim->setIdleAnimation(idle1);
	playerAnim->setWalkAnimation(walk);
	playerAnim->setRunAnimation(run);

	playerMotion->setMaxSpeed(Vector3(25.0f, 0.0f, 25.0f));
	playerMotion->setAcceleration(Vector3(100.0f, 100.0f, 100.0f));

	pGameObjectSystem->add(object);
	g_object = object;

	// Let's load the box for this object.
	//o3d::Transform* root = g_mgr->LoadAndAppend("/sdcard/collada/cube.zip");

  render->setTransform(g_mgr->GetRoot());
  animation->setSceneRoot(g_mgr->GetScene());
}


extern "C" {
    JNIEXPORT void JNICALL Java_com_android_o3djni_O3DJNILib_init(JNIEnv * env, jobject obj,  jint width, jint height);
    JNIEXPORT void JNICALL Java_com_android_o3djni_O3DJNILib_step(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_android_o3djni_O3DJNILib_onKeyDown(JNIEnv * env, jobject obj,  jint keycode);
    JNIEXPORT void JNICALL Java_com_android_o3djni_O3DJNILib_onKeyUp(JNIEnv * env, jobject obj,  jint keycode);
    JNIEXPORT void JNICALL Java_com_android_o3djni_O3DJNILib_onTouch(JNIEnv * env, jobject obj,
        jint x, jint y, jfloat directionX, jfloat directionY);
    JNIEXPORT void JNICALL Java_com_android_o3djni_O3DJNILib_onRoll(JNIEnv * env, jobject obj,
    		jfloat directionX, jfloat directionY);
    JNIEXPORT jobjectArray JNICALL Java_com_android_o3djni_O3DJNILib_getSystemList(JNIEnv * env, jobject obj);
    JNIEXPORT jobjectArray JNICALL Java_com_android_o3djni_O3DJNILib_getMetaData(JNIEnv * env, jobject obj, jobjectArray path);
};

JNIEXPORT void JNICALL Java_com_android_o3djni_O3DJNILib_init(JNIEnv * env, jobject obj,  jint width, jint height) {
    glViewport(0, 0, width, height);

    if (g_mgr != NULL) {
      g_mgr->ResizeViewport(width, height);
    } else {
      g_mgr = new O3DManager();
      g_mgr->Initialize(width, height);
    }

    if (g_mainLoop == NULL) {
      startUpGame();
    }
}

JNIEXPORT void JNICALL Java_com_android_o3djni_O3DJNILib_step(JNIEnv * env, jobject obj) {
    g_mainLoop->updateAll();

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

  g_object->getRuntimeData()->insertInt(1, "go");
}

JNIEXPORT void JNICALL Java_com_android_o3djni_O3DJNILib_onRoll(JNIEnv * env, jobject obj,
		jfloat directionX, jfloat directionY) {
	LOG(INFO) << "onRoll: (" << directionX << ", " << directionY << ")";

	Vector3 orientation = g_object->getRuntimeData()->getVector("orientation");
	orientation[1] += directionX;
	g_object->getRuntimeData()->insertVector(orientation, "orientation");

}


JNIEXPORT jobjectArray JNICALL Java_com_android_o3djni_O3DJNILib_getSystemList(JNIEnv * env, jobject obj) {
  const int systemCount = SystemRegistry::getSystemRegistry()->getCount();

  jobjectArray ret = MetaInterface::getSystems(env);
  
  return ret;
}

// Format is:
// System/Field[/Index]/Object/
JNIEXPORT jobjectArray JNICALL Java_com_android_o3djni_O3DJNILib_getMetaData(JNIEnv * env, jobject obj, jobjectArray path) {
  const jsize path_elements = env->GetArrayLength(path);
  
  MetaInterface interface(env, path, path_elements);

  jobjectArray result = interface.parsePath();
  
  return result;
}
