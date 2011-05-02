LOCAL_PATH      := $(call my-dir)

include $(O3D_START_MODULE)

LOCAL_MODULE    := o3dutils
LOCAL_CPP_EXTENSION := .cc
LOCAL_C_INCLUDES += \
  $(O3D_THIRD_PARTY)/fcollada/files/LibXML/include \
  $(O3D_THIRD_PARTY)/fcollada/files \

LOCAL_SRC_FILES := $(addprefix cross/, \
  base64.cc \
  dataurl.cc \
  )

include $(O3D_BUILD_MODULE)
