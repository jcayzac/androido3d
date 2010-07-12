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

struct Triangle {
  Triangle(uint32 i0, uint32 i1, uint32 i2) {
    indices[0] = i0;
    indices[1] = i1;
    indices[2] = i2;
  }

  uint32& operator[] (int index) {
    DCHECK(index >= 0 && index < 3);
    return indices[index];
  }

  uint32 indices[3];
};

struct Vector2 {
  Vector2(float x, float y) {
    v[0] = x;
    v[1] = y;
  }

  float& operator[] (int index) {
    DCHECK(index >= 0 && index < 2);
    return v[index];
  }

  float v[2];
};

COMPILE_ASSERT(sizeof(Triangle) == sizeof(uint32) * 3, Triangle_size_is_wrong);
COMPILE_ASSERT(sizeof(Vector2) == sizeof(float) * 2, Vector2_size_is_wrong);

/**
 * Sets the bounding box and z sort point of an element.
 * @param {!o3d.Element} element Element to set bounding box and z sort point
 *     on.
 */
void Primitives::SetBoundingBoxAndZSortPoint(o3d::Element* element) {
  o3d::BoundingBox boundingBox;
  element->GetBoundingBox(0, &boundingBox);
  o3d::Point3 minExtent = boundingBox.min_extent();
  o3d::Point3 maxExtent = boundingBox.max_extent();
  element->set_bounding_box(boundingBox);
  element->set_cull(true);
  element->set_z_sort_point(o3d::Float3(
      (minExtent.getX() + maxExtent.getX()) / 2.0f,
      (minExtent.getY() + maxExtent.getY()) / 2.0f,
      (minExtent.getZ() + maxExtent.getZ()) / 2.0f));
};

void Primitives::ApplyMatrix(
    const o3d::Matrix4& mat,
    std::vector<o3d::Vector3>* vectors) {
  DCHECK(vectors);
  for (size_t ii = 0; ii < vectors->size(); ++ii) {
    o3d::Vector4 vec(mat * (*vectors)[ii]);
    (*vectors)[ii] = o3d::Vector3(vec[0], vec[1], vec[2]);
  }
}

void Primitives::ApplyMatrix(
    const o3d::Matrix4& mat,
    std::vector<o3d::Point3>* points) {
  DCHECK(points);
  for (size_t ii = 0; ii < points->size(); ++ii) {
    o3d::Vector4 vec(mat * (*points)[ii]);
    (*points)[ii] = o3d::Point3(vec[0], vec[1], vec[2]);
  }
}

void Primitives::SetFieldFromVector3s(
  o3d::FloatField* field,
  const std::vector<o3d::Vector3>& vectors) {
  scoped_array<float> floats(new float[vectors.size() * 3]);
  for (size_t ii = 0; ii < vectors.size(); ++ii) {
    size_t off = ii * 3;
    const o3d::Vector3& src = vectors[ii];
    floats[off + 0] = src[0];
    floats[off + 1] = src[1];
    floats[off + 2] = src[2];
  }
  field->SetFromFloats(floats.get(), 3, 0, vectors.size());
}

void Primitives::SetFieldFromPoint3s(
  o3d::FloatField* field,
  const std::vector<o3d::Point3>& points) {
  scoped_array<float> floats(new float[points.size() * 3]);
  for (size_t ii = 0; ii < points.size(); ++ii) {
    size_t off = ii * 3;
    const o3d::Point3& src = points[ii];
    floats[off + 0] = src[0];
    floats[off + 1] = src[1];
    floats[off + 2] = src[2];
  }
  field->SetFromFloats(floats.get(), 3, 0, points.size());
}

