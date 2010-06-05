//	ObjectHandle.h
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

 
#ifndef _OBJECTHANDLE_H
#define _OBJECTHANDLE_H

#include "Object.h"
#include "Serializable.h"

#ifndef NULL
#define NULL 0
#endif

template<class T>
class ObjectHandle : public Serializable
{
	public:
		ObjectHandle() 
		{ 
			// only nullify object pointer if this isn't in-place creation.
			if (getCreateInPlace() == false) 
			{ 
				mpObject = NULL; 
			} 
		}
			
		ObjectHandle(T* pObject) 
		:	mpObject(pObject)
		{ 
			if (mpObject)
			{
				mpObject->incrementReferenceCount();
			}
		}
		
		ObjectHandle(const ObjectHandle& pObjectHandle) 
		:	mpObject(pObjectHandle.getAsObject())
		{ 
			if (mpObject)
			{
				mpObject->incrementReferenceCount();
			}
		}
			
		~ObjectHandle() 
		{
			if (mpObject)
			{
				mpObject->decrementReferenceCount();
			}
		}
		
		operator T*();
		operator T*() const;
		T* operator ->();
		T* operator ->() const;
		const ObjectHandle<T>& operator = (T* pOther);
		const ObjectHandle<T>& operator = (ObjectHandle<T>& pOther);
		bool operator == (const T* pOther) const;
		bool operator == (const ObjectHandle<T>& pOther) const;
		T* get();
		T* get() const;
		Object* getAsObject() const;
	private:
		Object* mpObject;
};


template<class T>
inline ObjectHandle<T>::operator T*()
{
	return static_cast<T*>(mpObject);
}

template<class T>
inline ObjectHandle<T>::operator T*() const
{
	return static_cast<T*>(mpObject);
}

template<class T>
inline T* ObjectHandle<T>::operator ->()
{
	ASSERT(mpObject != NULL, ("Attempt to dereference a NULL pointer!"));
	return static_cast<T*>(mpObject);
}

template<class T>
inline T* ObjectHandle<T>::operator ->() const
{
	ASSERT(mpObject != NULL, ("Attempt to dereference a NULL pointer!"));
	return static_cast<T*>(mpObject);
}

template<class T>
inline const ObjectHandle<T>& ObjectHandle<T>::operator = (T* pOther)
{
	// if this handle already has an object, reduce the count
	if (mpObject)
	{
		mpObject->decrementReferenceCount();
	}
	
	mpObject = pOther;

	if (pOther != NULL)
	{
		mpObject->incrementReferenceCount();
	}
	
		
	return *this;
}

template<class T>
inline const ObjectHandle<T>& ObjectHandle<T>::operator = (ObjectHandle<T>& pOther)
{
	// if this handle already has an object, reduce the count
	if (mpObject)
	{
		mpObject->decrementReferenceCount();
	}
	
	mpObject = pOther.mpObject;

	if (mpObject != NULL)
	{
		mpObject->incrementReferenceCount();
	}
	
	return *this;
}

template<class T>
inline bool ObjectHandle<T>::operator == (const T* pOther) const 
{
	return get() == pOther;
}

template<class T>
inline bool ObjectHandle<T>::operator == (const ObjectHandle<T>& pOther) const
{
	return get() == pOther.get();
}

template<class T>
inline T* ObjectHandle<T>::get()
{
	// This should be a static_cast<>, but the type shouldn't need to be known for this object to be defined.
	return (T*)(mpObject);
}

template<class T>
inline T* ObjectHandle<T>::get() const
{
	return (T*)(mpObject);
}

template<class T>
inline Object* ObjectHandle<T>::getAsObject() const
{
	return mpObject;
}
#endif