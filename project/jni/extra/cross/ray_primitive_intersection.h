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

#include "core/cross/primitive.h"

namespace o3d {
	namespace extra {

		/** O3D ray/triangle intersection is broken. Here is a working and faster
		 * implementation, used by the picking code.
		 */
		class RayPrimitiveIntersectionFunctor: public Primitive::PolygonFunctor {
		private:
			const Point3&  origin;    ///< Ray origin, in world space.
			const Vector3& direction; ///< Ray direction, in world space.
			float distances[2];       ///< First distance is the minimum. Second one is the current one.
			bool  hasIntersection;    ///< Set to <code>true</code> when an intersection is found.
		public:

			/** Functor constructor.
			 *
			 * @param origin    Ray origin, in world space.
			 * @param direction  Ray direction, in world space.
			 */
			RayPrimitiveIntersectionFunctor(const Point3& origin, const Vector3& direction);

			/// @return true if an intersection was found, false otherwise.
			bool intersected() const {
				return hasIntersection;
			}

			/// @return the intersection point (undefined if none was found)
			Point3 intersection() const {
				return Point3(origin + direction * (*distances));
			}

			/// @return distance between ray's origin and intersection point, or FLT_MAX if none was found.
			const float& distance() const {
				return *distances;
			}

		private:
			// From Primitive::PolygonFunctor
			void ProcessTriangle(unsigned, const Point3&, const Point3&, const Point3&);
			void ProcessLine(unsigned, const Point3&, const Point3&) {
				/* nothing to do here */
			}
			void ProcessPoint(unsigned, const Point3&) {
				/* nothing to do here */
			}
		};
	} // extra
} // o3d

/* vim: set sw=2 ts=2 sts=2 expandtab ff=unix: */
