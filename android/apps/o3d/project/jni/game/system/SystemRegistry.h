//	SystemRegistry.h
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


#ifndef _SYSTEMREGISTRY_H
#define _SYSTEMREGISTRY_H

#include "KeyValueMap.h"
#include "Object.h"
#include "ObjectHandle.h"

#include "my_assert.h"

class MetaObject;
class System;

// typedef KeyValueMap<unsigned int, ObjectHandle<System> > SystemMap;

class SystemMap : public KeyValueMap<unsigned int, ObjectHandle<System> >
{
	public:
		enum
		{
			MAX_systemCount = 32
		};
		
		SystemMap() : KeyValueMap<unsigned int, ObjectHandle<System> >(true, MAX_systemCount) { };
};


class SystemRegistry : public Object
{
	public:
		static SystemRegistry* getSystemRegistry();
		
		void addSystem(System* pSystem);
		void removeSystem(const System* pSystem);
		void removeAll();
		
		// Returns a system that has been added to the registry.	Note that
		// meta objects from higher up in the inheritance tree can be passed
		// here.	The comparison is effectively a dynamic_cast.
		System* getSystem(const MetaObject* pMeta);
		
		System* get(const int index);
		int getCount() const;
	
	protected:
		void addInheritanceTree(System* pSystem, const MetaObject* pMeta);
		void removeInheritanceTree(const MetaObject* pMeta);
		
	private:
		SystemMap mSystems;
		// singleton
		static SystemRegistry s_mSystemRegistry;

};

inline SystemRegistry* SystemRegistry::getSystemRegistry()
{
	return &s_mSystemRegistry;
}

inline System* SystemRegistry::get(const int index)
{
  return mSystems.get(index);
}

inline int SystemRegistry::getCount() const
{
  return mSystems.getElementCount();
}

template <typename T>
T* getSystem()
{
	return static_cast<T*>(SystemRegistry::getSystemRegistry()->getSystem(T::getClassMetaObject()));
}



#endif //_SYSTEMREGISTRY_H