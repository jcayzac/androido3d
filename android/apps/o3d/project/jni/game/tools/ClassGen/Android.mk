#### classgen
#
LOCAL_PATH      := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := classgen
LOCAL_CPP_EXTENSION := .cpp
LOCAL_CFLAGS    := \
  -I$(LOCAL_PATH)/.. \


LOCAL_SRC_FILES := \
  ClassGenGlue.cpp \
  ClassGenerator.cpp \
  ParseElement.c \
  classgen_lexer.l \
  classgen_parser.y \


include $(BUILD_HOST_EXECUTABLE)


