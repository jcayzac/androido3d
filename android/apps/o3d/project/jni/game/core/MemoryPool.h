//	MemoryPool.h
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

 
 #ifndef _MEMORYPOOL_H
 #define _MEMORYPOOL_H
 
 #include "Object.h"
 
#include <stdio.h>
 
 class MemoryPool : public Object
 {
	public:
		MemoryPool() : mAllocationCount(0) {};
		~MemoryPool() {};
	
		virtual void* allocate(size_t size);
		virtual void deallocate(void* pData);
		
		int getAllocationCount() const;
		virtual size_t getSize(void* pData); // only implemented for tracked pools
		
	private:
		int mAllocationCount;
 };
 
inline int MemoryPool::getAllocationCount() const
{
	return mAllocationCount;
}

inline size_t MemoryPool::getSize(void*) // this pool isn't tracked
{
	return 0;
}
 
 
 #endif //_MEMORYPOOL_H
 
 