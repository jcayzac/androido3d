// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef O3D_UTILS_CAMERA_H_
#define O3D_UTILS_CAMERA_H_

#include <string>
#include <vector>
#include "base/cross/types.h"

namespace o3d {

class Transform;

}  // namespace o3d_utils

namespace o3d_utils {

float degToRad(float deg);

struct CameraInfo {
  CameraInfo(const o3d::Matrix4& _view, float _zNear, float _zFar)
      : view(_view),
        projection(o3d::Matrix4::identity()),
        zNear(_zNear),
        zFar(_zFar),
        fieldOfViewRadians(degToRad(30.f)),
        eye(o3d::Point3(0.0f, 5.0f, 10.0f)),
        target(o3d::Point3(0.0f, 0.0f, 0.0f)),
        up(o3d::Vector3(0.0f, 1.0f, 0.0f)),
        magX(1.0f),
        magY(1.0f) {
  }

  CameraInfo(const o3d::Matrix4& _view, float _zNear, float _zFar,
             const o3d::Point3& _eye, const o3d::Point3& _target,
             const o3d::Vector3& _up)
      : view(_view),
        projection(o3d::Matrix4::identity()),
        zNear(_zNear),
        zFar(_zFar),
        fieldOfViewRadians(degToRad(30.f)),
        eye(_eye),
        target(_target),
        up(_up),
        magX(1.0f),
        magY(1.0f) {
  }

  /**
   * Sets the CameraInfo to an orthographic camera.
   * @param {number} magX horizontal magnification.
   * @param {number} magY vertical magnification.
   */
  void setAsOrthographic(float _magX, float _magY) {
    orthographic = true;
    magX = _magX;
    magY = _magY;
  }

  /**
   * Sets the CameraInfo to an orthographic camera.
   * @param {number} fieldOfView Field of view in radians.
   */
  void setAsPerspective(float _fieldOfView) {
    orthographic = false;
    fieldOfViewRadians = _fieldOfView;
  };

  /**
   * Computes a projection matrix for this CameraInfo using the areaWidth
   * and areaHeight passed in.
   *
   * @param {number} areaWidth width of client area.
   * @param {number} areaHeight heigh of client area.
   * @return {!o3djs.math.Matrix4} The computed projection matrix.
   */
  const o3d::Matrix4& computeProjection(
    float areaWidth,
    float areaHeight);

  o3d::Matrix4 view;
  o3d::Matrix4 projection;
  bool orthographic;
  float zNear;
  float zFar;
  float fieldOfViewRadians;
  o3d::Point3 eye;
  o3d::Point3 target;
  o3d::Vector3 up;
  float magX;
  float magY;
};

class Camera {
 public:
   static std::vector<o3d::Transform*> getTransformsInTreeByTags(
    o3d::Transform* treeRoot,
    const std::string& searchTags);

   static std::vector<o3d::Transform*> findCameras(o3d::Transform* treeRoot);

   static CameraInfo* getViewAndProjectionFromCameras(
       o3d::Transform* treeRoot,
       float areaWidth,
       float areaHeight);

   static CameraInfo* getViewAndProjectionFromCamera(
       o3d::Transform* camera,
       float areaWidth,
       float areaHeight);

   static CameraInfo* getCameraFitToScene(
       o3d::Transform* treeRoot,
       float clientWidth,
       float clientHeight);

   static o3d::Matrix4 perspective(
       float fovy_in_radians, float aspect, float z_near, float z_far);
};

}  // namespace o3d_utils

#endif  // O3D_UTILS_CAMERA_H_

