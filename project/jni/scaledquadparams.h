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

#ifndef O3D_UTILS_SCALEDQUADPARAMS_H_
#define O3D_UTILS_SCALEDQUADPARAMS_H_

#include "core/cross/types.h"
#include "core/cross/param.h"
#include "core/cross/pack.h"

// The handle is just the Id the O3D transform object.

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

	// Modifies a previously created O3D transform to have control parameters for a
	// scaled quad (i.e. billboard or camspacequad).
	// Note: The provided transform should generally be a "newly" created transform
	//       created with: pack->Create<o3d::Transform>();
	//       If the transform was not newly created, the behavior is undefined although
	//       it will likely not be catastrophic in nature.
	bool MakeScaledQuadParams(Transform *transform,
							  Pack *pack,
							  Sampler *sampler,
							  float x, float y, float z,
							  float width, float height);
	
	// Sets the sampler (i.e. texture used with the scaled quad object)
	void SetSampler(Transform *transform, Sampler* sampler);
	void SetSampler(Pack *pack, Id bid, Sampler* sampler);
	
	// Sets the texcoords (i.e. textures used to apply the texture)
	void SetTexcoords(Transform *transform,
					  float x, float y, float width, float height);
	void SetTexcoords(Pack *pack, Id bid,
					  float x, float y, float width, float height);
	
	// Sets the position
	void SetPosition(Transform *transform, float x, float y, float z);
	void SetPosition(Pack *pack, Id bid, float x, float y, float z);
	
	// Sets the size (in world space)
	void SetSize(Transform *transform, float width, float height);
	void SetSize(Pack *pack, Id bid, float width, float height);
	
	// Sets the color used to modulate the texture
	void SetColor(Transform *transform, float r, float g, float b, float a);
	void SetColor(Pack *pack, Id bid, float r, float g, float b, float a);
	
	// Sets the rotation amount about the Z-axis
	void SetZAxisRotationRadians(Transform *transform, float radians);
	void SetZAxisRotationRadians(Pack *pack, Id bid, float radians);
	
	// Function for querying if a transform has the scaled quad parameters.
	bool isScaledQuadTransform(const Transform*);
	
}  // namespace o3d_utils

#endif  // O3D_UTILS_SCALEDQUADPARAMS_H_

