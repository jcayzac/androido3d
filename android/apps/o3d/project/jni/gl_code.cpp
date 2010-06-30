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
#include "GameObject.h"
#include "GameObjectSystem.h"
#include "MainLoop.h"
#include "MathUtils.h"
#include "MetaRegistry.h"
#include "MovementComponent.h"
#include "ProfileSystem.h"
#include "RenderComponent.h"
#include "SystemRegistry.h"
#include "TimeSystemPosix.h"
#include "Vector3.h"

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
  bool Render();
  o3d::Transform* GetRoot();
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
      "/sdcard/collada/cube.zip",
//      "/sdcard/collada/kitty_151_idle_stand05_cff1.zip",
//      "/sdcard/collada/character.zip",
      NULL);
  scene_->SetParent(root_);

  o3d_utils::CameraInfo* camera_info =
      o3d_utils::Camera::getViewAndProjectionFromCameras(
          root_, width, height);

  main_view_->draw_context()->set_view(camera_info->view);
  main_view_->draw_context()->set_projection(camera_info->projection);

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

// I have no idea if this is right.
bool O3DManager::ResizeViewport(int width, int height) {
  renderer_->Resize(width, height);

  o3d_utils::CameraInfo* camera_info =
      o3d_utils::Camera::getViewAndProjectionFromCameras(
          root_, width, height);

  main_view_->draw_context()->set_view(camera_info->view);
  main_view_->draw_context()->set_projection(camera_info->projection);
}

bool O3DManager::Render() {
  time_ += timer_.GetElapsedTimeAndReset();
  if (time_ > 249.0f / 30.0f) {  // end of kitty anim
    time_ = 0.0f;
  }
  scene_->SetAnimationTime(time_);

  client_->Tick();
  client_->RenderClient(true);

  CheckError();
}

o3d::Transform* O3DManager::GetRoot() {
  return root_;
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
	object->add(render);
	object->add(movement);
	

	pGameObjectSystem->add(object);
	g_object = object;
	
	// Let's load the box for this object.
	//o3d::Transform* root = g_mgr->LoadAndAppend("/sdcard/collada/cube.zip");
  
  render->setTransform(g_mgr->GetRoot());
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
}

JNIEXPORT void JNICALL Java_com_android_o3djni_O3DJNILib_onRoll(JNIEnv * env, jobject obj,
		jfloat directionX, jfloat directionY) {
	LOG(INFO) << "onRoll: (" << directionX << ", " << directionY << ")";
	
	Vector3 current = g_object->getRuntimeData()->getVector("velocity");
	const float speed = 0.3f;
	
  Vector3 velocity = current + Vector3(directionX != 0.0f ? speed * Sign(directionX) : 0.0f, 
        directionY != 0.0f ? -speed * Sign(directionY) : 0.0f, 0.0f);   
	
	g_object->getRuntimeData()->insertVector(velocity, "velocity");
	g_object->getRuntimeData()->insertVector(Vector3::ZERO, "targetVelocity");
	g_object->getRuntimeData()->insertVector(Vector3::ONE, "acceleration");
	
}


JNIEXPORT jobjectArray JNICALL Java_com_android_o3djni_O3DJNILib_getSystemList(JNIEnv * env, jobject obj) {
  const int systemCount = SystemRegistry::getSystemRegistry()->getCount();
  
  jobjectArray ret;
  
  ret = (jobjectArray)env->NewObjectArray(systemCount, 
    env->FindClass("java/lang/String"), NULL);

  for (int x = 0; x < systemCount; x++) {
    const char* name = SystemRegistry::getSystemRegistry()->get(x)->getMetaObject()->getName();
    env->SetObjectArrayElement(ret, x, env->NewStringUTF(name));
  }
  
  return ret;
}

// Format is:
// System/Field[/Index]/Object/
JNIEXPORT jobjectArray JNICALL Java_com_android_o3djni_O3DJNILib_getMetaData(JNIEnv * env, jobject obj, jobjectArray path) {
  const jsize pathElements = env->GetArrayLength(path);
  
  jobjectArray result = NULL;
  
  if (pathElements > 0) {
    const jstring system_jni = (jstring)env->GetObjectArrayElement(path, 0);
    const char* system_string = env->GetStringUTFChars(system_jni, NULL);
    DLOG(INFO) << "Looking for system: " << system_string;
    const MetaObject* system_meta = MetaRegistry::getMetaRegistry()->getMetaObject(system_string);
    env->ReleaseStringUTFChars(system_jni, system_string);
    const System* system = SystemRegistry::getSystemRegistry()->getSystem(system_meta);
    MetaBase const* root = system;
    
    if (system != NULL) {
      DLOG(INFO) << "Found system: " << system->getMetaObject()->getName();
      bool done = false;
      for (int x = 1; x < pathElements && !done; x++) {
        const MetaObject* meta = root->getMetaObject();
        if (!meta) {
          break;
        }
        const jstring field = (jstring)env->GetObjectArrayElement(path, x);
        const char* field_string = env->GetStringUTFChars(field, NULL);
        const int field_count = meta->getFieldCount();
        for (int y = 0; y < field_count; y++) {
          const MetaField* metaField = meta->getField(y);
          DLOG(INFO) << "Field: " << metaField->getName() << " (offset: " << metaField->getOffset() << ")";

          if (strcmp(field_string, metaField->getName()) == 0) {
            DLOG(INFO) << "Found field: " << metaField->getName();
            // found the field!
            if (metaField->getStorageType() == MetaField::TYPE_pointer) {
              // todo: array support
              void* object = *((void**)metaField->get(root));
              if (object && MetaBase::authenticatePointer(object)) {
                // safe to cast!
                root = static_cast<MetaBase*>(object);
                break;
              }
              
              DLOG(INFO) << "Field null or unknown type: " << metaField->getTypeName();
            }
            // we found the field but can't go any further.
            done = true;
            break;
          }
        }
        
        
        env->ReleaseStringUTFChars(field, field_string);
      }
    
    }
    
    if (root) {
      DLOG(INFO) << "Return object: " << root->getMetaObject()->getName();
      const MetaObject* meta = root->getMetaObject();
      const int field_count = meta->getFieldCount();
      result = (jobjectArray)env->NewObjectArray(field_count, 
        env->FindClass("java/lang/String"), NULL);
    
      for (int x = 0; x < field_count; x++) {
        env->SetObjectArrayElement(result, x, env->NewStringUTF(meta->getField(x)->getName()));
      }
      
    }
    
    
  }
  
  return result;
}
