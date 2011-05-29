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

namespace o3d {
	class Transform;
	class BoundingBox;
}

namespace o3d_extra {

	/** This class holds collision detection stuff */
	class CollisionDetection {
	public:
		/// Level of precision for when computing the intersection of two entities
		enum TPrecisionLevel {
			/// Only check entities' bounding boxes.
			PRECISION_LEVEL_ENTITIES,
			/// Check that at least 2 primitives' bounding boxes intersect.
			PRECISION_LEVEL_PRIMITIVES_ANY,
			/// Be exhaustive and return a full collision zone.
			PRECISION_LEVEL_PRIMITIVES_ALL,
		};
	public:
		/** @brief Check if two entities intersect
		  *
		  * @param entity1 first entity
		  * @param entity2 second entity
		  * @param collisionZone axis-aligned bounding box of found intersection, if any.
		  * @param precisionLevel precision level:<ul>
		  *		<li>If <code>PRECISION_LEVEL_ENTITIES</code> is used, the method will only
		  *		check the two entities' bounding boxes for collision, and the returned
		  *		<code>collisionZone</code> will be coarse.</li>
		  *		<li>If <code>PRECISION_LEVEL_PRIMITIVES_ANY</code> is used, the method
		  *		will check the two entities's bounding box for an intersection, then will
		  *		try to detect a collision between two of their respective primitives.
		  *		<code>collisionZone</code> will represent the intersection zone of these
		  *		two primitives.</li>
		  *		<li>Lastly, if <code>PRECISION_LEVEL_PRIMITIVES_ALL</code> is used, the
		  *		method will try to detect all colliding primitives, and <code>collisionZone</code>
		  *		will encompass all those.</li>
		  * </ul>
		  * @return true if a collision was found (so collisionZone is valid), false otherwise.
		  */
		static bool ComputeIntersection(const o3d::Transform& entity1,
										const o3d::Transform& entity2,
										o3d::BoundingBox& collisionZone,
										TPrecisionLevel precisionLevel);
	};
}

