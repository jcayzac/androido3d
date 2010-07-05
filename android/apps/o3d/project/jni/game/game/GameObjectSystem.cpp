//	GameObjectSystem.cpp
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


#include "GameObjectSystem.h"

#include "ComponentList.h"
#include "GameObject.h"
#include "KeyValueMap.h"


GameObjectSystem::GameObjectSystem()
{
}

void GameObjectSystem::update(const float timeDelta, const UpdatePhase phase)
{
	ASSERT(phase == System::PHASE_logic || phase == System::PHASE_movement || phase == System::PHASE_resolution ||
		phase == System::PHASE_postMovement || phase == System::PHASE_render, "GameObjectSystem running in an invalid phase!");
	setCurrentPhase(static_cast<GameObjectUpdatePhase>(phase));
	const int count = mGameObjects.getCount();
	for (int x = 0; x < count; x++)
	{
		mGameObjects.get(x)->update(timeDelta, static_cast<GameObjectUpdatePhase>(phase));
	}
}

int GameObjectSystem::add(GameObject* pObject)
{
	pObject->setID(mGameObjects.getCount());
	mGameObjects.append(pObject);
	return pObject->getID();
}

void GameObjectSystem::remove(GameObject* pObject)
{
	// TODO: instead of leaving empty slots in this list, we could probably maintain a list of
	// free slots and reuse them upon allocation.  But for now this will suffice.
	const int id = pObject->getID();
	mGameObjects.set(pObject->getID(), NULL);
	
}

void GameObjectSystem::removeAll()
{
	mGameObjects.removeAll();
}

GameObject* GameObjectSystem::get(int id)
{
	// for now, id == index;
	GameObject* result = NULL;
	if (id < mGameObjects.getCount())
	{
		result = mGameObjects.get(id);
	}
	
	return result;
}

void GameObjectSystem::getAll(Array< ObjectHandle<GameObject> >* objects)
{
	ASSERT(objects, "NULL object list passed to getAll()!");
	if (objects)
	{
		objects->copyFrom(mGameObjects);
	}
}

void GameObjectSystem::findObjectsInSphere(const Vector3& position, const float radius, Array<GameObject*>* objects) 
{
	ASSERT(objects, "NULL object list passed to findObjectsInSphere!");
	if (objects)
	{
		const int count = mGameObjects.getCount();
		for (int x = 0; x < count; x++)
		{
			GameObject* object = get(x);
			if (object)
			{
				Vector3 delta = object->getPosition() - position;
				if (delta.length2() <= radius * radius)
				{
					objects->append(object);
				}
			}
		}
	}
}