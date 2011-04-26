#### o3dimport
#
LOCAL_PATH := $(call my-dir)

include $(O3D_START_MODULE)

LOCAL_MODULE := o3dimport
LOCAL_CPP_EXTENSION := .cc
LOCAL_CFLAGS += \
  -DO3D_IMPORT_NO_CG \
  -DO3D_IMPORT_NO_CONDITIONER \
  -DO3D_IMPORT_NO_DXT_TO_PNG \
  -DO3D_IMPORT_DECOMPRESS_DXT \
  -DO3D_NO_TEMP_FILES \

LOCAL_C_INCLUDES += \
  $(O3D_THIRD_PARTY)/fcollada/files/LibXML/include \
  $(O3D_THIRD_PARTY)/fcollada/files \

LOCAL_SRC_FILES := $(addprefix cross/, \
  targz_processor.cc \
  archive_processor.cc \
  gz_decompressor.cc \
  tar_processor.cc \
  collada.cc \
  collada_zip_archive.cc \
  destination_buffer.cc \
  memory_stream.cc \
  raw_data.cc \
  zip_archive.cc \
  )

include $(O3D_BUILD_MODULE)


