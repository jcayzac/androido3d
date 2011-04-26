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

# library and applications should use the same version of the STL,
# or global objects of the std:: namespace won't match!
APP_STL := stlport_static

# library and applications must use the same ABI
APP_ABI := armeabi-v7a

# O3D native source code lies here:
O3D_NATIVE_DIR := $(O3D_DIR)/jni

# CFLAGS used at various places, that applications
# should use too as they're used in some header files
APP_CFLAGS := \
  -Wall \
  -Wfloat-equal \
  -Wdisabled-optimization \
  -DUNICODE \
  -DRENDERER_GLES2 \
  -DGLES2_BACKEND_NATIVE_GLES2 \
  -DFCOLLADA_EXCEPTION=0 \
  -I$(O3D_NATIVE_DIR) \
  -I$(O3D_NATIVE_DIR)/third_party/loggingshim \

ifneq ($(NDK_DEBUG), 0)
  # build system adds -O0 -g, so we don't need to define them here
  APP_CFLAGS += -D_DEBUG -DDEBUG
else
  APP_CFLAGS += \
    -DRETAIL \
    -O3 \
    -ffast-math \
    -ftree-vectorize \
    -fomit-frame-pointer \
    -ftree-loop-ivcanon \
    -fgcse-sm \
    -fgcse-las \
    -fgcse-after-reload \
    -funsafe-loop-optimizations \
    -fsee \
    -ftracer \

endif

# Makefiles
include $(O3D_DIR)/build/make/o3d-imports.mk
