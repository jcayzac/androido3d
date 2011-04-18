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

include $(CLEAR_VARS)

# Root of 3rd party libraries
O3D_THIRD_PARTY := $(O3D_NATIVE_DIR)/third_party

# Common CFLAGS for all modules
LOCAL_CFLAGS :=

# Treat warnings as errors, except in 3rd party source code (which is hopeless)
ifneq ($(filter $(O3D_THIRD_PARTY)/%,$(LOCAL_PATH)), $(LOCAL_PATH))
  LOCAL_CFLAGS += -Werror
endif

LOCAL_C_INCLUDES := \
  $(LOCAL_PATH) \
  $(O3D_THIRD_PARTY) \
  $(O3D_THIRD_PARTY)/zlib/include \
