#### libtxc_dxtn
#
LOCAL_PATH      := $(call my-dir)

include $(O3D_START_MODULE)

LOCAL_MODULE := txc_dxtn
LOCAL_CPP_EXTENSION := .cc
LOCAL_SRC_FILES := \
  txc_compress_dxtn.c \
  txc_fetch_dxtn.cc \

include $(O3D_BUILD_MODULE)


