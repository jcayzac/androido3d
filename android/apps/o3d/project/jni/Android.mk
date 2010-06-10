# Copyright (C) 2009 The Android Open Source Project
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

LOCAL_PATH      := $(call my-dir)
MY_PATH         := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := libo3djni
LOCAL_CFLAGS    := \
  -Werror \
  -D__ANDROID__ \
  -I$(LOCAL_PATH)/third_party/stlport/stlport \
  -I$(LOCAL_PATH)/third_party/loggingshim \

LOCAL_LDLIBS    := -llog -lGLESv2 -lz
LOCAL_SRC_FILES := \
  gl_code.cpp \
  render_graph.cpp \
  shader_builder.cpp \

LOCAL_STATIC_LIBRARIES := \
  o3dcore \
  o3drenderer \
  o3dimport \
  o3dutils \
  fcollada \
  libxml \
  libjpeg \
  libpng \
  zlib \
  loggingshim \
  stlport \


include $(BUILD_SHARED_LIBRARY)

include $(MY_PATH)/core/Android.mk
include $(MY_PATH)/import/Android.mk
include $(MY_PATH)/utils/Android.mk
include $(MY_PATH)/third_party/stlport/Android.mk
include $(MY_PATH)/third_party/fcollada/files/Android.mk
include $(MY_PATH)/third_party/libjpeg/Android.mk
include $(MY_PATH)/third_party/libpng/Android.mk
include $(MY_PATH)/third_party/zlib/Android.mk
include $(MY_PATH)/third_party/loggingshim/Android.mk

#libgame

include $(CLEAR_VARS)

include $(MY_PATH)/game/Android.mk