//	MetaBase.cpp
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


#include "MetaBase.h"

#include "MetaObject.h"
#include "MetaField.h"
#include "my_assert.h"

// copy all the fields in some other object into this object.
// note that this object must be castable to pOther's type for this function to work.
void MetaBase::copy(const MetaBase* pOther)
{
	bool areTypesRelated = isOfType(pOther->getMetaObject());
	ASSERT(areTypesRelated, ("Attempt to copy unrelated types!"));
	
	if (areTypesRelated)
	{
		const MetaObject* pMeta = getMetaObject();
		
		for (int x = 0; x < pMeta->getFieldCount(); x++)
		{
			const MetaField* pField = pMeta->getField(x);
			pField->set(this, pField->get(pOther));
		}
	}
}

bool MetaBase::isOfType(const MetaObject* pOtherMeta)
{
	return getMetaObject()->isOfType(pOtherMeta);
}
	
bool MetaBase::isType(const MetaObject* pOtherMeta)
{
	return getMetaObject() == pOtherMeta;
}
