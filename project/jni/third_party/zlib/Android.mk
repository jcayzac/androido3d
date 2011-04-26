#### zlib
#
LOCAL_PATH      := $(call my-dir)

include $(O3D_START_MODULE)

LOCAL_MODULE    := zlib

LOCAL_SRC_FILES := \
  contrib/minizip/ioapi.c \
  contrib/minizip/unzip.c \
  contrib/minizip/zip.c \
  adler32.c \
  compress.c \
  crc32.c \
  deflate.c \
  gzio.c \
  infback.c \
  inffast.c \
  inflate.c \
  inftrees.c \
  trees.c \
  uncompr.c \
  zutil.c \

include $(O3D_BUILD_MODULE)
