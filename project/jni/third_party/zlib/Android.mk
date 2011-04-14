#### zlib
#
LOCAL_PATH      := $(call my-dir)

include $(O3D_START_MODULE)

LOCAL_MODULE    := zlib

LOCAL_SRC_FILES := $(addprefix current/, \
  $(addprefix contrib/minizip/, \
    ioapi.c \
    unzip.c \
    zip.c \
  ) \
  adler32.c \
  compress.c \
  crc32.c \
  deflate.c \
  gzclose.c \
  gzlib.c \
  gzread.c \
  gzwrite.c \
  infback.c \
  inffast.c \
  inflate.c \
  inftrees.c \
  trees.c \
  uncompr.c \
  zutil.c \
)

include $(O3D_BUILD_MODULE)
