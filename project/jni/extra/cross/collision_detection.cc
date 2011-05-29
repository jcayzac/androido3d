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

#include "extra/cross/collision_detection.h"

#include "core/cross/primitive.h"
#include "core/cross/types.h"
#include "core/cross/param.h"
#include "core/cross/transform.h"

static inline bool degenerate(const o3d::Point3& pmin, const o3d::Point3& pmax) {
	return (pmin[0]>=pmax[0] || pmin[1]>=pmax[1] || pmin[2]>=pmax[2]);
}

namespace o3d_extra {

bool CollisionDetection::ComputeIntersection(
	const o3d::Transform& entity1,
	const o3d::Transform& entity2,
	o3d::BoundingBox& collisionZone,
	TPrecisionLevel precisionLevel) {

	o3d::Matrix4 entity1Transform = entity1.world_matrix();
	o3d::Matrix4 entity2Transform = entity2.world_matrix();

	o3d::BoundingBox entity1BB, entity2BB;
	entity1.bounding_box().Mul(entity1Transform, &entity1BB);
	entity2.bounding_box().Mul(entity2Transform, &entity2BB);

	// Build intersection volume
	o3d::Point3 pmin(Vectormath::Aos::maxPerElem(entity1BB.min_extent(), entity2BB.min_extent()));
	o3d::Point3 pmax(Vectormath::Aos::minPerElem(entity1BB.max_extent(), entity2BB.max_extent()));
	o3d::BoundingBox intersection(pmin,pmax);
	if (degenerate(pmin, pmax)) return false;

	if (precisionLevel==PRECISION_LEVEL_ENTITIES) return true;

	const o3d::ShapeRefArray& entity1Shapes(entity1.GetShapeRefs());
	const o3d::ShapeRefArray& entity2Shapes(entity2.GetShapeRefs());

	// Iterate thru elements of the 1st entity
	collisionZone=o3d::BoundingBox();
	for(size_t shape1idx(0); shape1idx<entity1Shapes.size(); ++shape1idx) {
		const o3d::ElementRefArray& elements1(entity1Shapes[shape1idx]->GetElementRefs());
		for(size_t elem1idx(0); elem1idx<elements1.size(); ++elem1idx) {
			o3d::Primitive& elem1(*static_cast<o3d::Primitive*>(elements1[elem1idx].Get()));

			o3d::BoundingBox elem1BB;
			elem1.GetBoundingBox(0, &elem1BB);
			elem1BB.Mul(entity1Transform, &elem1BB);

			// Refine the box by only keeping the part that intersects with both entities
			pmin=Vectormath::Aos::maxPerElem(intersection.min_extent(), elem1BB.min_extent());
			pmax=Vectormath::Aos::minPerElem(intersection.max_extent(), elem1BB.max_extent());
			elem1BB = o3d::BoundingBox(pmin,pmax);
			if (degenerate(pmin,pmax)) continue;

			// Iterate thru elements of the 2nd entity
			for(size_t shape2idx(0); shape2idx<entity2Shapes.size(); ++shape2idx) {
				const o3d::ElementRefArray& elements2(entity2Shapes[shape2idx]->GetElementRefs());
				for(size_t elem2idx(0); elem2idx<elements2.size(); ++elem2idx) {
					o3d::Primitive& elem2(*static_cast<o3d::Primitive*>(elements2[elem2idx].Get()));

					// Get the element's OBB and tranform it into an AABB
					o3d::BoundingBox elem2BB;
					elem2.GetBoundingBox(0, &elem2BB);
					elem2BB.Mul(entity2Transform, &elem2BB);

					// Refine the box by only keeping the part that intersects with elem1's also refined box
					pmin=Vectormath::Aos::maxPerElem(elem1BB.min_extent(), elem2BB.min_extent());
					pmax=Vectormath::Aos::minPerElem(elem1BB.max_extent(), elem2BB.max_extent());
					elem2BB = o3d::BoundingBox(pmin,pmax);
					if (degenerate(pmin,pmax)) continue;

					// Here we could test for triangle/triangle intersection
					// between elem1 and elem2 inside elem2SubAABB, but that would be overkill…

					// Grow our collision volume
					collisionZone.Add(elem2BB, &collisionZone);

					if (precisionLevel==PRECISION_LEVEL_PRIMITIVES_ANY) return true;
				}
			}
		}
	}

	return collisionZone.valid();
};

}

