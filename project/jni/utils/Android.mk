LOCAL_PATH      := $(call my-dir)

include $(O3D_START_MODULE)

LOCAL_MODULE    := o3dutils
LOCAL_CPP_EXTENSION := .cc
LOCAL_C_INCLUDES += \
  $(O3D_THIRD_PARTY)/fcollada/files/LibXML/include \
  $(O3D_THIRD_PARTY)/fcollada/files \
  $(O3D_THIRD_PARTY)/zlib \

LOCAL_SRC_FILES := $(addprefix cross/, \
  base64.cc \
  dataurl.cc \
  file_path_utils.cc \
  file_text_reader.cc \
  file_text_writer.cc \
  json_writer.cc \
  string_reader.cc \
  string_writer.cc \
  text_reader.cc \
  text_writer.cc \
  )

include $(O3D_BUILD_MODULE)
