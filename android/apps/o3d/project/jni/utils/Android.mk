#### o3dimport
#
LOCAL_PATH      := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := o3dutils
LOCAL_CPP_EXTENSION := .cc
LOCAL_CFLAGS    := \
  -D__ANDROID__ \
  -DRENDERER_GLES2 \
  -DGLES2_BACKEND_NATIVE_GLES2 \
  -DO3D_IMPORT_NO_CG \
  -DO3D_IMPORT_NO_CONDITIONER \
  -DO3D_IMPORT_NO_DXT_DECOMPRESSION \
  -DFCOLLADA_EXCEPTION=0 \
  -DUNICODE \
  -I$(LOCAL_PATH)/../third_party/stlport/stlport \
  -I$(LOCAL_PATH)/../third_party/fcollada/files/LibXML/include \
  -I$(LOCAL_PATH)/../third_party/fcollada/files \
  -I$(LOCAL_PATH)/../third_party/loggingshim \
  -I$(LOCAL_PATH)/../third_party/zlib \
  -I$(LOCAL_PATH)/../third_party \
  -I$(LOCAL_PATH)/.. \

LOCAL_REMOVED   := \
  temporary_file.cc \

LOCAL_SRC_FILES := $(addprefix cross/, \
  base64.cc \
  dataurl.cc \
  file_path_utils.cc \
  file_text_reader.cc \
  file_text_writer.cc \
  json_writer.cc \
  string_reader.cc \
  string_writer.cc \
  structured_writer.h \
  text_reader.cc \
  text_writer.cc \
  )

include $(BUILD_STATIC_LIBRARY)


