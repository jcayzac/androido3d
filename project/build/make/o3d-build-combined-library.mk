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

$(call assert-defined,\
TARGET_OUT TARGET_OBJS TARGET_ARCH_ABI TOOLCHAIN_PREFIX \
O3D_DIR O3D_COMBINED_LIBRARY O3D_MODULES \
)

get-static-lib-path  = $(addprefix $(TARGET_OUT)/lib, $(addsuffix .a, $1))
get-install-lib-path = $(addprefix $(O3D_DIR)/libs/$(TARGET_ARCH_ABI)/lib, $(addsuffix .a, $1))

all: $(call get-install-lib-path,$(O3D_COMBINED_LIBRARY))

$(call get-install-lib-path,$(O3D_COMBINED_LIBRARY)): $(call get-static-lib-path,$(O3D_COMBINED_LIBRARY))
	@if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi
	@cp $< $@

$(call get-static-lib-path,$(O3D_COMBINED_LIBRARY)): $(call get-static-lib-path,$(O3D_MODULES))
	@if [ -d $(TARGET_OBJS)/$(O3D_COMBINED_LIBRARY) ]; then rm -rf $(TARGET_OBJS)/$(O3D_COMBINED_LIBRARY); fi
	@$(foreach lib,$(O3D_MODULES), \
		mkdir -p $(TARGET_OBJS)/$(O3D_COMBINED_LIBRARY)/$(lib) && \
		cd $(TARGET_OBJS)/$(O3D_COMBINED_LIBRARY)/$(lib) && \
		$(TOOLCHAIN_PREFIX)ar x $(call get-static-lib-path,$(lib)); \
	)
	@if [ -f $@ ]; then rm $@; fi
	@find $(TARGET_OBJS)/$(O3D_COMBINED_LIBRARY) -iname "*.o" -print0 | xargs -0 $(TOOLCHAIN_PREFIX)ar qcs $@
