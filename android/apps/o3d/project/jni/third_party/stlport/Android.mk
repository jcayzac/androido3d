LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := stlport

LOCAL_CFLAGS := \
  -D__ANDROID__ \
  -I$(LOCAL_PATH)/stlport \
  -I$(LOCAL_PATH)/src \

LOCAL_CPP_EXTENSION := .cpp

LOCAL_SRC_FILES := $(addprefix src/,$(notdir $(wildcard $(LOCAL_PATH)/src/*.cpp $(LOCAL_PATH)/src/*.c))))

include $(BUILD_STATIC_LIBRARY)

