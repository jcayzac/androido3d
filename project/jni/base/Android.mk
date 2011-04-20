#### o3dcore
#
LOCAL_PATH      := $(call my-dir)

include $(O3D_START_MODULE)

LOCAL_MODULE    := o3dbase
LOCAL_CPP_EXTENSION := .cc
LOCAL_SRC_FILES := $(addprefix cross/, \
  file_path.cc \
  file_util.cc \
  log.cc \
  string_util.cc \
)

include $(O3D_BUILD_MODULE)
