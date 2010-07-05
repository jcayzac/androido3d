//	GameObject.cpp
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


#include "GameObject.h" // self

#include "GameComponent.h"
#include "ProfileSystem.h"
#include "System.h"
#include "SystemRegistry.h"

void GameObject::update(const float& time, const GameObjectSystem::GameObjectUpdatePhase currentPhase)
{
	const int count = mComponents.getCount();
	ProfileSystem* profiler = getSystem<ProfileSystem>();
	
	for (int x = 0; x < count; x++)
	{
		GameComponent* current = mComponents.get(x);
		if (current->runsInPhase(currentPhase))
		{
			if (profiler)
			{
				profiler->startTracking(current->getMetaObject()->getName());
			}	
			
			current->update(time, this);
			
			if (profiler)
			{
				profiler->stopTracking();
			}
		}
	}
}

void GameObject::remove(const GameComponent* component)
{
  const int count = mComponents.getCount();
	
	for (int x = 0; x < count; x++)
	{
		if (component == mComponents.get(x))
		{
		  // Note, this means that the fist *instance* of this component will be removed.
		  // but we never require instances to appear only one in this list.
		  mComponents.remove(x);
		  break;
		}
		
  }
}
