//	GameObject.h
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


#ifndef _GAMEOBJECT_H
#define _GAMEOBJECT_H

#include "Blackboard.h"
#include "GameComponent.h"
#include "ObjectManager.h"
#include "Vector3.h"

class GameObject : public ObjectManager< GameComponent >
{
	public:
		GameObject() : mID(-1), mPosition(0.0f, 0.0f, 0.0f) {};
		
		void update(const float& time);
		
		int getID() const;
		void setID(const int id);
		
		const Vector3& getPosition() const;
		void setPosition(const Vector3& newPosition);
		
		Blackboard* getRuntimeData();
		const Blackboard* getRuntimeData() const;
		
	protected:
		int mID;
		Vector3 mPosition;
		Blackboard mRuntimeData;
};

inline int GameObject::getID() const
{
	ASSERT(mID > -1, "Trying to get the ID for an object that hasn't been added to the game yet!");
	return mID;
}
	
inline void GameObject::setID(const int id)
{
	ASSERT(id > -1, "Trying to set the ID for an object to something invalid!");
	ASSERT(mID == -1, "Trying to change the ID for an object that's already been setup");
	mID = id;
}
		
inline const Vector3& GameObject::getPosition() const
{
	return mPosition;
}

inline void GameObject::setPosition(const Vector3& newPosition)
{
	mPosition = newPosition;
}

inline Blackboard* GameObject::getRuntimeData()
{
	return &mRuntimeData;
}

inline const Blackboard* GameObject::getRuntimeData() const
{
	return &mRuntimeData;
}

#endif // _GAMEOBJECT_H