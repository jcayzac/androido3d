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

LOCAL_MODULE := protobuf-lzma
LOCAL_CPP_EXTENSION := .cc

LOCAL_C_INCLUDES += \
  $(LOCAL_PATH)/include \
  $(O3D_THIRD_PARTY)/protobuf/include \
  $(O3D_THIRD_PARTY)/lzma/include \

LOCAL_SRC_FILES := current/google/protobuf/io/lzma_protobuf_stream.cc

include $(O3D_BUILD_MODULE)