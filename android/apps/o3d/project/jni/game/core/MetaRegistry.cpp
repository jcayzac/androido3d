//	MetaRegistry.cpp
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


#include "MetaRegistry.h"

#include "MemoryManager.h"
#include "MemoryPool.h"
#include "MetaObject.h"

void MetaRegistry::addMetaObject(const MetaObject* pMeta)
{
	// ensure allocations come out of the MetaRegistry's pool
	MemoryManager::getMemoryManager()->pushMemoryPool(&mMemoryPool);
	
	mMetaTable.addUnique(pMeta->getName(), pMeta);
	
	MemoryManager::getMemoryManager()->popMemoryPool();

}
		
const MetaObject* MetaRegistry::getMetaObject(const char* pTypeName)
{
	int index = mMetaTable.findIndex(pTypeName);
	if (index > -1)
	{
		return mMetaTable.get(index);
	}
	
	return NULL;
}

const MetaObject* MetaRegistry::getMetaObject(const unsigned int typeID)
{
	const int elementCount = mMetaTable.getElementCount();

	for (int x = 0; x < elementCount; x++)
	{
		const MetaObject* pMeta = mMetaTable.get(x);
		if (pMeta->getTypeID() == typeID)
		{
			return pMeta;
		}
	}
	
	return NULL;
}

void* MetaRegistry::allocateByMeta(const MetaObject* pMeta)
{
	void* pResult = NULL;
	if (pMeta)
	{
		pResult = pMeta->allocateRaw();
	}
	
	return pResult;
}

void* MetaRegistry::constructByMeta(const MetaObject* pMeta)
{
	void* pResult = NULL;
	if (pMeta)
	{
		pResult = pMeta->allocateAndConstruct();
	}
	
	return pResult;
}