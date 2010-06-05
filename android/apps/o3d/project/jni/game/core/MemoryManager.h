//	MemoryManager.h
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


#ifndef _MEMORYMANAGER_H
#define _MEMORYMANAGER_H

#include "MemoryPool.h"
#include "Object.h"
#include "ObjectHandle.h"

class MemoryManager : public Object
{
	public:
		MemoryManager();
		~MemoryManager();
		
		static inline MemoryManager* getMemoryManager();	// singleton access
		
		void pushMemoryPool(MemoryPool* pPool);
		void popMemoryPool();
		
		void* allocate(size_t size);
		void deallocate(void* pData);
		
		MemoryPool* getDefaultPool();
		MemoryPool* getCurrentPool();
				
	private:
		enum
		{
			MAX_poolsInStack = 8,
		};
		
		// during bootstrap we have no memory pools to use for allocation
		MemoryPool mDefaultPool;
		MemoryPool* mpPoolStack[MAX_poolsInStack];
		int mPoolTop;
};

MemoryManager* MemoryManager::getMemoryManager()
{
	// singleton
	static MemoryManager s_mMemoryManager;
	
	return &s_mMemoryManager;
}

inline MemoryPool* MemoryManager::getDefaultPool()
{
	return &mDefaultPool;
}

inline MemoryPool* MemoryManager::getCurrentPool()
{
	MemoryPool* pPool = &mDefaultPool;
	if (mPoolTop > 0)
	{
		pPool = mpPoolStack[mPoolTop - 1];
	}
	
	return pPool;
}



// override global new and delete
inline void* operator new(size_t size)
{
	return MemoryManager::getMemoryManager()->allocate(size);
}

inline void operator delete(void* pData)
{
	MemoryManager::getMemoryManager()->deallocate(pData);
}


#endif