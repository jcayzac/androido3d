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

LOCAL_MODULE := protobuf
LOCAL_CPP_EXTENSION := .cc

LOCAL_C_INCLUDES += \
  $(LOCAL_PATH)/include \
  $(LOCAL_PATH)/android-config \

LOCAL_SRC_FILES := $(addprefix current/src/google/protobuf/, \
  generated_message_util.cc \
  message_lite.cc \
  repeated_field.cc \
  wire_format_lite.cc \
  stubs/common.cc \
  stubs/once.cc \
  io/zero_copy_stream_impl.cc \
  io/coded_stream.cc \
  io/gzip_stream.cc \
  io/zero_copy_stream.cc \
  io/zero_copy_stream_impl_lite.cc \
)

include $(O3D_BUILD_MODULE)

