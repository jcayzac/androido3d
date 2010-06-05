//	StringMetaField.h
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


#ifndef _STRINGMETAFIELD_H
#define _STRINGMETAFIELD_H

#include "MetaField.h"

#include "my_assert.h"

#include <string.h>

class StringMetaField : public MetaField
{
	public:
		StringMetaField(const MetaType type, const char* pName, const char* pTypeName, int offset, int fieldSize) 
			: MetaField(type, pName, pTypeName, offset, fieldSize) {};
		
		StringMetaField(const MetaType type, const char* pName, const char* pTypeName, int offset, int fieldSize, const ArrayType arrayType, int elementCount, size_t elementSize) 
			: MetaField(type, pName, pTypeName, offset, fieldSize, arrayType, elementCount, elementSize) {};
		
		virtual const size_t getAllocatedSize(const MetaBase* pObject, const int index) const;

};

inline const size_t StringMetaField::getAllocatedSize(const MetaBase* pObject, const int index) const
{
	size_t size = MetaField::getAllocatedSize(pObject, index);
	
	// if MetaField::getAllocatedSize() returns a non-zero value, we know
	// that the object is valid and there is non-null memory at this
	// field address.	We'll treat it as a string and get the length.
	if (size > 0)
	{
		char** ppString = static_cast<char**>(getElement(pObject, index));
		size = (strlen(*ppString) + 1) * sizeof(char);
	}
	
	return size;
}

#endif