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
#include <float.h>

namespace o3d {
	class Transform;
	class Renderer;
}

namespace o3d_utils {
	class ViewInfo;
}

namespace o3d_extra {

	/** @brief Find the intersection between a ray and the scenegraph.
	 *
	 * @param rayOrigin			World-space coordinates of ray's origin.
	 * @param rayDirection			World-space direction of ray.
	 * @param root					Root transform of the tree.
	 * @param intersectionPoint	Filled with World-space coordinates of closest intersection point, if any.
	 * @param intersectionDistance	Filled with distance from ray's origin to closest intersection point, if any.
	 * @param maxDistance			Maximum distance, beyond which any intersection point will be disregarded.
	 * @return <code>Transform</code> object the intersected geometry is part of, or <code>NULL</code> if none was found.
	 */
	o3d::Transform* intersectRayWithTree
	(
		const o3d::Point3& rayOrigin,
		const o3d::Vector3& rayDirection,
		o3d::Transform& root,
		o3d::Point3& intersectionPoint,
		float& intersectionDistance,
		float maxDistance=FLT_MAX
	);

	/** @brief Select nearest entity rendered at a certain pixel position, based on its static geometry.
	 *
	 * @param view The view in which the entity is rendered>
	 * @param renderer Our renderer.
	 * @param clientX X component of the screen space coordinates.
	 * @param clientY Y component of the screen space coordinates.
	 * @param intersectionPoint World-space coordinates of the intersection point between pixel coordinates and an entity, if any.
	 * @param intersectionDistance Distance between the X,Y point on the view plane and the intersection, in world units.
	 */
	o3d::Transform* pickUsingStaticGeometry
	(
		const o3d_utils::ViewInfo& view,
		const o3d::Renderer& renderer,
		int clientX,
		int clientY,
		o3d::Point3& intersectionPoint,
		float& intersectionDistance
	) __attribute__((deprecated));

}
