#### gamemath
#
LOCAL_PATH      := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := gamemath
LOCAL_CPP_EXTENSION := .cpp
LOCAL_CFLAGS    := \
  -D__ANDROID__ \
  -I$(NDK_APP_PROJECT_PATH)/bin/headers \
  -I$(LOCAL_PATH)/../core \
  


LOCAL_SRC_FILES := \
  Box.cpp \
  MathUtils.cpp \
  Vector2.cpp \
  Vector3.cpp \


include $(BUILD_STATIC_LIBRARY)


