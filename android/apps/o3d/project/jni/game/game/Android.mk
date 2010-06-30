#### gamecore
#
LOCAL_PATH      := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := gamegame
LOCAL_CPP_EXTENSION := .cpp
LOCAL_CFLAGS    := \
  -D__ANDROID__ \
  -I$(NDK_APP_PROJECT_PATH)/bin/headers \
  -I$(LOCAL_PATH)/../core \
  -I$(LOCAL_PATH)/../system \
  -I$(LOCAL_PATH)/../math \
  -I$(LOCAL_PATH)/../.. \
  -I$(LOCAL_PATH)/../../third_party/stlport/stlport \
   -I$(LOCAL_PATH)/../../third_party/loggingshim \


#stlport and loggingshim included so that o3d::Transform can be referenced.

LOCAL_SRC_FILES := \
  RenderComponent.cpp \
  CollisionComponent.cpp \
  CollisionPairSystem.cpp \
  GameComponent.cpp \
  GameObject.cpp \
  GameObjectSystem.cpp \
  GravityComponent.cpp \
  Interpolator.cpp \
  Lerper.cpp \
  LinearMotionComponent.cpp \
  MovementComponent.cpp \
  PhysicsComponent.cpp \


include $(BUILD_STATIC_LIBRARY)


