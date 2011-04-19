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
  -DUSE_FILE32API \
  -DGOOGLE_PROTOBUF_NO_RTTI \
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
    -funsafe-math-optimizations \
    -fsee \
    -ftracer \

endif

ifeq ($(NEON), 1)
  # Enable application wide NEON optimization
  APP_CFLAGS += -mfpu=neon -mfloat-abi=softfp
else
  # Fallback to vfp3-d16 for ARMv7 cpus that
  # don't support NEON.
  # Tegra2, for instance, doesn't have a full
  # VFPv3 with 32 registers, only 16.
  APP_CFLAGS += -mfpu=vfpv3-d16 -mfloat-abi=softfp
endif

# Makefiles
include $(O3D_DIR)/build/make/o3d-imports.mk
include $(O3D_DIR)/build/make/o3d-protobuf.mk
