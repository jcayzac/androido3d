//	MovementComponent.cpp
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


#include "MovementComponent.h"

#include "GameObject.h"
#include "Interpolator.h"
#include "MathUtils.h"
#include "SystemRegistry.h"
#include "TimeSystem.h"
#include "Vector3.h"

// The movement system has the following rules:
//	- We move a character by defining a current velocity, a target velocity, and an acceleration
//	- Acceleration always points in the direction of the target velocity
//	- If the current velocity is equal to the target velocity, we don't apply acceleration
//		This causes velocity to hold when the target velocity has been reached.
// The Interpolator utility does the actual work for us.
void MovementComponent::update(const float timeDelta, GameObject* pParentObject)
{	
	Vector3 position = pParentObject->getPosition();
	Vector3 velocity = pParentObject->getRuntimeData()->getVector("velocity");
	Vector3 targetVelocity = pParentObject->getRuntimeData()->getVector("targetVelocity");
	Vector3 acceleration = pParentObject->getRuntimeData()->getVector("acceleration");
  
	position[0] += Interpolator::interpolate(velocity[0], targetVelocity[0], acceleration[0], timeDelta);
	position[1] += Interpolator::interpolate(velocity[1], targetVelocity[1], acceleration[1], timeDelta);
	position[2] += Interpolator::interpolate(velocity[2], targetVelocity[2], acceleration[2], timeDelta);
	
	// if the object is frozen, position does not move even though velocity continues to accumulate.
	if (pParentObject->getRuntimeData()->getInt("positionFrozen") == 0)
	{
		pParentObject->setPosition(position);
	}
	
	pParentObject->getRuntimeData()->insertVector(velocity, "velocity");
	pParentObject->getRuntimeData()->insertVector(targetVelocity, "targetVelocity");
	pParentObject->getRuntimeData()->insertVector(acceleration, "acceleration");
}	
