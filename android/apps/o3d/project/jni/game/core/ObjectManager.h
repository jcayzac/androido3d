//	ObjectManager.h
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


#ifndef _OBJECTMANAGER_H
#define _OBJECTMANAGER_H

#include "ObjectHandle.h"
#include "LinkedList.h"


template <class T>
class ObjectManager : public LinkedList< ObjectHandle<T> >
{
	public:
	
		int add(T* pObject);
		int addUnique(T* pObject);
		void remove(T* pObject);
};

template <class T>
inline int ObjectManager<T>::add(T* pObject)
{
	return LinkedList< ObjectHandle<T> >::add(ObjectHandle<T>(pObject));
}

template <class T>
inline int ObjectManager<T>::addUnique(T* pObject)
{
	return LinkedList< ObjectHandle<T> >::addUnique(ObjectHandle<T>(pObject));
}

template <class T>
inline void ObjectManager<T>::remove(T* pObject)
{
	LinkedList< ObjectHandle<T> >::remove(ObjectHandle<T>(pObject));
}



#endif