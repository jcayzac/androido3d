#### libpng
#
LOCAL_PATH := $(call my-dir)

include $(O3D_START_MODULE)

LOCAL_MODULE := png

LOCAL_SRC_FILES := \
  png.c \
  pngerror.c \
  pnggccrd.c \
  pngget.c \
  pngmem.c \
  pngpread.c \
  pngread.c \
  pngrio.c \
  pngrtran.c \
  pngrutil.c \
  pngset.c \
  pngtrans.c \
  pngvcrd.c \
  pngwio.c \
  pngwrite.c \
  pngwtran.c \
  pngwutil.c \

include $(O3D_BUILD_MODULE)


