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

# This Makefile aggregates all the static libraries into a big one.
# The Android build system apparently doesn't support this (building
# a static library project that depends on other static libraries
# doesn't build them), so we're doing it by hand (which is not a
# big deal anyway)

LOCAL_PATH     := $(O3D_NATIVE_DIR)

include $(O3D_START_COMBINED_LIBRARY)
O3D_COMBINED_LIBRARY := o3dcombined-$(_app)

include $(O3D_NATIVE_DIR)/AndroidLib.mk
include $(O3D_NATIVE_DIR)/base/Android.mk
include $(O3D_NATIVE_DIR)/core/Android.mk
include $(O3D_NATIVE_DIR)/import/Android.mk
include $(O3D_NATIVE_DIR)/extra/Android.mk
include $(O3D_NATIVE_DIR)/utils/Android.mk
include $(O3D_NATIVE_DIR)/third_party/fcollada/Android.mk
include $(O3D_NATIVE_DIR)/third_party/libtxc_dxtn/files/Android.mk
include $(O3D_NATIVE_DIR)/third_party/libjpeg/Android.mk
include $(O3D_NATIVE_DIR)/third_party/libpng/Android.mk
include $(O3D_NATIVE_DIR)/third_party/zlib/Android.mk
include $(O3D_NATIVE_DIR)/third_party/lzma/Android.mk
include $(O3D_NATIVE_DIR)/third_party/protobuf/Android.mk
include $(O3D_NATIVE_DIR)/third_party/protobuf-lzma/Android.mk

include $(O3D_BUILD_COMBINED_LIBRARY)
