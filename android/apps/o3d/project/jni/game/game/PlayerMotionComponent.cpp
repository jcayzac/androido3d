//	PlayerMotionComponent.cpp
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


#include "PlayerMotionComponent.h"

#include "Blackboard.h"
#include "GameObject.h"
#include "Vector3.h"

#include <math.h>

void PlayerMotionComponent::update(const float timeDelta, GameObject* pParentObject)
{
  if (pParentObject->getRuntimeData()->exists("go"))
  {
    pParentObject->getRuntimeData()->remove("go"); 
    Vector3 orientation = pParentObject->getRuntimeData()->getVector("orientation");

    const float x = sinf(orientation[1]);
    const float y = cosf(orientation[1]);
    
    pParentObject->getRuntimeData()->insertVector(getMaxSpeed() * Vector3(x, 0.0f, y), "targetVelocity");

  }
  else
  {
    pParentObject->getRuntimeData()->insertVector(Vector3::ZERO, "targetVelocity");
  }
  
  pParentObject->getRuntimeData()->insertVector(getAcceleration(), "acceleration");
}