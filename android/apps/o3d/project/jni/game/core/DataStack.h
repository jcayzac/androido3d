//	DataStack.h
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

#ifndef DATASTACK_H_
#define DATASTACK_H_

#include "DataTable.h"

template <class T>
class DataStack : public DataTable<T>
{
	public:
		DataStack(int maxElements);
		
		void push(const char* name, T element);
		T pop();
		T* peek() const;
		
		const char* getTopName() const;
};


template <class T>
inline DataStack<T>::DataStack(int maxElements)
:	DataTable<T>(false, maxElements, true)
{
}

template <class T>
inline void DataStack<T>::push(const char* name, T element)
{
	add(name, element);
}


template <class T>
inline T DataStack<T>::pop()
{
	T element = get(DataTable<T>::getElementCount() - 1);
	removeIndex(DataTable<T>::getElementCount() - 1);
	return element;
}

template <class T>
inline T* DataStack<T>::peek() const
{
	T* element = getEntry(DataTable<T>::getElementCount() - 1);
	return element;
}

template <class T>
inline const char* DataStack<T>::getTopName() const
{
	return getKey(DataTable<T>::getElementCount() - 1);
}

#endif // DATASTACK_H_
