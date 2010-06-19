//	BoxCollisionSystem.cpp
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

#include "BoxCollisionSystem.h"

#include "Box.h"
#include "MathUtils.h"
#include "Vector3.h"

bool BoxCollisionSystem::snapOut(Box& box1, const Box& box2, Vector3& surfaceNormal) const
{
	bool intersecting = testIntersection(box1, box2);
	if (intersecting)
	{
		// TODO: handle changes in size here as well.
		
		Vector3 snapNormal = Vector3::ZERO;
		// we always snap box1 out of box2.
		Vector3 movementDirection = box1.getPosition() - box1.getLastPosition();
			
		if (movementDirection.length2() == 0.0f) 
		{
			// Box1 didn't move, so Box2 must have.  Use Box2's movement as the inverse snap direction.
			movementDirection = (box2.getPosition() - box2.getLastPosition()) * -1.0f;
		}
		
		if (movementDirection.length2() == 0.0f)
		{
			// Bad case.  Neither box1 nor box2 are moving, yet somehow they are intersecting.  Just pick a random direction to snap against.
			movementDirection = Vector3::ONE;
		}
		
		Box horizontalTestBox = box1;
		Box verticalTestBox = box1;
		Vector3 horizontalSnapNormal = Vector3::ZERO;
		Vector3 verticalSnapNormal = Vector3::ZERO;
		snapHorizontal(horizontalTestBox, box2, movementDirection, horizontalSnapNormal);
		snapVertical(verticalTestBox, box2, movementDirection, verticalSnapNormal);

		const float horizontalDepth = Abs(horizontalTestBox.getPosition()[0] - box1.getPosition()[0]);
		const float verticalDepth = Abs(verticalTestBox.getPosition()[1] - box1.getPosition()[1]);
		const bool opposesHorizontal = Sign(box2.getPosition()[0] - box1.getPosition()[0]) == Sign(movementDirection[0]);
		const bool opposesVertical = Sign(box2.getPosition()[1] - box1.getPosition()[1]) == Sign(movementDirection[1]);
		
		if (horizontalDepth < verticalDepth && opposesHorizontal)
		{
			box1 = horizontalTestBox;
			snapNormal = horizontalSnapNormal;
		}
		else if (opposesVertical)
		{
			box1 = verticalTestBox;
			snapNormal = verticalSnapNormal;
		}
		else
		{
			intersecting = false;
		}
		
		
		surfaceNormal = snapNormal;
	}
	return intersecting;
}

void BoxCollisionSystem::snapHorizontal(Box& box1, const Box& box2, const Vector3& movementDirection, Vector3& snapNormal) const
{
	if (movementDirection[0] > 0.0f) // Moving right?
	{
		// horizontal snap to the left.
		Vector3 position = box2.getPosition();
		position[0] = box2.getLeft() - 1;
		position[0] -= box1.getSize()[0] / 2.0f;
		position[1] = box1.getPosition()[1];
		position[2] = box1.getPosition()[2];
		box1.setPosition(position);
		snapNormal[0] = -1;
	} 
	else if (movementDirection[0] < 0.0f) // Moving left?
	{
		// horizontal snap to the right.
		Vector3 position = box2.getPosition();
		position[0] = box2.getRight() + 1;
		position[0] += box1.getSize()[0] / 2.0f;
		position[1] = box1.getPosition()[1];
		position[2] = box1.getPosition()[2];
		box1.setPosition(position);
		snapNormal[0] = 1;
	}
}

void BoxCollisionSystem::snapVertical(Box& box1, const Box& box2, const Vector3& movementDirection, Vector3& snapNormal) const
{
	if (movementDirection[1] > 0.0f) // Moving up?
	{
		// vertical snap down.
		Vector3 position = box2.getPosition();
		position[1] = box2.getBottom() - 1;
		position[1] -= box1.getSize()[1] / 2.0f;
		position[0] = box1.getPosition()[0];
		position[2] = box1.getPosition()[2];
		box1.setPosition(position);
		snapNormal[1] = -1;
	} 
	else if (movementDirection[1] < 0.0f)  // Moving down?
	{
		// vertical snap up.
		Vector3 position = box2.getPosition();
		position[1] = box2.getTop() + 1;
		position[1] += box1.getSize()[1] / 2.0f;
		position[0] = box1.getPosition()[0];
		position[2] = box1.getPosition()[2];
		box1.setPosition(position);
		snapNormal[1] = 1;
	}
}

bool BoxCollisionSystem::testIntersection(const Box& box1, const Box& box2) const
{
	return boxIntersect(box1, box2) || boxIntersect(box2, box1);
}

bool BoxCollisionSystem::boxIntersect(const Box& box1, const Box& box2) const
{
	const bool horizontalIntersection = box1.getLeft() < box2.getRight() && box2.getLeft() < box1.getRight();
	const bool verticalIntersection = box1.getTop() > box2.getBottom() && box2.getTop() > box1.getBottom();
	const bool intersecting = horizontalIntersection && verticalIntersection;
	return intersecting;
}