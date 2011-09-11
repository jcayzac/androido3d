/*
 * Copyright (C) 2009 The Android Open Source Project
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

#include "primitives.h"
#include <vector>
#include "core/cross/buffer.h"
#include "core/cross/pack.h"
#include "core/cross/primitive.h"
#include "core/cross/stream.h"
#include "core/cross/stream_bank.h"

namespace o3d_utils {
	using namespace o3d;

	struct Triangle {
		Triangle(uint32_t i0, uint32_t i1, uint32_t i2) {
			indices[0] = i0;
			indices[1] = i1;
			indices[2] = i2;
		}

		uint32_t& operator[](int index) {
			O3D_ASSERT(index >= 0 && index < 3);
			return indices[index];
		}

		uint32_t indices[3];
	};

	struct Vector2 {
		Vector2(float x, float y) {
			v[0] = x;
			v[1] = y;
		}

		float& operator[](int index) {
			O3D_ASSERT(index >= 0 && index < 2);
			return v[index];
		}

		float v[2];
	};

	/**
	 * Sets the bounding box and z sort point of an element.
	 * @param {!o3d.Element} element Element to set bounding box and z sort point
	 *     on.
	 */
	void Primitives::SetBoundingBoxAndZSortPoint(Element* element) {
		BoundingBox boundingBox;
		element->GetBoundingBox(0, &boundingBox);
		Point3 minExtent = boundingBox.min_extent();
		Point3 maxExtent = boundingBox.max_extent();
		element->set_bounding_box(boundingBox);
		element->set_cull(true);
		element->set_z_sort_point(Float3(
		                              (minExtent.getX() + maxExtent.getX()) / 2.0f,
		                              (minExtent.getY() + maxExtent.getY()) / 2.0f,
		                              (minExtent.getZ() + maxExtent.getZ()) / 2.0f));
	};

	void Primitives::ApplyMatrix(
	    const Matrix4& mat,
	    std::vector<Vector3>* vectors) {
		O3D_ASSERT(vectors);

		for(size_t ii = 0; ii < vectors->size(); ++ii) {
			Vector4 vec(mat * (*vectors)[ii]);
			(*vectors)[ii] = Vector3(vec[0], vec[1], vec[2]);
		}
	}

	void Primitives::ApplyMatrix(
	    const Matrix4& mat,
	    std::vector<Point3>* points) {
		O3D_ASSERT(points);

		for(size_t ii = 0; ii < points->size(); ++ii) {
			Vector4 vec(mat * (*points)[ii]);
			(*points)[ii] = Point3(vec[0], vec[1], vec[2]);
		}
	}

	void Primitives::SetFieldFromVector3s(
	    FloatField* field,
	    const std::vector<Vector3>& vectors) {
		::o3d::base::scoped_array<float> floats(new float[vectors.size() * 3]);

		for(size_t ii = 0; ii < vectors.size(); ++ii) {
			size_t off = ii * 3;
			const Vector3& src = vectors[ii];
			floats[off + 0] = src[0];
			floats[off + 1] = src[1];
			floats[off + 2] = src[2];
		}

		field->SetFromFloats(floats.get(), 3, 0, vectors.size());
	}

	void Primitives::SetFieldFromPoint3s(
	    FloatField* field,
	    const std::vector<Point3>& points) {
		::o3d::base::scoped_array<float> floats(new float[points.size() * 3]);

		for(size_t ii = 0; ii < points.size(); ++ii) {
			size_t off = ii * 3;
			const Point3& src = points[ii];
			floats[off + 0] = src[0];
			floats[off + 1] = src[1];
			floats[off + 2] = src[2];
		}

		field->SetFromFloats(floats.get(), 3, 0, points.size());
	}

	Primitive* CreatePrimitive(
	    Pack* pack,
	    std::vector<Point3>* positions,
	    std::vector<Vector3>* normals,
	    std::vector<Vector2>* tex_coords,
	    std::vector<Triangle>* indices,
	    Primitive::PrimitiveType primitive_type) {
		O3D_ASSERT(positions);
		O3D_ASSERT(indices);
		O3D_ASSERT(!normals || normals->size() == positions->size());
		O3D_ASSERT(!tex_coords || tex_coords->size() == positions->size());
		size_t num_vertices = positions->size();
		size_t num_indices = indices->size() * 3;
		size_t num_primitives = 0;

		switch(primitive_type) {
		case Primitive::POINTLIST:
			num_primitives = num_indices / 1;
			break;
		case Primitive::LINELIST:
			num_primitives = num_indices / 2;
			break;
		case Primitive::LINESTRIP:
			num_primitives = num_indices - 1;
			break;
		case Primitive::TRIANGLELIST:
			num_primitives = num_indices / 3;
			break;
		case Primitive::TRIANGLESTRIP:
		case Primitive::TRIANGLEFAN:
			num_primitives = num_indices - 2;
			break;
		default:
			O3D_NEVER_REACHED();
		}

		Primitive* prim = pack->Create<Primitive>();
		StreamBank* sb = pack->Create<StreamBank>();
		prim->set_stream_bank(sb);
		prim->set_number_primitives(num_primitives);
		prim->set_number_vertices(num_vertices);
		prim->set_primitive_type(primitive_type);
		prim->CreateDrawElement(pack, NULL);
		VertexBuffer* vb = pack->Create<VertexBuffer>();
		FloatField* position_field = vb->CreateTypedField<FloatField>(3);
		FloatField* normal_field = NULL;
		FloatField* texcoord_field = NULL;

		if(normals) {
			normal_field = vb->CreateTypedField<FloatField>(3);
		}

		if(tex_coords) {
			texcoord_field = vb->CreateTypedField<FloatField>(2);
		}

		vb->AllocateElements(num_vertices);
		Primitives::SetFieldFromPoint3s(position_field, *positions);
		sb->SetVertexStream(Stream::POSITION, 0, position_field, 0);

		if(normals) {
			Primitives::SetFieldFromVector3s(normal_field, *normals);
			sb->SetVertexStream(Stream::NORMAL, 0, normal_field, 0);
		}

		if(tex_coords) {
			texcoord_field->SetFromFloats(&(*tex_coords)[0][0], 2, 0, num_vertices);
			sb->SetVertexStream(Stream::TEXCOORD, 0, texcoord_field, 0);
		}

		IndexBuffer* ib = pack->Create<IndexBuffer>();
		ib->AllocateElements(num_indices);
		ib->index_field()->SetFromUInt32s(&(*indices)[0][0], 1, 0, num_indices);
		prim->set_index_buffer(ib);
		Primitives::SetBoundingBoxAndZSortPoint(prim);
		return prim;
	}

	Primitive* Primitives::CreatePlane(
	    Pack* pack,
	    float width,
	    float depth,
	    int subdivisionsWidth,
	    int subdivisionsDepth,
	    Matrix4* matrix) {
		O3D_ASSERT(subdivisionsWidth >= 1);
		O3D_ASSERT(subdivisionsDepth >= 1);
		std::vector<Point3> positions;
		std::vector<Vector3> normals;
		std::vector<Vector2> tex_coords;
		std::vector<Triangle> indices;

		for(int z = 0; z <= subdivisionsDepth; z++) {
			for(int x = 0; x <= subdivisionsWidth; x++) {
				float u = static_cast<float>(x) / static_cast<float>(subdivisionsWidth);
				float v = static_cast<float>(z) / static_cast<float>(subdivisionsDepth);
				positions.push_back(Point3(
				                        width * u - width * 0.5f,
				                        0.0f,
				                        depth * v - depth * 0.5f));
				normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
				tex_coords.push_back(Vector2(u, 1 - v));
			}
		}

		int numVertsAcross = subdivisionsWidth + 1;

		for(int z = 0; z < subdivisionsDepth; z++) {
			for(int x = 0; x < subdivisionsWidth; x++) {
				// triangle 1 of quad
				indices.push_back(Triangle(
				                      (z + 0) * numVertsAcross + x,
				                      (z + 1) * numVertsAcross + x,
				                      (z + 0) * numVertsAcross + x + 1));
				// triangle 2 of quad
				indices.push_back(Triangle(
				                      (z + 1) * numVertsAcross + x,
				                      (z + 1) * numVertsAcross + x + 1,
				                      (z + 0) * numVertsAcross + x + 1));
			}
		}

		if(matrix) {
			ApplyMatrix(*matrix, &positions);
			ApplyMatrix(*matrix, &normals);
		}

		return CreatePrimitive(
		           pack, &positions, &normals, &tex_coords, &indices,
		           Primitive::TRIANGLELIST);
	}

	Primitive* Primitives::CreateSphere(
	    Pack* pack,
	    float radius,
	    int subdivisionsAxis,
	    int subdivisionsHeight,
	    Matrix4* matrix) {
		O3D_ASSERT(subdivisionsAxis >= 1);
		O3D_ASSERT(subdivisionsHeight >= 1);
		std::vector<Point3> positions;
		std::vector<Vector3> normals;
		std::vector<Vector2> tex_coords;
		std::vector<Triangle> indices;
		const float kPI = 3.14159265f;

		for(int y = 0; y <= subdivisionsHeight; ++y) {
			for(int x = 0; x <= subdivisionsAxis; ++x) {
				// Generate a vertex based on its spherical coordinates
				float u = static_cast<float>(x) / static_cast<float>(subdivisionsAxis);
				float v = static_cast<float>(y) / static_cast<float>(subdivisionsHeight);
				float theta = 2.0f * kPI * u;
				float phi = kPI * v;
				float sinTheta = sinf(theta);
				float cosTheta = cosf(theta);
				float sinPhi = sinf(phi);
				float cosPhi = cosf(phi);
				float ux = cosTheta * sinPhi;
				float uy = cosPhi;
				float uz = sinTheta * sinPhi;
				positions.push_back(Point3(radius * ux, radius * uy, radius * uz));
				normals.push_back(Vector3(ux, uy, uz));
				tex_coords.push_back(Vector2(1 - u, 1 - v));
			}
		}

		int numVertsAround = subdivisionsAxis + 1;

		for(int x = 0; x < subdivisionsAxis; ++x) {
			for(int y = 0; y < subdivisionsHeight; ++y) {
				// Make triangle 1 of quad.
				indices.push_back(Triangle(
				                      (y + 0) * numVertsAround + x,
				                      (y + 0) * numVertsAround + x + 1,
				                      (y + 1) * numVertsAround + x));
				// Make triangle 2 of quad.
				indices.push_back(Triangle(
				                      (y + 1) * numVertsAround + x,
				                      (y + 0) * numVertsAround + x + 1,
				                      (y + 1) * numVertsAround + x + 1));
			}
		}

		if(matrix) {
			ApplyMatrix(*matrix, &positions);
			ApplyMatrix(*matrix, &normals);
		}

		return CreatePrimitive(
		           pack, &positions, &normals, &tex_coords, &indices,
		           Primitive::TRIANGLELIST);
	}

}  // namespace o3d_utils

