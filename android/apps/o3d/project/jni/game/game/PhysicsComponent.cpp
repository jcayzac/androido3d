//	PhysicsComponent.cpp
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


#include "PhysicsComponent.h" // self

#include "Array.h"
#include "CollisionSystem.h"
#include "CollisionPairSystem.h"
#include "GameObject.h"
#include "MathUtils.h"
#include "SystemRegistry.h"
#include "TimeSystem.h"
#include "Vector2.h"
#include "Vector3.h"


void PhysicsComponent::update(const float timeDelta, GameObject* pParentObject)
{
	// we look to user data so that other code can provide impulses
	Vector3 impulseVector = pParentObject->getRuntimeData()->getVector("impulse");
		
 	const Vector3 currentVelocity = pParentObject->getRuntimeData()->getVector("velocity");
	
	if (pParentObject->getRuntimeData()->exists("backgroundSurfaceNormal"))
	{
		const Vector3 surfaceNormal = pParentObject->getRuntimeData()->getVector("backgroundSurfaceNormal");
		if (surfaceNormal.length2() > 0.0f)
		{
			impulseVector = resolveCollision(currentVelocity, impulseVector, surfaceNormal);
		}
	}
	
	// bounce off entities we hit this frame
	CollisionPairSystem* pPairs = getSystem<CollisionPairSystem>();
	GameObjectSystem* pGameObjects = getSystem<GameObjectSystem>();

	if (pPairs && pGameObjects)
	{
		Array<CollisionPairSystem::CollisionRecord*> objects;
		pPairs->findPairs(pParentObject, objects);
				
		const int count = objects.getCount();
		for (int x = 0; x < count; x++)
		{
			const CollisionPairSystem::CollisionRecord* record = objects.get(x);
			if (record->getIntersecting())
			{
				const GameObject* object = pGameObjects->get(record->getOtherObject(pParentObject->getID()));
				const Vector3 normal = record->getObjectOpposingNormal(pParentObject->getID());
				if (object->getRuntimeData()->exists("mass"))
				{
					const float otherMass = object->getRuntimeData()->getFloat("mass");
					const Vector3 otherVelocity = object->getRuntimeData()->getVector("velocity");
					const Vector3 otherImpulse = object->getRuntimeData()->getVector("impulse");
					const float otherBounciness = object->getRuntimeData()->getFloat("bounciness");
					impulseVector = resolveCollision(currentVelocity, impulseVector, normal, otherMass, otherVelocity, otherImpulse, otherBounciness);
				}
				else
				{
					// assume infinite mass
					impulseVector = resolveCollision(currentVelocity, impulseVector, normal);
				}
			}
		}
	}
	// if our speed is below inertia, we need to overcome inertia before
	// we can move.
	
	bool physicsCausesMovement = true;
	
	float inertiaSquared = getInertia() * getInertia();
	
	if (currentVelocity.length2() < inertiaSquared)
	{
		if ((currentVelocity + impulseVector).length2() < inertiaSquared)
		{
			physicsCausesMovement = false;
		}
	}
	
	Vector3 newVelocity = currentVelocity;
	newVelocity += impulseVector;
	
	const TimeSystem* pTimeSystem = getSystem<TimeSystem>();
	const float time = pTimeSystem->getGameTime();

	bool touchingFloor = false;
	if (pParentObject->getRuntimeData()->exists("touchingFloorLastTime"))
	{
		touchingFloor = Close(pParentObject->getRuntimeData()->getFloat("touchingFloorLastTime"), time);
	}
	
	if (touchingFloor && currentVelocity[1] <= 0.0f && Abs(newVelocity[0]) > 0.0f && pParentObject->getRuntimeData()->exists("gravity"))
	{
		const Vector3 gravityVector = pParentObject->getRuntimeData()->getVector("gravity");
	
		// if we were moving last frame, we'll use dynamic friction.	Else static.
		float frictionCoeffecient = Abs(currentVelocity[0]) > 0.0f ? getDynamicFrictionCoeffecient() : getStaticFrictionCoeffecient();
		frictionCoeffecient *= timeDelta;
		
		// Friction = cofN, where cof = friction coefficient and N = force perpendicular to the ground.
		const float maxFriction = Abs(gravityVector[1]) * getMass() * frictionCoeffecient;
		
		if (maxFriction > Abs(newVelocity[0]))
		{
			newVelocity[0] = 0.0f;
		}
		else
		{
			newVelocity[0] -= maxFriction * Sign(newVelocity[0]);
		}
	}
	
	if (Abs(newVelocity[0]) < 0.1f)
	{
		newVelocity[0] = 0.0f;
	}
	
	if (Abs(newVelocity[1]) < 0.1f)
	{
		newVelocity[1] = 0.0f;
	}
	
	// physics-based movements means constant acceleration, always.	set the target
	// to the velocity.
	pParentObject->getRuntimeData()->insertVector(newVelocity, "velocity");
	pParentObject->getRuntimeData()->insertVector(newVelocity, "targetVelocity");
	pParentObject->getRuntimeData()->insertVector(Vector3::ZERO, "acceleration");
	pParentObject->getRuntimeData()->insertVector(Vector3::ZERO, "impulse");
	pParentObject->getRuntimeData()->insertFloat(getMass(), "mass");
	pParentObject->getRuntimeData()->insertFloat(getBounciness(), "bounciness");
}



