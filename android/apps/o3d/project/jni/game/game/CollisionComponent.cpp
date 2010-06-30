//	CollisionComponent.cpp
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


#include "CollisionComponent.h" // self

#include "Array.h"
#include "BoxCollisionSystem.h"
#include "CollisionPairSystem.h"
#include "CollisionSystem.h"
#include "GameObject.h"
#include "GameObjectSystem.h"
#include "MathUtils.h"
#include "SystemRegistry.h"
#include "TimeSystem.h"
#include "Vector2.h"
#include "Vector3.h"


void CollisionComponent::update(const float timeDelta, GameObject* pParentObject)
{
	Vector3 currentPosition = pParentObject->getPosition();
	
	Box bounds = getBounds();
	bounds.moveTo(currentPosition);
	
	// If this object teleported last frame, make sure that we don't sweep the
	// old position to the new position for collision.
	const bool teleported = pParentObject->getRuntimeData()->exists("teleported");
	if (teleported)
	{
		pParentObject->getRuntimeData()->remove("teleported");
		
		// Instead of using the real bounds from the previous frame, we're going to
		// pretend that we've teleported and then moved slightly.	So we'll generate
		// a fake bound that we can pretend to have been at last frame so that we still
		// test collision in the direction of movement.
		const Vector3 currentVelocity = pParentObject->getRuntimeData()->getVector("velocity");
		Vector3 normalizedVelocity = currentVelocity;
		normalizedVelocity.normalize();
		Vector3 fakeLastFramePosition = currentPosition - normalizedVelocity;
		bounds.moveTo(fakeLastFramePosition);
		bounds.moveTo(currentPosition);
	}

	CollisionSystem* pCollisionSystem = getSystem<CollisionSystem>();
	
	Vector3 correctedPosition = pParentObject->getPosition();
	Vector3 surfaceNormal = Vector3::ZERO;

	if (pCollisionSystem)
	{
		Vector3 snapOffset = Vector3::ZERO;
		Vector3 testPoint = correctedPosition;
		testPoint += Vector3(bounds.getSize()[0] / 2.0f, 0.0f, bounds.getSize()[2] / 2.0f);
		pCollisionSystem->testPoint(testPoint, &surfaceNormal);
		pParentObject->getRuntimeData()->insertVector(surfaceNormal, "backgroundSurfaceNormal");
	}
	
	Vector3 solidEntitySurfaces = Vector3::ZERO;

	// Entity-entity test
	if (getOpposeEntities())
	{
		BoxCollisionSystem* pBoxCollision = getSystem<BoxCollisionSystem>();
		GameObjectSystem* pGameObjects = getSystem<GameObjectSystem>();
		CollisionPairSystem* pPairs = getSystem<CollisionPairSystem>();
		
		if (pBoxCollision && pGameObjects && pPairs)
		{
			bool snappedAgainstEntity = false;
			Box preEntitySnap = bounds;
			
			Array<CollisionPairSystem::CollisionRecord*> objects;
			pPairs->findPairs(pParentObject, objects);
					
			const int count = objects.getCount();
			for (int x = 0; x < count; x++)
			{
				CollisionPairSystem::CollisionRecord* record = objects.get(x);
				ASSERT(record, "NULL collision record!");
				const GameObject* object = pGameObjects->get(record->getOtherObject(pParentObject->getID()));
				ASSERT(object != pParentObject, "Object colliding with itself!");
				if (object != pParentObject)
				{
					if (object->getRuntimeData()->exists("boundsSize"))
					{
						bool otherSolid = object->getRuntimeData()->getInt("solid");
						Vector3 boundsSize = object->getRuntimeData()->getVector("boundsSize");
						// TODO: this method discards the other user's movement history.
						// The box should live in the blackboard!
						const Box otherBounds(object->getPosition(), boundsSize);
						Box snapBox = bounds;
						Vector3 entitySurfaceNormal;
						if (pBoxCollision->snapOut(snapBox, otherBounds, entitySurfaceNormal))
						{
							if (otherSolid && getCanBeSnapped())
							{
								snappedAgainstEntity = true;
								bounds = snapBox;
								solidEntitySurfaces += entitySurfaceNormal;
							}
							record->setObjectOpposingNormal(pParentObject->getID(), entitySurfaceNormal);
							record->setIntersecting(true);
						}
					}
				}
			}
			
			// if we snapped against an entity, we need to double-check the background collision.
			if (snappedAgainstEntity && pCollisionSystem)
			{
				// generate a bounds that describes the movement before and after entity collision.
				Box testBounds = preEntitySnap;
				testBounds.moveTo(bounds.getPosition());
				Vector3 snapOffset = Vector3::ZERO;
				Vector3 testPoint = bounds.getPosition();
		    testPoint += Vector3(bounds.getSize()[0] / 2.0f, 0.0f, bounds.getSize()[2] / 2.0f);
		    pCollisionSystem->testPoint(testPoint, &surfaceNormal);
		    // The original code included snapping here, beware.
		    
				// We'll overwrite the surface normal and use the most recent one in this case.
				pParentObject->getRuntimeData()->insertVector(surfaceNormal, "backgroundSurfaceNormal");
				bounds.setPosition(testBounds.getPosition());
			}
		}
	}
	
	correctedPosition = bounds.getPosition();
	currentPosition = correctedPosition;
	pParentObject->setPosition(correctedPosition);
	setBounds(bounds);
	
	// If we're opting-in to entity collision, provide our bounding volume to other entities.
	if (getOpposeEntities())
	{
		pParentObject->getRuntimeData()->insertVector(bounds.getSize(), "boundsSize");
		pParentObject->getRuntimeData()->insertInt(getSolid(), "solid");
	}

	// find those surfaces we are touching
	const Vector3 topMiddle = (bounds.getTopLeft() + bounds.getTopRight()) / 2.0f;
	const Vector3 bottomMiddle = (bounds.getBottomLeft() + bounds.getBottomRight()) / 2.0f;
	const Vector3 leftMiddle = (bounds.getTopLeft() + bounds.getBottomLeft()) / 2.0f;
	const Vector3 rightMiddle = (bounds.getTopRight() + bounds.getBottomRight()) / 2.0f;
	
	const bool touchingCeiling = solidEntitySurfaces[1] < 0.0f || findSurface(bounds, topMiddle, Vector3(0.0f, 1.0f, 0.0f), getSurfaceProximityTolerance());
	const bool touchingFloor = solidEntitySurfaces[1] > 0.0f || findSurface(bounds, bottomMiddle, Vector3(0.0f, -1.0f, 0.0f), getSurfaceProximityTolerance());
	const bool touchingLeftWall = solidEntitySurfaces[0] > 0.0f || findSurface(bounds, leftMiddle, Vector3(-1.0f, 0.0f, 0.0f), getSurfaceProximityTolerance());
	const bool touchingRightWall = solidEntitySurfaces[0] < 0.0f || findSurface(bounds, rightMiddle, Vector3(1.0f, 0.0f, 0.0f), getSurfaceProximityTolerance());
	
	const TimeSystem* pTimeSystem = getSystem<TimeSystem>();
	if (pTimeSystem)
	{
		const float time = pTimeSystem->getGameTime();
		if (touchingFloor)
		{
			pParentObject->getRuntimeData()->insertFloat(time, "touchingFloorLastTime");
		}
		
		if (touchingCeiling)
		{
			pParentObject->getRuntimeData()->insertFloat(time, "touchingCeilingLastTime");
		}
		
		if (touchingLeftWall)
		{
			pParentObject->getRuntimeData()->insertFloat(time, "touchingLeftWallLastTime");
		}
		
		if (touchingRightWall)
		{
			pParentObject->getRuntimeData()->insertFloat(time, "touchingRightWallLastTime");
		}
	}
}


// Shoots a short ray and reports if any surfaces block movement in that direction.
bool CollisionComponent::findSurface(const Box& box, const Vector3& point, const Vector3& direction, const float tolerance)
{
	bool touching = false;
	/*CollisionSystem* pCollisionSystem = getSystem<CollisionSystem>();
	
	if (pCollisionSystem)
	{
		Vector3 hitNormal;
		Vector2 hitPosition;
		
		if (pCollisionSystem->testStraightLine(point, point + (direction * tolerance), &hitPosition, &hitNormal))
		{
			if (hitNormal.length2() > 0.0f && hitNormal.dot(direction) < 0.0f)
			{
				touching = true;
			}
		}
	}*/
	return touching;
}