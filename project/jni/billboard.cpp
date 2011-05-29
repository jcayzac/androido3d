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

#include "billboard.h"
#include "render_graph.h"
#include "core/cross/primitive.h"
#include "core/cross/sampler.h"
#include "primitives.h"

namespace o3d_utils {
	using namespace o3d;

static const char* const kBillboardShader =
	"uniform mat4 worldViewProjection;\n"
	"uniform mat4 billboard;\n"
	"uniform float aspectRatio;\n"
	"uniform vec2 texCoordScale;\n"
	"uniform vec2 texCoordOffset;\n"
	"uniform vec2 zAxisRotX;\n"
	"uniform vec2 zAxisRotY;\n"
	"\n"
	"attribute vec4 position;\n"
	"attribute vec2 texCoord0;\n"
	"varying vec2 v_texCoord;\n"
	"\n"
	"void main() {\n"
	"  vec4 pos = position;\n"
	"  vec2 spos = vec2( pos.x, aspectRatio * pos.y );\n"
	"  pos.x = dot(zAxisRotX, spos.xy);\n"
	"  pos.y = dot(zAxisRotY, spos.xy);\n"
	"  gl_Position = worldViewProjection * billboard * pos;\n"
	"  v_texCoord = texCoord0 * texCoordScale + texCoordOffset;\n"
	"}\n"
	"\n"
	"// #o3d SplitMarker\n"
	"\n"
	"uniform vec4 colorMult;\n"
	"uniform sampler2D texSampler0;\n"
	"varying vec2 v_texCoord;\n"
	"\n"
	"void main() {\n"
	"  gl_FragColor = texture2D(texSampler0, v_texCoord) * colorMult;\n"
	"}\n"
	"// #o3d MatrixLoadOrder RowMajor\n"
	"";

static const char* const kBillboardShapeName = "__BillboardShape";
static const char* const kBillboardPrimitiveName = "__BillboardPrimitive";
static const char* const kBillboardEffectName = "__BillboardEffect";
static const char* const kBillboardMaterialName = "__BillboardMaterial";
static const char* const kBillboardTransformFlag = "__isBillboard";

static Shape* GetBillboardPlaneShape( Pack *pack, ViewInfo *view_info )
{
	DCHECK(pack);
	DCHECK(view_info);
	
	// Let's assume if we find the shape then everything else is already created
	// and conversely if it's not found then nothing is created.
	// A shape is valid for a combination of a Pack and a DrawList so we must
	// create/search for a shape in the Pack with a unique name per DrawList.
	std::stringstream shapeName;
	shapeName << kBillboardShapeName << view_info->z_ordered_draw_pass_info()->draw_list();
	std::vector<Shape*> shapes = pack->Get<Shape>(shapeName.str());
	if (!shapes.empty()) {
		return shapes[0];
	}
	
	// Create the effect.
	Effect* effect = pack->Create<Effect>();
	effect->set_name(kBillboardEffectName);
	bool success = effect->LoadFromFXString(kBillboardShader);
	DCHECK(success);
	
	// Create the material.
	Material* material = pack->Create<Material>();
	material->set_name(kBillboardMaterialName);
	material->set_draw_list(view_info->z_ordered_draw_pass_info()->draw_list());
	material->set_effect(effect);
	effect->CreateUniformParameters(effect);
	
	// Create the plane.
	Matrix4 mat(Vector4(1.0f, 0.0f, 0.0f, 0.0f),
				Vector4(0.0f, 0.0f, 1.0f, 0.0f),
				Vector4(0.0f,-1.0f, 0.0f, 0.0f),
				Vector4(0.0f, 0.0f, 0.0f, 1.0f));
	Primitive* prim = Primitives::CreatePlane(pack, 1, 1, 1, 1, &mat);
	prim->set_name(kBillboardPrimitiveName);
	prim->set_material(material);
	prim->set_cull(false);
	
	// Create the shape.
	Shape* shape = pack->Create<Shape>();
	prim->SetOwner(shape);
	shape->set_name(shapeName.str());
	return shape;
}

bool MakeBillboard(Transform *transform,
				   Pack *pack,
				   ViewInfo *viewInfo,
				   Sampler *sampler,
				   float x, float y, float z,
				   float width, float height)
{
	DCHECK(transform);
	DCHECK(pack);
	DCHECK(viewInfo);
	DCHECK(sampler);

	// Flag the transform a billboard by defining this boolean param
	transform->CreateParam<ParamBoolean>(kBillboardTransformFlag)->set_value(true);
	
	MakeScaledQuadParams(transform, pack, sampler, x, y, z, width, height);
	
	Shape* shape = GetBillboardPlaneShape(pack, viewInfo);
	transform->AddShape(shape);
	
	return true;
}
	
Id GenerateBillboard(Pack *pack,
					 ViewInfo *viewInfo,
					 Transform *parent,
					 Sampler *sampler,
					 float x, float y, float z,
					 float width, float height)
{
	DCHECK(pack);
	DCHECK(viewInfo);
	DCHECK(parent);
	DCHECK(sampler);
	
	// Create the transform for positioning
	Transform *transform = pack->Create<Transform>();
	
	// Set the parent.
	transform->SetParent(parent);

	// Fill in the data/parameters to make it a billboard.
	MakeBillboard(transform, pack, viewInfo, sampler, x, y, z, width, height);
	
	return transform->id();
}
	
bool DeleteBillboard(Pack *pack, Id bid)
{
	DCHECK(pack);
	Transform *transform = down_cast<Transform*>(pack->GetObjectBaseById(bid, Transform::GetApparentClass()));
	DCHECK(transform);
	transform->SetParent(NULL);
	return pack->RemoveObject(transform);
}

bool isBillBoardTransform(const Transform *transform)
{
	// The transform is a billboard if it has the special param defined.
	// Checking if the param exists is sufficient so the value is ignored.
	DCHECK(transform);
	return(transform->GetParam<ParamBoolean>(kBillboardTransformFlag) != NULL);
}

}  // namespace o3d_utils
