#### libtxc_dxtn
#
LOCAL_PATH      := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := libtxc_dxtn
LOCAL_CPP_EXTENSION := .cc
LOCAL_CFLAGS    := \
  -D__ANDROID__ \
  -DUNICODE \
  -I$(LOCAL_PATH)/../../../third_party/loggingshim \

LOCAL_SRC_FILES := \
  txc_compress_dxtn.c \
  txc_fetch_dxtn.cc \

include $(BUILD_STATIC_LIBRARY)


