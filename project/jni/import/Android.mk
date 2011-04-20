#### o3dimport
#
LOCAL_PATH := $(call my-dir)

include $(O3D_START_MODULE)

LOCAL_MODULE := o3dimport
LOCAL_CPP_EXTENSION := .cc

LOCAL_C_INCLUDES += $(addprefix $(O3D_THIRD_PARTY)/fcollada/include/, \
  FCollada \
  FCollada/LibXML/include \
  FColladaPlugins \
)

LOCAL_SRC_FILES := $(addprefix cross/, \
  collada.cc \
  collada_zip_archive.cc \
  destination_buffer.cc \
  memory_stream.cc \
  raw_data.cc \
  zip_archive.cc \
  )

include $(O3D_BUILD_MODULE)
