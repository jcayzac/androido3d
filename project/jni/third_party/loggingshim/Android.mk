#### loggingshim
#
LOCAL_PATH := $(call my-dir)

include $(O3D_START_MODULE)

LOCAL_MODULE := loggingshim
LOCAL_CPP_EXTENSION := .cc
LOCAL_SRC_FILES := $(addprefix base/, \
  third_party/dmg_fp/dtoa.cc \
  third_party/dmg_fp/g_fmt.cc \
  third_party/icu/icu_utf.cc \
  third_party/nspr/prtime.cc \
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
  
include $(O3D_BUILD_MODULE)
