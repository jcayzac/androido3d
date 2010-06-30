#### gamecore
#
LOCAL_PATH      := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := gamesystems
LOCAL_CPP_EXTENSION := .cpp
LOCAL_CFLAGS    := \
  -D__ANDROID__ \
  -I$(NDK_APP_PROJECT_PATH)/bin/headers \
  -I$(LOCAL_PATH)/../core \
  -I$(LOCAL_PATH)/../math \
  


LOCAL_SRC_FILES := \
  Blackboard.cpp \
  BoxCollisionSystem.cpp \
  CollisionSystem.cpp \
  MainLoop.cpp \
  ProfileSystem.cpp \
  RenderableObject.cpp \
  Renderer.cpp \
  SystemRegistry.cpp \
  TimeSystem.cpp \
  TimeSystemPosix.cpp \


include $(BUILD_STATIC_LIBRARY)


