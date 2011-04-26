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

LOCAL_PATH := $(O3D_DIR)/libs/$(TARGET_ARCH_ABI)
include $(CLEAR_VARS)

# Name and path of the combined static library
O3D_STATIC_LIBRARY_NAME := o3dcombined
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
  ifeq ($(NEON), 1)
    O3D_STATIC_LIBRARY_NAME := $(addsuffix -neon, $(O3D_STATIC_LIBRARY_NAME))
  endif
endif
ifneq ($(NDK_DEBUG), 0)
  O3D_STATIC_LIBRARY_NAME := $(addsuffix -debug, $(O3D_STATIC_LIBRARY_NAME))
else
  O3D_STATIC_LIBRARY_NAME := $(addsuffix -release, $(O3D_STATIC_LIBRARY_NAME))
endif

LOCAL_MODULE := o3dcombined
LOCAL_SRC_FILES := lib$(O3D_STATIC_LIBRARY_NAME).a
LOCAL_EXPORT_LDLIBS := -llog -lGLESv2 -ldl
include $(PREBUILT_STATIC_LIBRARY)
