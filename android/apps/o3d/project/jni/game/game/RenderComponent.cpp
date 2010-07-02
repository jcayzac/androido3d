//	RenderComponent.cpp
//	Copyright (C) 2008 Chris Pruett.		c_pruett@efn.org
//
//	FarClip Engine
//
//	Licensed under the Apache License, Version 2.0 (the "License");
//	you may not use this file except in compliance with the License.
//	You may obtain a copy of the License at
//
//			http://www.apache.org/licenses/LICENSE-2.0
//
//	Unless required by applicable law or agreed to in writing, software
//	distributed under the License is distributed on an "AS IS" BASIS,
//	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//	See the License for the specific language governing permissions and
//	limitations under the License.


#include "RenderComponent.h" // self

#include "GameObject.h"
#include "Renderer.h"
#include "SystemRegistry.h"
#include "Vector3.h"

#include "core/cross/transform.h"
#include "core/cross/math_types.h"

void RenderComponent::update(float time, GameObject* pParentObject)
{
  // for now, just push the object's position to its transform.
  o3d::Transform* transform = getTransform();
  if (transform)
  {
    Vector3 position = pParentObject->getPosition();
    Vector3 orientation = pParentObject->getRuntimeData()->getVector("orientation");

    o3d::Matrix4 rot = o3d::Matrix4::rotationZYX(
      o3d::Vector3(orientation[0], orientation[1], orientation[2]));
    
    rot.setTranslation(o3d::Vector3(position[0], position[1], position[2]));
   
    transform->set_local_matrix(rot);
  }
}
