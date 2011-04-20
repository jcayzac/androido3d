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

#include "extra/cross/ray_primitive_intersection.h"
#include <float.h>
#include <cmath>

namespace o3d{
namespace extra {

RayPrimitiveIntersectionFunctor::RayPrimitiveIntersectionFunctor
(const Point3& origin, const Vector3& direction)
: origin(origin)
, direction(direction)
, hasIntersection(false)
{
  *distances=FLT_MAX;
}

void RayPrimitiveIntersectionFunctor::ProcessTriangle
(
  unsigned int primitive_index,
  const Point3& p0,
  const Point3& p1,
  const Point3& p2
)
{
  // See "Fast, Minimum Storage Ray/Triangle Intersection",
  // by Tomas Möller and Ben Trumbore,
  // Journal of Graphics Tools, 2(1):21--28, 1997.
  // http://www.cs.virginia.edu/~gfx/Courses/2003/ImageSynthesis/papers/Acceleration/Fast%20MinimumStorage%20RayTriangle%20Intersection.pdf
  //
  // Here I implemented their method using Vectormath, and
  // made it branchless…

  const Vector3 edge1(p1-p0);
  const Vector3 edge2(p2-p0);
  const Vector3 pvec(cross(direction, edge2));
  const Vector3 tvec(origin-p0);
  const Vector3 qvec(cross(tvec, edge1));
  const float det(dot(edge1, pvec));
  const float inv_det(1.f/det);
  const Vector3 tuv(
    Vector3(
      dot(edge2, qvec),
      dot(tvec, pvec),
      dot(direction, qvec)
    )
    *
    inv_det
  );
  distances[1]=tuv.getX();
  const bool andWeVeGotAHit(
    // I can't use 'isfinite(distances[1])' here as it would be
    // optimized away when compiling with -ffast-math.
    // Instead I must check if 'det' was a decent divider.
    //
    // NB: It looks like isfinite() works fine with -ffast-math
    // if using a recent GCC, but this goes against stardard.
    (std::abs(det)>.000001f) &&
    // I only want to detect intersection facing my ray, not those
    // which are behind its origin (FIXME: is this check needed?)
    (distances[1]>=.0f) &&
    // Ray intersected only if 0<=u<=1, 0<=v, and u+v<=1
    (tuv.getY()>=.0f) &&
    (tuv.getY()<=1.f) &&
    (tuv.getZ()>=.0f) &&
    ((tuv.getY()+tuv.getZ())<=1.f)
  );

  // if we've got a hit, do distances[0]=min(distances[0],distances[1])
  *distances = std::min(*distances, distances[andWeVeGotAHit]);
  hasIntersection=hasIntersection||andWeVeGotAHit;
}

} // extra
} // o3d

/* vim: set sw=2 ts=2 sts=2 expandtab ff=unix: */
