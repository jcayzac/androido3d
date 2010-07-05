//	DynamicArrayMetaField.h
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


#ifndef _DYNAMICARRAYMETAFIELD_H
#define _DYNAMICARRAYMETAFIELD_H

#include "Array.h"
#include "MetaField.h"

#include "my_assert.h"

#include <string.h>

// T must be a descendent of MetaField
template <class T>
class DynamicArrayMetaField : public T
{
	public:
		DynamicArrayMetaField(const MetaField::MetaType type, const char* pName, const char* pTypeName, int offset, int fieldSize, int elementSize) 
			: T(type, pName, pTypeName, offset, fieldSize, MetaField::ARRAY_dynamic, 1, elementSize) {};
			
		virtual void* get(const MetaBase* pObject) const;
		
		virtual const int getElementCount(const MetaBase* pObject) const;
		
		virtual void* getElement(const MetaBase* pObject, const int index) const;
		virtual void setElement(MetaBase* pObject, const int index, const void* pData) const;
		
		virtual void validateIndividualRef(const MetaBase* pObject, const int index) const;

		void appendElement(MetaBase* pObject, const void* pData) const;
};

template <class T>
inline void* DynamicArrayMetaField<T>::get(const MetaBase* pObject) const
{
	return getElement(pObject, 0);
}

template <class T>
inline const int DynamicArrayMetaField<T>::getElementCount(const MetaBase* pObject) const
{
  // We must use T::getElement(0) to get the pointer to the array object rather than its first entry.
	ArrayBase* pArray = static_cast<ArrayBase*>(T::getElement(pObject, 0));
	int count = 0;
	if (pArray)
	{
		count = pArray->getCount();
	}
	return count;
}

template <class T>		
inline void* DynamicArrayMetaField<T>::getElement(const MetaBase* pObject, const int index) const
{
	void* result = NULL;
	// We must use T::getElement(0) to get the pointer to the array object rather than its first entry.
	ArrayBase* pArray = static_cast<ArrayBase*>(T::getElement(pObject, 0));
	if (pArray && pArray->getCount() > index)
	{
		void* pointerToElement = pArray->getElement(index);
		result = pointerToElement;
	}
	return result;
}

template <class T>
inline void DynamicArrayMetaField<T>::setElement(MetaBase* pObject, const int index, const void* pData) const
{
  // We must use T::getElement(0) to get the pointer to the array object rather than its first entry.
	ArrayBase* pArray = static_cast<ArrayBase*>(T::getElement(pObject, 0));
	if (pArray && pArray->getCount() > index)
	{
		pArray->setElement(index, const_cast<void*>(pData));
	}
}

template <class T>
inline void DynamicArrayMetaField<T>::validateIndividualRef(const MetaBase* /*pObject*/, const int /*index*/) const
{
	// if this is a ref-counted object, the array will increment the ref count on assignment
	// so we don't have to do anything here.
}

template <class T>
inline void DynamicArrayMetaField<T>::appendElement(MetaBase* pObject, const void* pData) const
{
  // We must use T::getElement(0) to get the pointer to the array object rather than its first entry.
	ArrayBase* pArray = static_cast<ArrayBase*>(T::getElement(pObject, 0));
	if (pArray)
	{
		pArray->appendElement(const_cast<void*>(pData));
	}
}

#endif  //_DYNAMICARRAYMETAFIELD_H