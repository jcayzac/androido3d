//	MetaObject.cpp
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


#include "MetaObject.h" // self

#include "CRCUtility.h"

#include <string.h>

unsigned int MetaObject::generateTypeIDFromString(const char* pTypeName)
{
	// it's weird that I have to do a reinterpret_cast here.	static_cast can't change signed-ness?
	return calculateCRC<unsigned int>(reinterpret_cast<const unsigned char*>(pTypeName), strlen(pTypeName));
}		

void* MetaObject::allocateRaw(void* pAddress) const
{
	return (mpConstructionFunction)(pAddress, false);
}

void* MetaObject::allocateAndConstruct(void* pAddress) const
{
	return (mpConstructionFunction)(pAddress, true);
}


const MetaField* MetaObject::getField(const int index) const
{
	ASSERT(index >= 0 && index < mTotalFieldCount, ("Invalid field passed to MetaObject::getField"));
	
	const MetaField* pField = NULL;
	int localFieldRangeStart = mTotalFieldCount - mFieldCount;
	if (index >= localFieldRangeStart)
	{
		pField = mpFields[index - localFieldRangeStart];
	}
	else if (mpParentMeta)
	{
		// get this from the parent's meta
		pField = mpParentMeta->getField(index);
	}
	else
	{
		ASSERT(false, ("Invalid field passed to MetaObject::getField!"));
	}
	
	return pField;
}

bool MetaObject::isOfType(const MetaObject* pType) const
{
	bool related = false;
	if (pType)
	{
		if (pType == this)
		{
			related = true;
		}
		else if (mpParentMeta)
		{
			related = mpParentMeta->isOfType(pType);
		}
	}
	
	return related;
}
