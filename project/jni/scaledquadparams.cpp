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

#include "scaledquadparams.h"
#include "core/cross/sampler.h"

#define FAST_SCALEDQUADPARAM_CREATION

#ifdef FAST_SCALEDQUADPARAM_CREATION
#define GET_PARAM	GetParam
#else
#define	GET_PARAM	GetOrCreateParam
#endif

namespace o3d_utils {
	using namespace o3d;

static const char* const kScaledQuadTransformFlag = "__isScaledQuad";
static const char* const kScaledQuadAspectName = "aspectRatio";
static const char* const kScaledQuadColorName = "colorMult";
static const char* const kScaledQuadSamplerName = "texSampler0";
static const char* const kScaledQuadTexOffsetName = "texCoordOffset";
static const char* const kScaledQuadTexScaleName = "texCoordScale";
static const char* const kScaledQuadZAxisRotXName = "zAxisRotX";
static const char* const kScaledQuadZAxisRotYName = "zAxisRotY";

static void SetBoundingBox(Transform *transform, float width, float height)
{
	// Expanded bounding box to account for application of billboarding matrix
	// Use a cube containing the sphere containing the unit cube:
	//     sqrt((0.5^2) + (0.5^2)) = 0.707106781
	// Tight bbox would be:
	//   min = (-0.5f, -0.5f*aspect, 0.0f)
	//   max = ( 0.5f,  0.5f*aspect, 0.0f)
	float halfsize = 0.707106781f;
	float aspect = height/width;
	BoundingBox bbox(Point3(-halfsize, -halfsize*aspect, -halfsize),
					 Point3( halfsize,  halfsize*aspect,  halfsize));
	transform->set_bounding_box(bbox);
}

bool MakeScaledQuadParams(Transform *transform,
						  Pack *pack,
						  Sampler *sampler,
						  float x, float y, float z,
						  float width, float height)
{
	DCHECK(transform);
	DCHECK(pack);
	DCHECK(sampler);

	// Flag the transform a scaled quad by defining this boolean param
	transform->CreateParam<ParamBoolean>(kScaledQuadTransformFlag)->set_value(true);
	
#ifdef FAST_SCALEDQUADPARAM_CREATION
	// Setup the position.
	Matrix4 mat(Matrix4::identity());
	mat.setElem(0, 0, width);
	mat.setElem(1, 1, width);
	mat.setElem(2, 2, width);
	mat.setTranslation(o3d::Vector3(x, y, z));
	transform->set_local_matrix(mat);
	transform->CreateParam<ParamFloat>(kScaledQuadAspectName)->set_value(height/width);
	transform->CreateParam<ParamFloat4>(kScaledQuadColorName)->set_value(Float4(1.0f, 1.0f, 1.0f, 1.0f));
	transform->CreateParam<ParamSampler>(kScaledQuadSamplerName)->set_value(sampler);
	transform->CreateParam<ParamFloat2>(kScaledQuadTexOffsetName)->set_value(Float2(0.0f, 0.0f));
	transform->CreateParam<ParamFloat2>(kScaledQuadTexScaleName)->set_value(Float2(1.0f, 1.0f));
	transform->CreateParam<ParamFloat2>(kScaledQuadZAxisRotXName)->set_value(Float2(1.0f,0.0f));
	transform->CreateParam<ParamFloat2>(kScaledQuadZAxisRotYName)->set_value(Float2(0.0f,1.0f));
#else
	Matrix4 mat(Matrix4::identity());
	transform->set_local_matrix(mat);
	SetPosition(pack, transform->id(), x, y, z);
	SetSize(pack, transform->id(), width, height);
	SetColor(pack, transform->id(), 1.0f, 1.0f, 1.0f, 1.0f);
	SetSampler(pack, transform->id(), sampler);
	SetTexcoords(pack, transform->id(), 0.0f, 0.0f, 1.0f, 1.0f);
	SetZAxisRotationRadians(pack, transform->id(), 0.0f);
#endif

	SetBoundingBox(transform, width, height);
	transform->set_cull(true);
	
	return true;
}

bool isScaledQuadTransform(const Transform *transform)
{
	// The transform is a scaled quad if it has the special param defined.
	// Checking if the param exists is sufficient so the value is ignored.
	DCHECK(transform);
	return(transform->GetParam<ParamBoolean>(kScaledQuadTransformFlag) != NULL);
}

void SetSampler(Transform *transform, Sampler *sampler)
{
	DCHECK(isScaledQuadTransform(transform));
	transform->GET_PARAM<ParamSampler>(kScaledQuadSamplerName)->set_value(sampler);
}
	
void SetSampler(Pack *pack, Id bid, Sampler *sampler)
{
	DCHECK(pack);
	Transform *transform = down_cast<Transform*>(pack->GetObjectBaseById(bid, Transform::GetApparentClass()));
	SetSampler(transform, sampler);
}

void SetTexcoords(Transform *transform, float x, float y, float w, float h)
{
	DCHECK(isScaledQuadTransform(transform));
	transform->GET_PARAM<ParamFloat2>(kScaledQuadTexOffsetName)->set_value(Float2(x,y+h));
	transform->GET_PARAM<ParamFloat2>(kScaledQuadTexScaleName)->set_value(Float2(w,-h));
}
		
void SetTexcoords(Pack *pack, Id bid, float x, float y, float w, float h)
{
	DCHECK(pack);
	Transform *transform = down_cast<Transform*>(pack->GetObjectBaseById(bid, Transform::GetApparentClass()));
	SetTexcoords(transform, x, y, w, h);
}
	
void SetPosition(Transform *transform, float x, float y, float z)
{
	DCHECK(isScaledQuadTransform(transform));
	Matrix4 mat( transform->local_matrix() );
	mat.setTranslation(o3d::Vector3(x, y, z));
	transform->set_local_matrix(mat);
}
	
void SetPosition(Pack *pack, Id bid, float x, float y, float z)
{
	DCHECK(pack);
	Transform *transform = down_cast<Transform*>(pack->GetObjectBaseById(bid, Transform::GetApparentClass()));
	SetPosition(transform, x, y, z);
}

void SetSize(Transform *transform, float w, float h)
{
	DCHECK(isScaledQuadTransform(transform));
	o3d::Matrix4 matrix(transform->local_matrix());
	matrix.setElem(0, 0, w);
	matrix.setElem(1, 1, w);
	matrix.setElem(2, 2, w);
	transform->set_local_matrix(matrix);
	transform->GET_PARAM<ParamFloat>(kScaledQuadAspectName)->set_value(h/w);
	SetBoundingBox(transform, w, h);
}
		
void SetSize(Pack *pack, Id bid, float w, float h)
{
	DCHECK(pack);
	Transform *transform = down_cast<Transform*>(pack->GetObjectBaseById(bid, Transform::GetApparentClass()));
	SetSize(transform, w, h);
}

void SetColor(Transform *transform, float r, float g, float b, float a)
{
	DCHECK(isScaledQuadTransform(transform));
	transform->GET_PARAM<ParamFloat4>(kScaledQuadColorName)->set_value(Float4(r, g, b, a));
}

void SetColor(Pack *pack, Id bid, float r, float g, float b, float a)
{
	DCHECK(pack);
	Transform *transform = down_cast<Transform*>(pack->GetObjectBaseById(bid, Transform::GetApparentClass()));
	SetColor(transform, r, g, b, a);
}	

void SetZAxisRotationRadians(Transform *transform, float radians)
{
	DCHECK(isScaledQuadTransform(transform));
	float cval = cosf(radians);
	float sval = sinf(radians);
	transform->GET_PARAM<ParamFloat2>(kScaledQuadZAxisRotXName)->set_value(Float2(cval,-sval));
	transform->GET_PARAM<ParamFloat2>(kScaledQuadZAxisRotYName)->set_value(Float2(sval,cval));
}

void SetZAxisRotationRadians(Pack *pack, Id bid, float radians)
{
	DCHECK(pack);
	Transform *transform = down_cast<Transform*>(pack->GetObjectBaseById(bid, Transform::GetApparentClass()));
	SetZAxisRotationRadians(transform, radians);
}
	
}  // namespace o3d_utils