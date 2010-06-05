#### loggingshim
#
LOCAL_PATH      := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := loggingshim
LOCAL_CPP_EXTENSION := .cc
LOCAL_CFLAGS    := \
  -D__ANDROID__ \
  -DUNICODE \
  -I$(LOCAL_PATH)/../../third_party/stlport/stlport \
  -I$(LOCAL_PATH)/.. \
  -I$(LOCAL_PATH) \


LOCAL_SRC_FILES := $(addprefix base/, \
  third_party/dmg_fp/dtoa.cc \
  third_party/dmg_fp/g_fmt.cc \
  at_exit.cc \
  dynamic_annotations.cc \
  file_path.cc \
  file_util.cc \
  file_util_linux.cc \
  file_util_posix.cc \
  lock.cc \
  lock_impl_posix.cc \
  logging.cc \
  platform_file_posix.cc \
  platform_thread_posix.cc \
  string_piece.cc \
  string_util.cc \
  string16.cc \
  sys_string_conversions_linux.cc \
  time.cc \
  time_posix.cc \
  utf_string_conversion_utils.cc \
  utf_string_conversions.cc \
  )
  
include $(BUILD_STATIC_LIBRARY)


