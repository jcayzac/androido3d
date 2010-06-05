//	MemoryPoolTracked.cpp
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


#include "MemoryPoolTracked.h"
#include "MemoryManager.h"
#include "MetaBase.h"
#include "MetaObject.h"

void* MemoryPoolTracked::allocate(size_t size)
{
	void* pData = MemoryPool::allocate(size);
	
	if (pData)
	{
		MemoryManager::getMemoryManager()->pushMemoryPool(MemoryManager::getMemoryManager()->getDefaultPool());
		mAllocationList.add(pData);
		mAllocationSizeList.add(size);
		MemoryManager::getMemoryManager()->popMemoryPool();
	}
	
	return pData;
}

void MemoryPoolTracked::deallocate(void* pData)
{
	if (pData)
	{
		const int index = findAllocation(pData);
		if (index >= 0)
		{
			MemoryManager::getMemoryManager()->pushMemoryPool(MemoryManager::getMemoryManager()->getDefaultPool());
			mAllocationList.removeIndex(index);
			mAllocationSizeList.removeIndex(index);
			MemoryManager::getMemoryManager()->popMemoryPool();
		}
		else
		{
			ASSERT(false, "Deallocating memory from the wrong pool!");
		}
	}
	
	MemoryPool::deallocate(pData);
}

size_t MemoryPoolTracked::getSize(void* pData)
{
	const int index = findAllocation(pData);
	size_t size = 0;
	if (index >= 0)
	{
		size = mAllocationSizeList.get(index);
	}
	return size;
}

int MemoryPoolTracked::findAllocation(void* pData)
{
	return mAllocationList.find(pData);
}

void MemoryPoolTracked::dumpAllocations()
{
	printf("\nDumping Tracked Pool %p\n", this);
	printf("---------------------------------------------------------------\n");
	printf("\t\tAddress\t\tSize\t\tType\t\t\t\tContents\n");
	printf("---------------------------------------------------------------\n");
	int allocCount = mAllocationList.getCount();
	size_t totalSize = 0;
	for (int x = 0; x < allocCount; x++)
	{
		void* pAddress = mAllocationList.get(x);
		size_t size = mAllocationSizeList.get(x);
		totalSize += size;
		
		char const* pAllocationTypeName = "<?>";
		char pStringBuffer[15];
		pStringBuffer[0] = 0;
		
		// try to cast this pointer as a meta base
		if (MetaBase::authenticatePointer(pAddress))
		{
			// this looks like a MetaBase object.	Cast and get the meta object name!
			MetaBase* pObject = reinterpret_cast<MetaBase*>(pAddress);
			pAllocationTypeName = pObject->getMetaObject()->getName();
		}
		else 
		{
			// might this be a string?
			char* pString = static_cast<char*>(pAddress);
			
			const int length = size < 15 ? size : 15;
			
			bool isString = true;
			
			for (int x = 0; x < length && isString; x++)
			{
				if ((pString[x] >= ' ' && pString[x] <= '~') || 
					pString[x] == 0 ||
					pString[x] == ' ' ||
					pString[x] == '\t')
				{
					pStringBuffer[x] = pString[x];
				}
				else
				{
					isString = false;
					break;
				}
			}
			
			if (isString)
			{	
				pAllocationTypeName = "char*";
				pStringBuffer[length - 1] = 0;
			}
			else
			{
				pStringBuffer[0] = 0;
			}
		
		}
		
		printf("%d.\t\t%p\t\t%d\t\t%s\t\t\t\t%s\n", x + 1, pAddress, static_cast<int>(size), pAllocationTypeName, pStringBuffer);
	}
	
	printf("---------------------------------------------------------------\n");
	printf("Total allocations: %d\tTotal Size: %d\n", mAllocationList.getCount(), static_cast<int>(totalSize));
	printf("---------------------------------------------------------------\n");
}