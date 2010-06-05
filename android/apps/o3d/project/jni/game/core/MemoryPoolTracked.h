//	MemoryPoolTracked.h
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


#ifndef _MEMORYPOOLTRACKED_H
#define _MEMORYPOOLTRACKED_H

#include "my_assert.h"

#include "LinkedList.h"
#include "MemoryPool.h"

class MemoryPoolTracked : public MemoryPool
{
	public:
		MemoryPoolTracked() {};
		~MemoryPoolTracked() 
		{
			ASSERT(mAllocationList.getCount() == 0, 
				("Leak Detected!	Freeing tracked memory pool with outstanding allocations!")); 
		};
	
		virtual void* allocate(size_t size);
		virtual void deallocate(void* pData);
		
		size_t getSize(void* pData);
		
		void dumpAllocations();
		
	protected:
		int findAllocation(void* pData);
		
	private:
		LinkedList<void*> mAllocationList;
		LinkedList<size_t> mAllocationSizeList;
};


#endif
