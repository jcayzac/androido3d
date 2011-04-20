// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include "core/cross/bounding_box.h"
#include "core/cross/math_utilities.h"
#include "core/cross/transform.h"
#include "camera.h"

namespace o3d_utils {
using namespace o3d;

float degToRad(float deg) {
  return deg * 3.14159f / 180.0f;
}

/**
 * Finds the bounding box of all primitives in the tree, in the local space of
 * the tree root. This will use existing bounding boxes on transforms and
 * elements, but not create new ones.
 * @param {!o3d.Transform} treeRoot Root of tree to search.
 * @return {!o3d.BoundingBox} The boundinding box of the tree.
 */
BoundingBox getBoundingBoxOfTree(Transform* treeRoot) {
  // If we already have a bounding box, use that one.
  BoundingBox box = treeRoot->bounding_box();
  if (box.valid()) {
    return box;
  }
  // Otherwise, create it as the union of all the children bounding boxes and
  // all the shape bounding boxes.
  const TransformRefArray& transforms = treeRoot->GetChildrenRefs();
  for (size_t i = 0; i < transforms.size(); ++i) {
    Transform* transform = transforms[i];
    BoundingBox childBox = getBoundingBoxOfTree(transform);
    if (childBox.valid()) {
      // transform by the child local matrix.
      BoundingBox localBox;
      childBox.Mul(transform->local_matrix(), &localBox);
      if (box.valid()) {
        BoundingBox temp;
        box.Add(localBox, &temp);
        box = temp;
      } else {
        box = localBox;
      }
    }
  }

  const ShapeRefArray& shapes = treeRoot->GetShapeRefs();
  for (size_t i = 0; i < shapes.size(); ++i) {
    const ElementRefArray& elements = shapes[i]->GetElementRefs();
    for (size_t j = 0; j < elements.size(); ++j) {
      BoundingBox elementBox = elements[j]->bounding_box();
      if (!elementBox.valid()) {
        elements[j]->GetBoundingBox(0, &elementBox);
      }
      if (box.valid()) {
        BoundingBox temp;
        box.Add(elementBox, &temp);
        box = temp;
      } else {
        box = elementBox;
      }
    }
  }
  return box;
};

const Matrix4& CameraInfo::computeProjection(
    float areaWidth,
    float areaHeight) {
  if (orthographic) {
    // TODO: figure out if there is a way to make this take the areaWidth
    //     and areaHeight into account. As it is, magX and magY from the
    //     collada file are relative to the aspect ratio of Maya's render
    //     settings which are not available here.
    // var magX = areaWidth * 0.5 / this.magX;
    // var magY = areaHeight * 0.5 / this.magY;
    projection = Vectormath::Aos::CreateOrthographicMatrix(
        -magX, magX, -magY, magY, zNear, zFar);
  } else {
    projection = Camera::perspective(
        fieldOfViewRadians,       // field of view.
        areaWidth / areaHeight,   // Aspect ratio.
        zNear,                    // Near plane.
        zFar);                    // Far plane.
  }
  return projection;
};

/**
 * Searches for all transforms with a "o3d.tags" ParamString
 * that contains specific tag keywords assuming comma separated
 * words.
 * @param {!o3d.Transform} treeRoot Root of tree to search for tags.
 * @param {string} searchTags Tags to look for. eg "camera", "ogre,dragon".
 * @return {!Array.<!o3d.Transform>} Array of transforms.
 */
std::vector<Transform*> Camera::getTransformsInTreeByTags(
    Transform* treeRoot,
    const std::string& searchTags) {
  // TODO: check for all tags.
  //var splitTags = searchTags.split(',');
  //var transforms = treeRoot.getTransformsInTree();
  //var found = [];
  //for (var n = 0; n < transforms.length; n++) {
  //  var tagParam = transforms[n].getParam('collada.tags');
  //  if (tagParam) {
  //     var tags = tagParam.value.split(',');
  //     for (var t = 0; t < tags.length; t++) {
  //       if (o3djs.util.arrayContains(splitTags, tags[t])) {
  //         found[found.length] = transforms[n];
  //         break;
  //       }
  //    }
  //  }
  //}
  //return found;
  TransformArray transforms = treeRoot->GetTransformsInTree();
  std::vector<Transform*> found;
  for (size_t ii = 0; ii < transforms.size(); ++ii) {
    ParamString* tagParam =
        transforms[ii]->GetParam<ParamString>("collada.tags");
    if (tagParam && tagParam->value().compare(searchTags) == 0) {
      found.push_back(transforms[ii]);
    }
  }
  return found;
};


/**
 * Searches for all nodes with a "o3d.tags" ParamString
 * that contains the word "camera" assuming comma separated
 * words.
 * @param {!o3d.Transform} treeRoot Root of tree to search for cameras.
 * @return {!Array.<!o3d.Transform>} Array of camera transforms.
 */
std::vector<Transform*> Camera::findCameras(Transform* treeRoot) {
  return getTransformsInTreeByTags(treeRoot, "camera");
};

/**
 * Creates a object with view and projection matrices using paramters found on
 * the camera 'o3d.projection_near_z', 'o3d.projection_far_z', and
 * 'o3d.perspective_fov_y' as well as the areaWidth and areaHeight passed
 * in.
 * @param {!o3d.Transform} camera Transform with camera information on it.
 * @param {number} areaWidth width of client area.
 * @param {number} areaHeight height of client area.
 * @return {!o3djs.camera.CameraInfo} A CameraInfo object.
 */
CameraInfo* Camera::getViewAndProjectionFromCamera(
    Transform* camera,
    float areaWidth,
    float areaHeight) {
  float fieldOfView = 30;
  float zNear = 1;
  float zFar = 5000;
  Matrix4 view;
  CameraInfo* cameraInfo = NULL;
  Point3 eye;
  Point3 target;
  Vector3 up;

  // Check if any LookAt elements were found for the camera and use their
  // values to compute a view matrix.
  ParamFloat3* eyeParam =
      camera->GetParam<ParamFloat3>("collada.eyePosition");
  ParamFloat3* targetParam =
      camera->GetParam<ParamFloat3>("collada.targetPosition");
  ParamFloat3* upParam =
      camera->GetParam<ParamFloat3>("collada.upVector");
  if (eyeParam && targetParam && upParam) {
    Float3 temp = eyeParam->value();
    eye = Point3(temp.getX(), temp.getY(), temp.getZ());
    temp = targetParam->value();
    target = Point3(temp.getX(), temp.getY(), temp.getZ());
    temp = upParam->value();
    up = Vector3(temp.getX(), temp.getY(), temp.getZ());
    view = Matrix4::lookAt(eye, target, up);
  } else {
    // Set it to the orientation of the camera.
    view = Vectormath::Aos::inverse(camera->world_matrix());
    eye = Point3(0,0,0) + camera->world_matrix().getTranslation();
    target = eye + camera->world_matrix().getUpper3x3().getCol2();
    up = camera->world_matrix().getUpper3x3().getCol1();
  }

  ParamString* projectionType =
      camera->GetParam<ParamString>("collada.projectionType");
  if (projectionType) {
    zNear =
        camera->GetParam<ParamFloat>("collada.projectionNearZ")->value();
    zFar  =
        camera->GetParam<ParamFloat>("collada.projectionFarZ")->value();

    if (projectionType->value().compare("orthographic") == 0) {
      float magX =
          camera->GetParam<ParamFloat>("collada.projectionMagX")->value();
      float magY =
          camera->GetParam<ParamFloat>("collada.projectionMagY")->value();

      cameraInfo = new CameraInfo(view, zNear, zFar);
      cameraInfo->setAsOrthographic(magX, magY);
    } else if (projectionType->value().compare("perspective") == 0) {
      fieldOfView =
          camera->GetParam<ParamFloat>("collada.perspectiveFovY")->value();
    }
  }

  if (!cameraInfo) {
    cameraInfo = new CameraInfo(view, zNear, zFar, eye, target, up);
    cameraInfo->setAsPerspective(degToRad(fieldOfView));
  }

  cameraInfo->computeProjection(areaWidth, areaHeight);
  return cameraInfo;
};

/**
 * Get CameraInfo that represents a view of the bounding box that encompasses
 * a tree of transforms.
 * @param {!o3d.Transform} treeRoot Root of sub tree to get extents from.
 * @param {number} clientWidth width of client area.
 * @param {number} clientHeight height of client area.
 * @return {!o3djs.camera.CameraInfo} A CameraInfo object.
 */
CameraInfo* Camera::getCameraFitToScene(Transform* treeRoot,
                                        float clientWidth,
                                        float clientHeight) {
  BoundingBox box = getBoundingBoxOfTree(treeRoot);
  Vector3 boxDimensions = box.max_extent() - box.min_extent();
  float diag = length(boxDimensions);
  Point3 minExtent = box.min_extent();
  Point3 maxExtent = box.max_extent();
  Point3 target(
      (minExtent.getX() + maxExtent.getX()) / 2.0f,
      (minExtent.getY() + maxExtent.getY()) / 2.0f,
      (minExtent.getZ() + maxExtent.getZ()) / 2.0f);
  Point3 eye(target + Vector3(boxDimensions.getX() * 0.3f,
                                        boxDimensions.getY() * 0.7f,
                                        diag * 1.5f));
  float nearPlane = diag / 1000.0f;
  float farPlane = diag * 10.0f;

  Vector3 up(0.0f, 1.0f, 0.0f);
  CameraInfo* cameraInfo = new CameraInfo(
      Matrix4::lookAt(eye, target, up),
      nearPlane,
      farPlane,
      eye,
      target,
      up);

  cameraInfo->setAsPerspective(degToRad(45.0f));
  cameraInfo->computeProjection(clientWidth, clientHeight);
  return cameraInfo;
};


/**
 * Calls findCameras and takes the first camera. Then calls
 * o3djs.camera.getViewAndProjectionFromCamera. If no camera is found it
 * sets up some defaults.
 * @param {!o3d.Transform} treeRoot Root of tree to search for cameras.
 * @param {number} areaWidth Width of client area.
 * @param {number} areaHeight Height of client area.
 * @return {!o3djs.camera.CameraInfo} A CameraInfo object.
 */
CameraInfo* Camera::getViewAndProjectionFromCameras(Transform* treeRoot,
                                                    float areaWidth,
                                                    float areaHeight) {
  std::vector<Transform*> cameras = findCameras(treeRoot);
  if (!cameras.empty()) {
    return getViewAndProjectionFromCamera(cameras[0],
                                          areaWidth,
                                          areaHeight);
  } else {
    // There was no camera in the file so make up a hopefully resonable default.
    return getCameraFitToScene(treeRoot,
                               areaWidth,
                               areaHeight);
  }
};

Matrix4 Camera::perspective(
    float fovy_in_radians, float aspect, float z_near, float z_far) {
  const float kEpsilon = 0.00001f;
  float radians = fovy_in_radians * 0.5f;
  float delta_z = z_far - z_near;
  float sine = sinf(radians);

  if ((fabsf(delta_z) < kEpsilon) ||
      (fabsf(sine) < kEpsilon) ||
      (fabsf(aspect) < kEpsilon)) {
  	return Matrix4::identity();
  }

  float cotangent = cosf(radians) / sine;

  return Matrix4(
    Vector4(cotangent / aspect, 0.0f, 0.0f, 0.0f),
    Vector4(0.0f, cotangent, 0.0f, 0.0f),
    Vector4(0.0f, 0.0f, -(z_far + z_near) / delta_z, -1.0f),
    Vector4(0.0f, 0.04f, -2.0f * z_near * z_far / delta_z, 0.0f));
}

}  // namespace o3d_utils

