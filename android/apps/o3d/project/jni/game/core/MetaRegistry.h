//	MetaRegistry.h
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


#ifndef _METAREGISTRY_H
#define _METAREGISTRY_H

#include "Object.h"
#include "DataTable.h"
#include "MemoryPool.h"

class MetaObject;

class MetaRegistry : public Object
{
	public:
		enum
		{
			DEFAULT_maxMetaObjects = 256,
		};
		
		MetaRegistry() : mMetaTable(true, DEFAULT_maxMetaObjects, true) { };
		
		// singleton
		static MetaRegistry* getMetaRegistry();
		
		void addMetaObject(const MetaObject* pMeta);
		const MetaObject* getMetaObject(const char* pTypeName);
		const MetaObject* getMetaObject(const unsigned int typeID);
		
		void* allocateByMeta(const MetaObject* pMeta);	// Creates the object and returns it.
		void* constructByMeta(const MetaObject* pMeta);	// Creates the object and initializes it.
		
	private:
		MemoryPool mMemoryPool;
		DataTable<const MetaObject*> mMetaTable;
		
};

inline MetaRegistry* MetaRegistry::getMetaRegistry()
{
	static MetaRegistry s_MetaRegistry;
	return &s_MetaRegistry;
}


#endif //_METAREGISTRY_H