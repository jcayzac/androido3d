//	Object.h
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


#ifndef _OBJECT_H
#define _OBJECT_H

#include "my_assert.h"
#include "Serializable.h"

// global base class
class Object : public Serializable
{
	public:
		Object() : mReferenceCount(0) { };
		virtual ~Object();		// C++ requires a virtual destructor to delete a derived class (w/virtuals) correctly
	
		int getReferenceCount() const;
		void incrementReferenceCount();
		void decrementReferenceCount();
	
	private:
		int mReferenceCount;
};

inline int Object::getReferenceCount() const
{
	return mReferenceCount;
}

inline void Object::incrementReferenceCount()
{
	mReferenceCount++;
}

inline void Object::decrementReferenceCount()
{
	mReferenceCount--;
	
	ASSERT(mReferenceCount >= 0, ("Invalid reference count!"));
	
	if (mReferenceCount <= 0)
	{
		// erase the object when the ref count goes to 0
		// what happens if this was created on the stack?	can we detect that?
		
		delete this;
	}
}


#endif