//	PlayerAnimationComponent.cpp
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


#include "PlayerAnimationComponent.h"

#include "Blackboard.h"
#include "GameObject.h"
#include "Vector3.h"

#include "third_party/loggingshim/base/logging.h"

void PlayerAnimationComponent::update(const float timeDelta, GameObject* pParentObject)
{
  Vector3 velocity = pParentObject->getRuntimeData()->getVector("velocity");
  Vector3 targetVelocity = pParentObject->getRuntimeData()->getVector("targetVelocity");

  velocity[1] = 0.0f; // we only want XZ velocity.
  targetVelocity[1] = 0.0f; 
  
  const float speed = velocity.length2();
  const float target = targetVelocity.length2();
  const float runSpeed = 22.0f * 22.0f;
  const float walkSpeed = 1.0f * 1.0f;
  
  if ((speed >= runSpeed || target >= runSpeed) && getRunAnimation() >= 0)
  {
    pParentObject->getRuntimeData()->insertInt(getRunAnimation(), "currentAnimation");
  } 
  else if (target >= walkSpeed && getWalkAnimation() >= 0)
  {
    pParentObject->getRuntimeData()->insertInt(getWalkAnimation(), "currentAnimation");
  }
  else if (getIdleAnimation() >= 0)
  {
    pParentObject->getRuntimeData()->insertInt(getIdleAnimation(), "currentAnimation");
  }
}