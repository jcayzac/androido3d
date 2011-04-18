#### libpng
#
LOCAL_PATH := $(call my-dir)

include $(O3D_START_MODULE)

LOCAL_MODULE := png

LOCAL_CFLAGS += \
  -DHAVE_CONFIG_H \
  -DPNG_CONFIGURE_LIBPNG \

LOCAL_C_INCLUDES += $(LOCAL_PATH)/include

LOCAL_SRC_FILES := $(addprefix current/, \
  png.c \
  pngerror.c \
  pngget.c \
  pngmem.c \
  pngpread.c \
  pngread.c \
  pngrio.c \
  pngrtran.c \
  pngrutil.c \
  pngset.c \
  pngtrans.c \
  pngwio.c \
  pngwrite.c \
  pngwtran.c \
  pngwutil.c \
)
include $(O3D_BUILD_MODULE)

