#### o3dimport
#
LOCAL_PATH      := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := o3dimport
LOCAL_CPP_EXTENSION := .cc
LOCAL_CFLAGS    := \
  -D__ANDROID__ \
  -DRENDERER_GLES2 \
  -DGLES2_BACKEND_NATIVE_GLES2 \
  -DO3D_IMPORT_NO_CG \
  -DO3D_IMPORT_NO_CONDITIONER \
  -DO3D_IMPORT_NO_DXT_DECOMPRESSION \
  -DO3D_NO_TEMP_FILES \
  -DFCOLLADA_EXCEPTION=0 \
  -DUNICODE \
  -I$(LOCAL_PATH)/../third_party/stlport/stlport \
  -I$(LOCAL_PATH)/../third_party/fcollada/files/LibXML/include \
  -I$(LOCAL_PATH)/../third_party/fcollada/files \
  -I$(LOCAL_PATH)/../third_party/loggingshim \
  -I$(LOCAL_PATH)/../third_party/zlib \
  -I$(LOCAL_PATH)/../third_party \
  -I$(LOCAL_PATH)/.. \

LOCAL_SRC_FILES := $(addprefix cross/, \
  collada.cc \
  collada_zip_archive.cc \
  destination_buffer.cc \
  memory_stream.cc \
  raw_data.cc \
  zip_archive.cc \
  )

include $(BUILD_STATIC_LIBRARY)


