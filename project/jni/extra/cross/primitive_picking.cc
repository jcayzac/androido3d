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

#include "extra/cross/primitive_picking.h"
#include "extra/cross/ray_primitive_intersection.h"
#include "extra/cross/unproject.h"
#include "render_graph.h"
#include "core/cross/transform.h"
#include "core/cross/primitive.h"

o3d::Transform* o3d_extra::intersectRayWithTree
(
	const o3d::Point3& rayOrigin,
	const o3d::Vector3& rayDirection,
	o3d::Transform& root,
	o3d::Point3& intersectionPoint,
	float& intersectionDistance,
	float maxDistance
) {
	o3d::Transform* bestHit(0);
	intersectionDistance=maxDistance;

	const o3d::Matrix4 toModelSpace(affineInverse(root.world_matrix())); // Assumes affine transform
	const o3d::Point3 rayLocalOrigin((toModelSpace * rayOrigin).getXYZ());
	const o3d::Vector3 rayLocalDirection(toModelSpace.getUpper3x3() * rayDirection);

	// Test ray against bounding box
	o3d::RayIntersectionInfo rii;
	root.bounding_box().IntersectRay(rayLocalOrigin, rayLocalOrigin+rayLocalDirection, &rii);
	if (!rii.intersected()) return 0;

	// Test against inner elements
	const o3d::ShapeRefArray& shapes(root.GetShapeRefs());
	for(size_t shape_index(0); shape_index<shapes.size(); ++shape_index) {
		const o3d::Shape& shape(*shapes[shape_index]);
		const o3d::ElementRefArray& elements(shape.GetElementRefs());
		for(size_t element_index(0); element_index<elements.size(); ++element_index) {
			o3d::Element* element(elements[element_index]);

			// Test against the element's bounding box
			o3d::BoundingBox aabb;
			element->GetBoundingBox(0, &aabb);
			aabb.IntersectRay(rayLocalOrigin, rayLocalOrigin+rayLocalDirection, &rii);
			if (!rii.intersected()) continue;

			// Test against geometry
			o3d::Primitive* primitive(reinterpret_cast<o3d::Primitive*>(element));
			if (primitive) {
				o3d_extra::RayPrimitiveIntersectionFunctor rpif(rayLocalOrigin, rayLocalDirection);
				if (primitive->WalkPolygons(0, &rpif)) {
					if (intersectionDistance>rpif.distance()) {
						intersectionDistance=rpif.distance();
						intersectionPoint=rayOrigin + rayDirection*rpif.distance();
						bestHit=&root;
					}
				}
			}
		}
	}

	// Then, test against childrens
	const o3d::TransformRefArray& children(root.GetChildrenRefs());
	for (size_t entity_index(0); entity_index<children.size(); ++entity_index) {
		o3d::Transform& child(*children[entity_index]);
		o3d::Point3 childIntersection;
		o3d::Transform* hit = intersectRayWithTree(rayOrigin, rayDirection, child, childIntersection, intersectionDistance, intersectionDistance);
		if (hit) {
			intersectionPoint=childIntersection;
			bestHit = hit;
		}
	}

	return bestHit;
}

o3d::Transform* o3d_extra::pickUsingStaticGeometry
(
	const o3d_utils::ViewInfo& view,
	const o3d::Renderer& renderer,
	int clientX,
	int clientY,
	o3d::Point3& intersectionPoint,
	float& intersectionDistance
) {
	// Get the root of the scenegraph
	o3d::Transform& root(*view.tree_root());

	// Build World-space ray from screen-space coordinates
	const o3d::Point3 rayWorldOrigin(unprojectPoint(view,renderer,o3d::Point3(clientX,clientY,0)));
	const o3d::Point3 rayWorldEnd(unprojectPoint(view,renderer,o3d::Point3(clientX,clientY,1)));
	const o3d::Vector3 rayWorldDirection(normalize(o3d::Vector3(rayWorldEnd-rayWorldOrigin)));

	// Call the real method.
	return intersectRayWithTree(rayWorldOrigin, rayWorldDirection, root, intersectionPoint, intersectionDistance);
}
