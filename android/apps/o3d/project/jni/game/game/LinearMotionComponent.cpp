//	LinearMotionComponent.cpp
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


#include "LinearMotionComponent.h"

#include "GameObject.h"
#include "Lerper.h"
#include "SystemRegistry.h"
#include "TimeSystem.h"
#include "Vector3.h"

void LinearMotionComponent::update(const float timeDelta, GameObject* pParentObject)
{	
	if (pParentObject->getRuntimeData()->exists("targetPosition"))
	{
		Vector3 position = pParentObject->getPosition();
		const Vector3 targetPosition = pParentObject->getRuntimeData()->getVector("targetPosition");
		const float lerpDuration = pParentObject->getRuntimeData()->getFloat("timeToTargetPosition");
		const float lerpOffset = pParentObject->getRuntimeData()->getFloat("elapsedToTargetPosition");
		const bool useEase = static_cast<bool>(pParentObject->getRuntimeData()->getInt("easeToTarget"));
		const Vector3 startPosition = pParentObject->getRuntimeData()->getVector("startPosition");

		const float newLerpOffset = lerpOffset + timeDelta;
		
		if (useEase)
		{
			position[0] = Lerper::ease(startPosition[0], targetPosition[0], lerpDuration, newLerpOffset);
			position[1] = Lerper::ease(startPosition[1], targetPosition[1], lerpDuration, newLerpOffset);
			position[2] = Lerper::ease(startPosition[2], targetPosition[2], lerpDuration, newLerpOffset);
		}
		else 
		{
			position[0] = Lerper::lerp(startPosition[0], targetPosition[0], lerpDuration, lerpOffset);
			position[1] = Lerper::lerp(startPosition[1], targetPosition[1], lerpDuration, lerpOffset);
			position[2] = Lerper::lerp(startPosition[2], targetPosition[2], lerpDuration, lerpOffset);
		}
	
		if (newLerpOffset > lerpDuration)
		{
			position = targetPosition;
			pParentObject->getRuntimeData()->remove("targetPosition");
			pParentObject->getRuntimeData()->remove("timeToTargetPosition");
			pParentObject->getRuntimeData()->remove("elapsedToTargetPosition");
			pParentObject->getRuntimeData()->remove("easeToTarget");
			pParentObject->getRuntimeData()->remove("startPosition");
		}
		else
		{
			pParentObject->getRuntimeData()->insertFloat(newLerpOffset, "elapsedToTargetPosition");
		}
		
		// if the object is frozen, position does not move even though velocity continues to accumulate.
		if (pParentObject->getRuntimeData()->getInt("positionFrozen") == 0)
		{
			pParentObject->setPosition(position);
		}
	}
}	
