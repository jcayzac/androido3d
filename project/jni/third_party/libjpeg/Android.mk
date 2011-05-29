#### libjpeg
#
LOCAL_PATH := $(call my-dir)

include $(O3D_START_MODULE)

LOCAL_MODULE    := jpeg

LOCAL_SRC_FILES := $(addprefix current/, \
jaricom.c \
jcapimin.c \
jcapistd.c \
jcarith.c \
jccoefct.c \
jccolor.c \
jchuff.c \
jcdctmgr.c \
jcinit.c \
jcmainct.c \
jcmarker.c \
jcmaster.c \
jcomapi.c \
jcparam.c \
jcprepct.c \
jcsample.c \
jctrans.c \
jdapimin.c \
jdapistd.c \
jdarith.c \
jdatadst.c \
jdatasrc.c \
jdcoefct.c \
jdcolor.c \
jddctmgr.c \
jdhuff.c \
jdinput.c \
jdmainct.c \
jdmarker.c \
jdmaster.c \
jdmerge.c \
jdpostct.c \
jdsample.c \
jdtrans.c \
jerror.c \
jfdctflt.c \
jfdctfst.c \
jfdctint.c \
jidctflt.c \
jidctfst.c \
jidctint.c \
jquant1.c \
jquant2.c \
jutils.c \
jmemmgr.c \
jmemname.c \
)
include $(O3D_BUILD_MODULE)

