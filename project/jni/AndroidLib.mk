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

include $(CLEAR_VARS)

LOCAL_MODULE    := o3dhelp
LOCAL_CFLAGS    := \
  -Werror \
  -D__ANDROID__ \
  -DRENDERER_GLES2 \
  -DGLES2_BACKEND_NATIVE_GLES2 \
  -I$(LOCAL_PATH)/third_party/stlport/stlport \
  -I$(LOCAL_PATH)/third_party/loggingshim \

LOCAL_SRC_FILES := \
  camera.cpp \
  debug.cpp \
  image_plane.cpp \
  primitives.cpp \
  render_graph.cpp \
  scene.cpp \
  shader_builder.cpp \

include $(BUILD_STATIC_LIBRARY)

#### Wchar functions
#
include $(CLEAR_VARS)

LOCAL_MODULE    := o3dwchar

LOCAL_CFLAGS    := \
  -Werror \
  -D__ANDROID__ \
  -DRENDERER_GLES2 \
  -DGLES2_BACKEND_NATIVE_GLES2 \
  -I$(LOCAL_PATH)/third_party/stlport/stlport \
  -I$(LOCAL_PATH)/third_party/loggingshim \

LOCAL_SRC_FILES := \
  wchar.cpp \

include $(BUILD_STATIC_LIBRARY)

