//	ObjectHandleMetaField.h
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


#ifndef _OBJECTHANDLEMETAFIELD_H
#define _OBJECTHANDLEMETAFIELD_H

#include "MetaField.h"
#include "Object.h"
#include "ObjectHandle.h"

#include "my_assert.h"

class ObjectHandleMetaField : public MetaField
{
	public:
		ObjectHandleMetaField(const MetaType type, const char* pName, const char* pTypeName, int offset, int fieldSize) 
			: MetaField(type, pName, pTypeName, offset, fieldSize) {};
		
		ObjectHandleMetaField(const MetaType type, const char* pName, const char* pTypeName, int offset, int fieldSize, const ArrayType arrayType, int elementCount, size_t elementSize) 
			: MetaField(type, pName, pTypeName, offset, fieldSize, arrayType, elementCount, elementSize) {};
		
		virtual void validateIndividualRef(const MetaBase* pObject, const int index) const;
		virtual const size_t getAllocatedSize(const MetaBase* pObject, const int index) const;

};

inline void ObjectHandleMetaField::validateIndividualRef(const MetaBase* pObject, const int index) const
{
	void** pPointerPointer = static_cast<void**>(getElement(pObject, index));
	Object* pRefObject = static_cast<Object*>(*pPointerPointer);
	if (pRefObject)
	{
		pRefObject->incrementReferenceCount();
	}
}

inline const size_t ObjectHandleMetaField::getAllocatedSize(const MetaBase* pObject, const int index) const
{
	size_t size = 0;
	
	// we want to return the size of the object pointed to by the object handle,
	// not the size of the object handle itself.
	
	ObjectHandle<Object>* pHandle = reinterpret_cast<ObjectHandle<Object>*>(MetaField::getElement(pObject, index));
	
	if (pHandle && pHandle->get())
	{
		size = sizeof(Object*);
	}
	
	return size;
}

#endif //_OBJECTHANDLEMETAFIELD_H