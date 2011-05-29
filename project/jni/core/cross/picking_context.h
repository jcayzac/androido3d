/*
 * Copyright (C) 2010 Tonchidot Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "core/cross/service_implementation.h"
#include "core/cross/param_object.h"

namespace o3d {

class PickingContext {
public:
	static const InterfaceId kInterfaceId;

	explicit PickingContext(ServiceLocator* service_locator)
	: service_(service_locator, this) {}

	// Retrieves the current pickable object.
	ParamObject* pickable() const {
		return pickable_.Get();
	}

	// Sets the current pickable object.
	void set_pickable(ParamObject* pickable) {
		pickable_ = ParamObject::Ref(pickable);
	}

private:
	ServiceImplementation<PickingContext> service_;
	ParamObject::Ref pickable_;
};

}  // namespace o3d