o3d::Primitive* CreatePrimitive(
  o3d::Pack* pack,
  std::vector<o3d::Point3>* positions,
  std::vector<o3d::Vector3>* normals,
  std::vector<Vector2>* tex_coords,
  std::vector<Triangle>* indices,
  o3d::Primitive::PrimitiveType primitive_type) {
  DCHECK(positions);
  DCHECK(indices);
  DCHECK(!normals || normals->size() == positions->size());
  DCHECK(!tex_coords || tex_coords->size() == positions->size());

  size_t num_vertices = positions->size();
  size_t num_indices = indices->size() * 3;
  size_t num_primitives = 0;
  switch (primitive_type) {
    case o3d::Primitive::POINTLIST:
      num_primitives = num_indices / 1;
      break;
    case o3d::Primitive::LINELIST:
      num_primitives = num_indices / 2;
      break;
    case o3d::Primitive::LINESTRIP:
      num_primitives = num_indices - 1;
      break;
    case o3d::Primitive::TRIANGLELIST:
      num_primitives = num_indices / 3;
      break;
    case o3d::Primitive::TRIANGLESTRIP:
    case o3d::Primitive::TRIANGLEFAN:
      num_primitives = num_indices - 2;
      break;
    default:
      NOTREACHED();
  }

  o3d::Primitive* prim = pack->Create<o3d::Primitive>();
  o3d::StreamBank* sb = pack->Create<o3d::StreamBank>();
  prim->set_stream_bank(sb);
  prim->set_number_primitives(num_primitives);
  prim->set_number_vertices(num_vertices);
  prim->set_primitive_type(primitive_type);
  prim->CreateDrawElement(pack, NULL);

  o3d::VertexBuffer* vb = pack->Create<o3d::VertexBuffer>();
  o3d::FloatField* position_field = vb->CreateTypedField<o3d::FloatField>(3);
  o3d::FloatField* normal_field = NULL;
  o3d::FloatField* texcoord_field = NULL;
  if (normals) {
    normal_field = vb->CreateTypedField<o3d::FloatField>(3);
  }
  if (tex_coords) {
    texcoord_field = vb->CreateTypedField<o3d::FloatField>(2);
  }
  vb->AllocateElements(num_vertices);
  Primitives::SetFieldFromPoint3s(position_field, *positions);
  sb->SetVertexStream(o3d::Stream::POSITION, 0, position_field, 0);
  if (normals) {
    normal_field->SetFromFloats(&(*normals)[0][0], 3, 0, num_vertices);
    Primitives::SetFieldFromVector3s(normal_field, *normals);
  }
  if (tex_coords) {
    texcoord_field->SetFromFloats(&(*tex_coords)[0][0], 2, 0, num_vertices);
    sb->SetVertexStream(o3d::Stream::TEXCOORD, 0, texcoord_field, 0);
  }

  o3d::IndexBuffer* ib = pack->Create<o3d::IndexBuffer>();
  ib->AllocateElements(num_indices);
  ib->index_field()->SetFromUInt32s(&(*indices)[0][0], 1, 0, num_indices);
  prim->set_index_buffer(ib);
  Primitives::SetBoundingBoxAndZSortPoint(prim);
  return prim;
}

o3d::Primitive* Primitives::CreatePlane(
      o3d::Pack* pack,
      float width,
      float depth,
      int subdivisionsWidth,
      int subdivisionsDepth,
      o3d::Matrix4* matrix) {
  DCHECK(subdivisionsWidth >= 1);
  DCHECK(subdivisionsDepth >= 1);

  std::vector<o3d::Point3> positions;
  std::vector<o3d::Vector3> normals;
  std::vector<Vector2> tex_coords;
  std::vector<Triangle> indices;

  for (int z = 0; z <= subdivisionsDepth; z++) {
    for (int x = 0; x <= subdivisionsWidth; x++) {
      float u = static_cast<float>(x) / static_cast<float>(subdivisionsWidth);
      float v = static_cast<float>(z) / static_cast<float>(subdivisionsDepth);
      positions.push_back(o3d::Point3(
          width * u - width * 0.5f,
          0.0f,
          depth * v - depth * 0.5f));
      normals.push_back(o3d::Vector3(0.0f, 1.0f, 0.0f));
      tex_coords.push_back(Vector2(u, 1 - v));
    }
  }

  int numVertsAcross = subdivisionsWidth + 1;

  for (int z = 0; z < subdivisionsDepth; z++) {
    for (int x = 0; x < subdivisionsWidth; x++) {
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

  if (matrix) {
    ApplyMatrix(*matrix, &positions);
    ApplyMatrix(*matrix, &normals);
  }

  return CreatePrimitive(
    pack, &positions, &normals, &tex_coords, &indices,
    o3d::Primitive::TRIANGLELIST);
}

o3d::Primitive* Primitives::CreateSphere(
      o3d::Pack* pack,
      float radius,
      int subdivisionsAxis,
      int subdivisionsHeight,
      o3d::Matrix4* matrix) {
  DCHECK(subdivisionsAxis >= 1);
  DCHECK(subdivisionsHeight >= 1);

  std::vector<o3d::Point3> positions;
  std::vector<o3d::Vector3> normals;
  std::vector<Vector2> tex_coords;
  std::vector<Triangle> indices;

  const float kPI = 3.14159265f;

  for (int y = 0; y <= subdivisionsHeight; ++y) {
    for (int x = 0; x <= subdivisionsAxis; ++x) {
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
      positions.push_back(o3d::Point3(radius * ux, radius * uy, radius * uz));
      normals.push_back(o3d::Vector3(ux, uy, uz));
      tex_coords.push_back(Vector2(1 - u, 1 - v));
    }
  }

  int numVertsAround = subdivisionsAxis + 1;

  for (int x = 0; x < subdivisionsAxis; ++x) {
    for (int y = 0; y < subdivisionsHeight; ++y) {
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

  if (matrix) {
    ApplyMatrix(*matrix, &positions);
    ApplyMatrix(*matrix, &normals);
  }

  return CreatePrimitive(
    pack, &positions, &normals, &tex_coords, &indices,
    o3d::Primitive::TRIANGLELIST);
}

}  // namespace o3d_utils