Vector3 PhysicsComponent::resolveCollision(Vector3 velocity, Vector3 impulse, Vector3 opposingNormal)
{
	Vector3 outputImpulse = impulse;
	
	Vector3 collisionNormal = opposingNormal;
	collisionNormal.normalize();
	
	Vector3 relativeVelocity = velocity + impulse;
	
	float dotRelativeAndNormal = relativeVelocity.dot(collisionNormal);
	
	// make sure the motion of the entity requires resolution
	if (dotRelativeAndNormal < 0.0f)
	{
		const float coefficientOfRestitution = getBounciness();	// 0 = perfectly inelastic, 1 = perfectly elastic
		
		// calculate an impulse to apply to the entity
		float j = ( -(1 + coefficientOfRestitution) * dotRelativeAndNormal);
		
		j /= ((collisionNormal.dot(collisionNormal)) * (1 / getMass()));
		
		Vector3 entityAdjust = (collisionNormal * j);
		entityAdjust[0] /= getMass();
		entityAdjust[1] /= getMass();
		entityAdjust[2] /= getMass();
		
		outputImpulse = entityAdjust;
	}
	
	return outputImpulse;
}

Vector3 PhysicsComponent::resolveCollision(Vector3 velocity, Vector3 impulse, Vector3 opposingNormal, const float otherMass, 
	const Vector3& otherVelocity, const Vector3& otherImpulse, const float otherBounciness)
{
	Vector3 collisionNormal = opposingNormal;
	collisionNormal.normalize();
	
	Vector3 entity1Velocity = velocity + impulse;
	Vector3 entity2Velocity = otherVelocity + otherImpulse;
	
	Vector3 relativeVelocity = entity1Velocity - entity2Velocity;
	
	float dotRelativeAndNormal = relativeVelocity.dot(collisionNormal);
	
	// make sure the entities' motion requires resolution
	if (dotRelativeAndNormal < 0.0f)
	{
		const float bounciness = Min(getBounciness() + otherBounciness, 1.0f);
		const float coefficientOfRestitution = bounciness;	 // 0 = perfectly inelastic, 1 = perfectly elastic
		
		// calculate an impulse to apply to both entities
		float j = ( -(1 + coefficientOfRestitution) * dotRelativeAndNormal);
		
		j /= ((collisionNormal.dot(collisionNormal)) * (1 / getMass() + 1 / otherMass));
		
		
		Vector3 entity1Adjust = (collisionNormal * j);
		entity1Adjust[0] /= getMass();
		entity1Adjust[1] /= getMass();
		entity1Adjust[2] /= getMass();
		
		const Vector3 newEntity1Impulse = impulse + entity1Adjust;

		
	/*
		Vector3 entity2Adjust = (collisionNormal * j);
		entity2Adjust[0] /= otherMass;
		entity2Adjust[1] /= otherMass;
		entity2Adjust[2] /= otherMass;
		
		const Vector3 newEntity2Impulse = otherImpulse + entity2Adjust;

	*/
		
		return newEntity1Impulse;
	}
	
}
