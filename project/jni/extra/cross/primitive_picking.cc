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

namespace o3d {
namespace extra {

Transform* intersectRayWithTree(
  const Point3& rayOrigin,
  const Vector3& rayDirection,
  Transform& root,
  Point3& intersectionPoint,
  float& intersectionDistance,
  float maxDistance
) {
  Transform* bestHit(0);
  intersectionDistance=maxDistance;

  const Matrix4 toModelSpace(affineInverse(root.world_matrix())); // Assumes affine transform
  const Point3 rayLocalOrigin((toModelSpace * rayOrigin).getXYZ());
  const Vector3 rayLocalDirection(toModelSpace.getUpper3x3() * rayDirection);

  // Test ray against bounding box
  RayIntersectionInfo rii;
  root.bounding_box().IntersectRay(rayLocalOrigin, rayLocalOrigin+rayLocalDirection, &rii);
  if (!rii.intersected()) return 0;

  // Test against inner elements
  const ShapeRefArray& shapes(root.GetShapeRefs());
  for(size_t shape_index(0); shape_index<shapes.size(); ++shape_index) {
    const Shape& shape(*shapes[shape_index]);
    const ElementRefArray& elements(shape.GetElementRefs());
    for(size_t element_index(0); element_index<elements.size(); ++element_index) {
      Element* element(elements[element_index]);

      // Test against the element's bounding box
      BoundingBox aabb;
      element->GetBoundingBox(0, &aabb);
      aabb.IntersectRay(rayLocalOrigin, rayLocalOrigin+rayLocalDirection, &rii);
      if (!rii.intersected()) continue;

      // Test against geometry
      Primitive* primitive(reinterpret_cast<Primitive*>(element));
      if (primitive) {
        RayPrimitiveIntersectionFunctor rpif(rayLocalOrigin, rayLocalDirection);
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
  const TransformRefArray& children(root.GetChildrenRefs());
  for (size_t entity_index(0); entity_index<children.size(); ++entity_index) {
    Transform& child(*children[entity_index]);
    Point3 childIntersection;
    Transform* hit = intersectRayWithTree(rayOrigin, rayDirection, child, childIntersection, intersectionDistance, intersectionDistance);
    if (hit) {
      intersectionPoint=childIntersection;
      bestHit = hit;
    }
  }

  return bestHit;
}

Transform* pickUsingStaticGeometry(
  const ::o3d_utils::ViewInfo& view,
  const Renderer& renderer,
  int clientX,
  int clientY,
  Point3& intersectionPoint,
  float& intersectionDistance
) {
  // Get the root of the scenegraph
  Transform& root(*view.tree_root());

  // Build World-space ray from screen-space coordinates
  const Point3 rayWorldOrigin(unprojectPoint(view,renderer,Point3(clientX,clientY,0)));
  const Point3 rayWorldEnd(unprojectPoint(view,renderer,Point3(clientX,clientY,1)));
  const Vector3 rayWorldDirection(normalize(Vector3(rayWorldEnd-rayWorldOrigin)));

  // Call the real method.
  return intersectRayWithTree(rayWorldOrigin, rayWorldDirection, root, intersectionPoint, intersectionDistance);
}

} // extra
} // o3d

/* vim: set sw=2 ts=2 sts=2 expandtab ff=unix: */
