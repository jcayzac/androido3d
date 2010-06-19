//	SystemRegistry.cpp
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


#include "DataTable.h"
#include "MetaObject.h"
#include "System.h"
#include "SystemRegistry.h"

// define the singleton
SystemRegistry SystemRegistry::s_mSystemRegistry;


void SystemRegistry::addSystem(System* pSystem)
{
	ASSERT(pSystem != NULL, ("Attempting to add a NULL system!"));
	if (pSystem)
	{
		addInheritanceTree(pSystem, pSystem->getMetaObject());
	}
}

void SystemRegistry::removeSystem(const System* pSystem)
{
	ASSERT(pSystem != NULL, ("Attempting to remove a system by NULL pointer!"));
	if (pSystem)
	{
		removeInheritanceTree(pSystem->getMetaObject());
	}
}

void SystemRegistry::removeAll()
{
	mSystems.removeAll();
}

System* SystemRegistry::getSystem(const MetaObject* pMeta)
{
	System* result = NULL;
	
	ASSERT(pMeta != NULL, ("Attempting to get a system by NULL meta!"));
	if (pMeta)
	{
		int index = mSystems.findIndex(pMeta->getTypeID());
		if (index != -1)
		{
			result = mSystems.get(index);
		}
	}
	
	return result;
}

void SystemRegistry::addInheritanceTree(System* pSystem, const MetaObject* pMeta)
{
	if (pSystem && pMeta && pMeta != System::getClassMetaObject())
	{
		mSystems.addUnique(pMeta->getTypeID(), pSystem);
		addInheritanceTree(pSystem, pMeta->getParentMetaObject());
	}
}

void SystemRegistry::removeInheritanceTree(const MetaObject* pMeta)
{
	if (pMeta && pMeta != System::getClassMetaObject())
	{
		int index = mSystems.findIndex(pMeta->getTypeID());
		if (index != -1)
		{
			mSystems.removeIndex(index);
		}
		
		removeInheritanceTree(pMeta->getParentMetaObject());
	}
}
