//	MemoryManager.cpp
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


#include "MemoryManager.h"

#include "my_assert.h"

MemoryManager::MemoryManager()
:	mPoolTop(0)
{
}

MemoryManager::~MemoryManager()
{
	ASSERT(mPoolTop == 0, ("Memory system shutting down while pools are still pushed!"));
}

void MemoryManager::pushMemoryPool(MemoryPool* pPool)
{
	ASSERT(mPoolTop + 1 < MAX_poolsInStack, ("Attempt to push too many pools!"));
	
	if (mPoolTop + 1 < MAX_poolsInStack)
	{
		mpPoolStack[mPoolTop] = pPool;
		mPoolTop++;
	}
}

void MemoryManager::popMemoryPool()
{
	ASSERT(mPoolTop - 1 >= 0, ("Attempt to pop too many pools!"));
	
	if (mPoolTop - 1 >= 0)
	{
		mPoolTop--;
		mpPoolStack[mPoolTop] = NULL;
	}
}
		
void* MemoryManager::allocate(size_t size)
{
	void* pData = NULL;
	
	if (mPoolTop == 0)
	{
		// allocations during bootstrap time must come out of the default pool
		pData = mDefaultPool.allocate(size);
	}
	else
	{
		pData = mpPoolStack[mPoolTop - 1]->allocate(size);
	}
	
	return pData;
}

void MemoryManager::deallocate(void* pData)
{
	if (mPoolTop == 0)
	{
		// frees during bootstrap time must come out of the default pool
		mDefaultPool.deallocate(pData);
	}
	else
	{
		mpPoolStack[mPoolTop - 1]->deallocate(pData);
	}
}