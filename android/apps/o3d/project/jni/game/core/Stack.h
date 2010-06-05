//	Stack.h
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


#ifndef _STACK_H
#define _STACK_H

#include "LinkedList.h"

template <class T>
class Stack : public LinkedList<T>
{
	public:
		void push(T* object);
		T* pop();
		T* peek() const;

};

template <class T>
inline void Stack<T>::push(T* object)
{
	add(object);
}

template <class T>
inline T* Stack<T>::pop()
{
	T* pObject = LinkedList<T>::getLast();
	remove(pObject);
	return pObject;
}

template <class T>
inline T* Stack<T>::peek() const
{
	T* pObject = LinkedList<T>::getLast();
	return pObject;
}

#endif //_STACK_H