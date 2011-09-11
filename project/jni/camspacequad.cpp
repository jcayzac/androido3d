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

#include "camspacequad.h"
#include "render_graph.h"
#include "core/cross/primitive.h"
#include "core/cross/sampler.h"
#include "primitives.h"

namespace o3d_utils {
	using namespace o3d;

	static const char* const kCamSpaceQuadShader =
	    "uniform mat4 worldViewProjection;\n"
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
	    "// This shader requires the view matrix to be the identity.\n"
	    "void main() {\n"
	    "  vec4 pos = position;\n"
	    "  vec2 spos = vec2( pos.x, aspectRatio * pos.y );\n"
	    "  pos.x = dot(zAxisRotX, spos.xy);\n"
	    "  pos.y = dot(zAxisRotY, spos.xy);\n"
	    "  gl_Position = worldViewProjection * pos;\n"
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

	static const char* const kCamSpaceQuadShapeName = "__CamSpaceQuadShape";
	static const char* const kCamSpaceQuadPrimitiveName = "__CamSpaceQuadPrimitive";
	static const char* const kCamSpaceQuadEffectName = "__CamSpaceQuadEffect";
	static const char* const kCamSpaceQuadMaterialName = "__CamSpaceQuadMaterial";
	static const char* const kCamSpaceQuadTransformFlag = "__isCamSpaceQuad";

	static Shape* GetCameraSpaceQuadPlaneShape(Pack* pack, ViewInfo* view_info) {
		O3D_ASSERT(pack);
		O3D_ASSERT(view_info);
		// Let's assume if we find the shape then everything else is already created
		// and conversely if it's not found then nothing is created.
		std::vector<Shape*> shapes = pack->Get<Shape>(kCamSpaceQuadShapeName);

		if(!shapes.empty()) {
			return shapes[0];
		}

		// Create the effect.
		Effect* effect = pack->Create<Effect>();
		effect->set_name(kCamSpaceQuadEffectName);
		bool success = effect->LoadFromFXString(kCamSpaceQuadShader);
		O3D_ASSERT(success);
		// Create the material.
		Material* material = pack->Create<Material>();
		material->set_name(kCamSpaceQuadMaterialName);
		material->set_draw_list(view_info->z_ordered_draw_pass_info()->draw_list());
		material->set_effect(effect);
		effect->CreateUniformParameters(effect);
		// Create the plane.
		Matrix4 mat(Vector4(1.0f, 0.0f, 0.0f, 0.0f),
		            Vector4(0.0f, 0.0f, 1.0f, 0.0f),
		            Vector4(0.0f, -1.0f, 0.0f, 0.0f),
		            Vector4(0.0f, 0.0f, 0.0f, 1.0f));
		Primitive* prim = Primitives::CreatePlane(pack, 1, 1, 1, 1, &mat);
		prim->set_name(kCamSpaceQuadPrimitiveName);
		prim->set_material(material);
		prim->set_cull(false);
		// Create the shape.
		Shape* shape = pack->Create<Shape>();
		prim->SetOwner(shape);
		shape->set_name(kCamSpaceQuadShapeName);
		return shape;
	}

	bool MakeCameraSpaceQuad(Transform* transform,
	                         Pack* pack,
	                         ViewInfo* viewInfo,
	                         Sampler* sampler,
	                         float x, float y, float z,
	                         float width, float height) {
		O3D_ASSERT(transform);
		O3D_ASSERT(pack);
		O3D_ASSERT(viewInfo);
		O3D_ASSERT(sampler);
		// Flag the transform a camera space quad by defining this boolean param
		transform->CreateParam<ParamBoolean>(kCamSpaceQuadTransformFlag)->set_value(true);
		MakeScaledQuadParams(transform, pack, sampler, x, y, z, width, height);
		Shape* shape = GetCameraSpaceQuadPlaneShape(pack, viewInfo);
		transform->AddShape(shape);
		return true;
	}

	Id GenerateCameraSpaceQuad(Pack* pack,
	                           ViewInfo* viewInfo,
	                           Transform* parent,
	                           Sampler* sampler,
	                           float x, float y, float z,
	                           float width, float height) {
		O3D_ASSERT(pack);
		O3D_ASSERT(viewInfo);
		O3D_ASSERT(parent);
		O3D_ASSERT(sampler);
		// Create the transform for positioning
		Transform* transform = pack->Create<Transform>();
		// Set the parent.
		transform->SetParent(parent);
		// Fill in the data/parameters to make it a camera space quad.
		MakeCameraSpaceQuad(transform, pack, viewInfo, sampler, x, y, z, width, height);
		return transform->id();
	}

	bool DeleteCameraSpaceQuad(Pack* pack, Id bid) {
		O3D_ASSERT(pack);
		Transform* transform = down_cast<Transform*>(pack->GetObjectBaseById(bid, Transform::GetApparentClass()));
		O3D_ASSERT(transform);
		transform->SetParent(NULL);
		return pack->RemoveObject(transform);
	}

	bool isCameraSpaceQuadTransform(const Transform* transform) {
		// The transform is a camera space quad if it has the special param defined.
		// Checking if the param exists is sufficient so the value is ignored.
		O3D_ASSERT(transform);
		return(transform->GetParam<ParamBoolean>(kCamSpaceQuadTransformFlag) != NULL);
	}

}  // namespace o3d_utils
