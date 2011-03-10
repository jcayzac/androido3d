#
# Copyright (C) 2010 Tonchidot Corporation.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

LOCAL_PATH := $(call my-dir)

include $(O3D_START_MODULE)

LOCAL_MODULE := lzma
# Threads.c doesn't build, so don't enable multithread for now
LOCAL_CFLAGS += -D_7ZIP_ST
LOCAL_C_INCLUDES += \
  $(LOCAL_PATH)/include/lzma/C \

LOCAL_SRC_FILES := $(addprefix current/C/, \
  LzFind.c \
  LzmaDec.c \
  LzmaEnc.c \
  Sha256.c \
)

include $(O3D_BUILD_MODULE)
