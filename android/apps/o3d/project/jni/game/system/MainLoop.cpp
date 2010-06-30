//	MainLoop.cpp
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


#include "MainLoop.h"

#include "ProfileSystem.h"
#include "SystemRegistry.h"

void MainLoop::updateAll()
{
	updateTime();
	
	updateGame();
}

void MainLoop::updateTime()
{
	if (mTimeSystem != NULL)
	{
		mTimeSystem->update();
		mCurrentTime = mTimeSystem->getGameTime();
	}
}

void MainLoop::updateGame()
{
	const float timeDelta = mTimeSystem->getGameTimeDelta();
	const int count = mSystems.getCount();
	ProfileSystem* profiler = getSystem<ProfileSystem>();
	
	// Copy the system list into an array so that the list itself may change as
	// this function executes (systems may be added or removed) without affecting
	// the app until the following frame.
	ObjectHandle<System>* systemArray = mSystems.convertToArray();
	for (int phase = PHASE_start; phase < PHASE_count; phase = phase++)
	{
	  UpdatePhase currentPhase = static_cast<UpdatePhase>(phase);
		for (int x = 0; x < count; x++)
		{
			if (systemArray[x]->runsInPhase(currentPhase))
			{
				if (profiler)
				{
					profiler->startTracking(systemArray[x]->getMetaObject()->getName());
				}
				
				systemArray[x]->update(timeDelta, currentPhase);
				
				if (profiler)
				{
					profiler->stopTracking();
				}
			}
		}
	}
	delete [] systemArray;
}


void MainLoop::addSystem(System* pSystem)
{
	mSystems.addUnique(pSystem);
}

void MainLoop::removeSystem(System* pSystem)
{
	const int index = mSystems.find(pSystem);
	if (index > -1)
	{
		mSystems.removeIndex(index);
	}
}

void MainLoop::removeAllSystems()
{
	mSystems.removeAll();
}
