#### o3dimport
#
LOCAL_PATH      := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := debug
LOCAL_CPP_EXTENSION := .cpp
LOCAL_CFLAGS    := \
  -I$(LOCAL_PATH)/../third_party/stlport/stlport \

LOCAL_SRC_FILES := \
  debugconsole.cpp \
  debugprint.cpp \
  debugtext.cpp \
  debugplatform_android.cpp \

include $(BUILD_STATIC_LIBRARY)


