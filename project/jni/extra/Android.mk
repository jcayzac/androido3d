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

LOCAL_MODULE := o3dextra
LOCAL_CPP_EXTENSION := .cc

LOCAL_SRC_FILES := \
  $(addprefix cross/, \
    unproject.cc \
    bounding_boxes_extra.cc \
    ray_primitive_intersection.cc \
    primitive_picking.cc \
    collision_detection.cc \
  )

include $(O3D_BUILD_MODULE)
