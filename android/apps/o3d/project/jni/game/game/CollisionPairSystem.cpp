//	CollisionPairSystem.cpp
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

#include "CollisionPairSystem.h"

#include "GameObject.h"
#include "GameObjectSystem.h"
#include "MathUtils.h"
#include "SystemRegistry.h"

CollisionPairSystem::CollisionPairSystem()
:	mPairs(true, MAX_collisionPairs)
{
}

void CollisionPairSystem::update(const float timeDelta, const UpdatePhase phase)
{
	// Iterate through all of the entities and find collision pairs to add to the map.
	// TODO: make it fast.  We could sort by X axis and then only compare against entities that had x overlap, etc.
	mPairs.removeAll();
	GameObjectSystem* objectSystem = getSystem<GameObjectSystem>();
	if (objectSystem)
	{
		Array< ObjectHandle<GameObject> > objects;
		objectSystem->getAll(&objects);
		const int objectCount = objects.getCount();
		// Slow.
		for (int x = 0; x < objectCount; x++)
		{
			GameObject* object = objects.get(x);
			if (object->getRuntimeData()->exists("boundsSize"))
			{
				Vector3 boundsSize = object->getRuntimeData()->getVector("boundsSize");
				const float sphereRadius = Max(boundsSize[0], boundsSize[1]);
				Array<GameObject*> hitObjects;
				// Slow.
				objectSystem->findObjectsInSphere(object->getPosition(), sphereRadius, &hitObjects);
				
				const int hitObjectCount = hitObjects.getCount();
				LinkedList< ObjectHandle<CollisionRecord> >* hitRecords;
				const int ownerID = object->getID();
				// Slow.
				const int ownerPairIndex = mPairs.findIndex(ownerID);
				if (ownerPairIndex != -1)
				{
					hitRecords = mPairs.get(ownerPairIndex);
				}
				else
				{
					hitRecords = new LinkedList< ObjectHandle<CollisionRecord> >;
				}
				
				for (int y = 0; y < hitObjectCount; y++)
				{
					// Slow.
					const int otherID = hitObjects.get(y)->getID();
					if (otherID != ownerID)
					{
						int index = findCollisionRecord(otherID, hitRecords);
						if (index == -1)
						{
							LinkedList< ObjectHandle<CollisionRecord> >* otherHitRecords;
							// Slow.
							const int otherPairIndex = mPairs.findIndex(otherID);
							if (otherPairIndex != -1)
							{
								otherHitRecords = mPairs.get(otherPairIndex);
							}
							else
							{
								otherHitRecords = new LinkedList< ObjectHandle<CollisionRecord> >;
							}
							
							CollisionRecord* newRecord = CollisionRecord::factory();
							newRecord->setObject1(ownerID);
							newRecord->setObject2(otherID);
							hitRecords->add(newRecord);
							otherHitRecords->add(newRecord);
							mPairs.add(ownerID, hitRecords);
							mPairs.add(otherID, otherHitRecords);
						}
					}
				}
			}
		}
	}
}

int CollisionPairSystem::findCollisionRecord(const int index, const LinkedList< ObjectHandle<CollisionRecord> >* list) 
{
	ASSERT(list != NULL, "NULL list passed to findCollisionRecord!");
	int found = -1;
	if (list)
	{
		const int count = list->getCount();
		for (int x = 0; x < count; x++)
		{
			CollisionRecord* record = list->get(x);
			if (record->getObject1() == index || record->getObject2() == index)
			{
				found = x;
				break;
			}
		}
	}
	
	return found;
}

bool CollisionPairSystem::findPairs(const GameObject* pObject, Array<CollisionRecord*>& intersectingObjectIDs)
{
	// Get the pairs generated for the object in question and return them.
	ASSERT(pObject, "Can't work with a NULL object!");
	intersectingObjectIDs.removeAll();
	bool foundPair = false;
	if (pObject)
	{
		const int index = mPairs.findIndex(pObject->getID());
		if (index != -1)
		{
			LinkedList< ObjectHandle<CollisionRecord> >* hitRecords = mPairs.get(index);
			const int count = hitRecords->getCount();
			if (count > 0)
			{
				for (int x = 0; x < count; x++)
				{	
					CollisionRecord* record = hitRecords->get(x);
					intersectingObjectIDs.append(record);
				}
				foundPair = true;
			}
		}
	}
	return foundPair;
}