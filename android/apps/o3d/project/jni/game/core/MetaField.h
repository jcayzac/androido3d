//	MetaField.h
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


#ifndef _METAFIELD_H
#define _METAFIELD_H

#include "MetaBase.h"
#include "Object.h"

#include "my_assert.h"

#include <string.h>

class MetaField : public Object
{
	public:
		enum MetaType
		{
			TYPE_value,
			TYPE_pointer,
		};
		
		enum ArrayType
		{
			ARRAY_inline,
			ARRAY_dynamic
		};
		
		// Constructor for basic fields.
		MetaField(const MetaType type, const char* pName, const char* pTypeName, int offset, size_t fieldSize) 
			:	mType(type), 
				mpName(pName), 
				mpTypeName(pTypeName), 
				mOffset(offset), 
				mFieldSize(fieldSize), 
				mArrayType(ARRAY_inline),
				mElementCount(1), 
				mElementSize(fieldSize) 
				{};
		
		// Constructor for fields that are arrays.
		MetaField(const MetaType type, const char* pName, const char* pTypeName, int offset, size_t fieldSize, const ArrayType arrayType, int elementCount, size_t elementSize) 
			:	mType(type), 
				mpName(pName), 
				mpTypeName(pTypeName), 
				mOffset(offset), 
				mFieldSize(fieldSize), 
				mArrayType(arrayType),
				mElementCount(elementCount), 
				mElementSize(elementSize) 
				{};
		
		const char* getName() const;
		const char* getTypeName() const;
		const int getOffset() const;
		const size_t getFieldSize() const;
		const MetaType getStorageType() const;
		const ArrayType getArrayType() const;
		
		virtual void* get(const MetaBase* pObject) const;
		virtual void set(MetaBase* pObject, const void* pData) const;
		virtual void copy(const MetaBase* pSource, MetaBase* pDest) const;
		
		// array support
		// The object must be passed so that dynamic arrays can be supported.
		virtual const int getElementCount(const MetaBase* pObject) const;
		const size_t getElementSize() const;
		
		virtual void* getElement(const MetaBase* pObject, const int index) const;
		virtual void setElement(MetaBase* pObject, const int index, const void* pData) const;
		
		virtual const size_t getAllocatedSize(const MetaBase* pObject, const int index) const;
		
		// Makes sure that the reference counts for smart objects are correct.
		virtual void validateRefs(const MetaBase* pObject) const;
		virtual void validateIndividualRef(const MetaBase* pObject, const int index) const;
		
	private:
		const MetaType mType;
		const char* mpName;
		const char* mpTypeName;
		const int mOffset;
		const size_t mFieldSize;	// total size including all elements
		const ArrayType mArrayType;
		const int mElementCount;
		const size_t mElementSize;	// the size of a single element.	mElementSize * mElementCount == mFieldSize
		
};

template<class T>
class TMetaField : public MetaField
{
	public:
		T* get(const void* pObject) const;
};

inline const char* MetaField::getName() const
{
	return mpName;
}

inline const char* MetaField::getTypeName() const
{
	return mpTypeName;
}

inline const int MetaField::getOffset() const
{
	return mOffset;
}

inline const size_t MetaField::getFieldSize() const
{
	return mFieldSize;
}

inline const MetaField::MetaType MetaField::getStorageType() const
{
	return mType;
}

inline const MetaField::ArrayType MetaField::getArrayType() const
{
	return mArrayType;
}

inline const int MetaField::getElementCount(const MetaBase* /*pObject*/) const
{
	return mElementCount;
}

inline const size_t MetaField::getElementSize() const
{
	return mElementSize;
}

inline void* MetaField::get(const MetaBase* pObject) const
{
	ASSERT(pObject != NULL, "NULL object passed to get()!");
	
	return getElement(pObject, 0);
}

inline void MetaField::set(MetaBase* pObject, const void* pData) const
{
	ASSERT(pObject != NULL, "NULL object passed to set()!");
	
	char* pAddress = (char*)pObject->getObjectMemory();
	pAddress += mOffset;
	
	::memcpy(pAddress, pData, getFieldSize());
}

inline void* MetaField::getElement(const MetaBase* pObject, const int index) const
{
	ASSERT(pObject != NULL, "NULL object passed to get()!");
	ASSERT(index >= 0 && index < mElementCount, "Invalid index!");
	
	char* pAddress = NULL;
	
	if (index >= 0 && index < mElementCount)
	{
		pAddress = (char*)pObject->getObjectMemory();
		pAddress += mOffset;
		pAddress += index * getElementSize();
	}
	
	return pAddress;
}

inline void MetaField::setElement(MetaBase* pObject, const int index, const void* pData) const
{
	ASSERT(pObject != NULL, "NULL object passed to set()!");
	ASSERT(index >= 0 && index < mElementCount, "Invalid index!");
	
	if (index >= 0 && index < mElementCount)
	{
		char* pAddress = (char*)pObject->getObjectMemory();
		pAddress += mOffset;
		pAddress += index * getElementSize();
		::memcpy(pAddress, pData, getElementSize());
	}
}

inline void MetaField::copy(const MetaBase* pSource, MetaBase* pDest) const
{
	set(pDest, get(pSource));
}

inline void MetaField::validateRefs(const MetaBase* pObject) const
{
	const int elementCount = getElementCount(pObject);
	for (int x = 0; x < elementCount; x++)
	{
		validateIndividualRef(pObject, x);
	}
}

inline void MetaField::validateIndividualRef(const MetaBase* pObject, const int index) const
{
	// base class knows nothing about ref counting, so do nothing
}


// may be overridden for complex objects.	default returns field size
inline const size_t MetaField::getAllocatedSize(const MetaBase* pObject, const int index) const
{
	ASSERT(pObject != NULL, "NULL object passed to getAllocatedSize()!");
	
	size_t size = 0;
	if (pObject)
	{
		if (getStorageType() == TYPE_value)
		{
			size = getElementSize();
		}
		else
		{
			// if this is a pointer we'll return the field size unless
			// it's set to NULL.	More complex functions could
			// return the actual allocated size if they knew it.
			void** pPointer = static_cast<void**>(getElement(pObject, index));
			if (*pPointer != NULL)
			{
				size = getElementSize();
			}
		}
	}
	
	return size;
}


template<class T>
T* TMetaField<T>::get(const void* pObject) const
{
	return static_cast<T*>(MetaField::get(pObject));
}
#endif //_METAFIELD_H