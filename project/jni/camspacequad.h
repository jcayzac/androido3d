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

#ifndef O3D_UTILS_CAMSPACEQUAD_H_
#define O3D_UTILS_CAMSPACEQUAD_H_

#include "scaledquadparams.h"

// The handle for the camera space quad is just the Id the O3D transform
// object used to represent the quad.

namespace o3d {

	class Pack;
	class ParamSampler;
	class Sampler;
	class Shape;
	class Transform;
	class Texture2D;

}  // namespace o3d.

namespace o3d_utils {
	using namespace o3d;

	class ViewInfo;

	// Modifies a previously created O3D transform to be a camera space quad.  This is
	// accomplished by filling the transform with data (shape, effect, material, etc.)
	// and control parameters (i.e. width, height, sampler, etc.) so that the
	// transform will render a quad in camera space.
	// Note: The provided transform should generally be a "newly" created transform
	//       created with: pack->Create<o3d::Transform>();
	//       If the transform was not newly created, the behavior is undefined although
	//       it will likely not be catastrophic in nature.
	bool MakeCameraSpaceQuad(Transform* transform,
	                         Pack* pack,
	                         ViewInfo* viewInfo,
	                         Sampler* sampler,
	                         float x, float y, float z,
	                         float width, float height);

	// Creates a camera space quad (i.e. an O3D transform) and puts it in the tree at parent.
	// Returns the Id of the newly created Transform representing the camera space quad.
	// Note: We can remove x, y, z, width, height parameters and require they are
	//       set with the functions below if that is preferred.
	Id GenerateCameraSpaceQuad(Pack* pack,
	                           ViewInfo* viewInfo,
	                           Transform* parent,
	                           Sampler* sampler,
	                           float x, float y, float z,
	                           float width, float height);

	// Deletes a camera space quad (transform)
	bool DeleteCameraSpaceQuad(Pack* pack, Id bid);

	// Function for querying if a transform is being used to represent a camera space quad.
	bool isCameraSpaceQuadTransform(const Transform*);

}  // namespace o3d_utils

#endif  // O3D_UTILS_CAMSPACEQUAD_H_

