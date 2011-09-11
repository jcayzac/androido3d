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
#include "core/cross/types.h"

namespace o3d_utils {
	class ViewInfo;
} // o3d_utils

namespace o3d {
	class Renderer;

	namespace extra {

		/** @brief Unprojects a point from 2D screen coordinates to 3D view cordinates.
		 *
		 * @param view The view in which we want to unproject our point.
		 * @param renderer Our renderer.
		 * @param screenSpaceCoordinates Screen space coordinates. z==0 matches zNear in view space.
		 *
		 * @return View space coordinates for the point.
		 */
		Point3 unprojectPoint(
		    const ::o3d_utils::ViewInfo& view,
		    const Renderer& renderer,
		    const Point3& screenSpaceCoordinates
		);

	} // extra
} // o3d

/* vim: set sw=2 ts=2 sts=2 expandtab ff=unix: */
