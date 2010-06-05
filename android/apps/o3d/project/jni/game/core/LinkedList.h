//	LinkedList.h
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


#ifndef _LINKEDLIST_H
#define _LINKEDLIST_H

#include "Object.h"

#include <stdio.h>

// Linked List container.
// This should live in the LinkedList definition (in the protected section),
// but some compilers (VC08) have trouble with nested structs in templated
// classes.
template <class T>
struct ListEntry
{
	T mObject;
	ListEntry* mpPrevious;
	ListEntry* mpNext;
};

template <class T>
class LinkedList : public Object
{
	public:
		LinkedList() : mpList(NULL), mCount(0), mpLastAccessedEntry(NULL), mLastAccessedIndex(-1) { };
		~LinkedList() { removeAll(); };
		
		virtual int add(T object);
		virtual int addUnique(T object);
		virtual void remove(T object);
		void removeIndex(int index);
		void removeAll();
		
		int getCount() const;
		virtual T get(int index) const;
		virtual T getFirst() const;
		virtual T getLast() const;

		int find(T object) const;
		
		T* convertToArray();	// Allocates memory that the caller must delete.
		void setFromArray(T* array, int count);
		
		void printList() const;
		
	protected:
		
		
		void removeEntry(ListEntry<T>* pEntry);
		
		ListEntry<T>* findEntry(const T& object) const;
		ListEntry<T>* findEntryIndex(int index) const;

		// store the last entry we found for fast sequential access
		void setAccessCache(const int index, ListEntry<T>* pEntry) const;
		void clearAccessCache() const;
		
	private:
		// circular linked list
		ListEntry<T>* mpList;
		int mCount;
		
		// access optimization
		mutable ListEntry<T>* mpLastAccessedEntry;
		mutable int mLastAccessedIndex;
};


template <class T>
inline int LinkedList<T>::add(T object)
{
	int index = -1;
	
	ListEntry<T>* pEntry = new ListEntry<T>;
	
	if (pEntry)
	{
		index = mCount;
		mCount++;
		
		// append at the end
		
		if (mpList)
		{
			// maintain circular linked list
			// node -> node -> mpList -> head 
			// becomes
			// node -> node -> mpList -> pEntry -> head
			
			pEntry->mpPrevious = mpList;
			pEntry->mpNext = mpList->mpNext;
			
			pEntry->mpPrevious->mpNext = pEntry;
			pEntry->mpNext->mpPrevious = pEntry;
		}
		else
		{
			pEntry->mpNext = pEntry;
			pEntry->mpPrevious = pEntry;
		}
		
		mpList = pEntry;
		
		pEntry->mObject = object;
	}
	
	clearAccessCache();
	
	return index;
}

template <class T>
inline int LinkedList<T>::addUnique(T object)
{
	int index = -1;
	
	if (findEntry(object) == NULL)
	{
		index = add(object);
	}
	
	return index;
}

template <class T>
inline void LinkedList<T>::remove(T object)
{
	ListEntry<T>* pEntry = findEntry(object);
	
	removeEntry(pEntry);
}

template <class T>
inline void LinkedList<T>::removeIndex(int index)
{
	ListEntry<T>* pEntry = findEntryIndex(index);
	
	removeEntry(pEntry);

}

template <class T>
inline void LinkedList<T>::removeAll()
{
	while (mCount > 0 && mpList != NULL)
	{
		removeEntry(mpList);
	}
	
	clearAccessCache();
}

template <class T>
inline int LinkedList<T>::getCount() const
{
	return mCount;
}

template <class T>
inline T LinkedList<T>::get(int index) const
{
	T result;
	
	ListEntry<T>* pEntry = findEntryIndex(index);
	
	if (pEntry)
	{
		result = pEntry->mObject;
	}
	else
	{
		ASSERT(false, ("Requested get of index that is out of range in linked list."));
	}
	
	return result;
}

template <class T>
inline T LinkedList<T>::getFirst() const
{
	T result;
	
	if (mpList)
	{
		ListEntry<T>* pEntry = mpList->mpNext;
		result = pEntry->mObject;
	}
	else
	{
		ASSERT(false, ("Requested getFirst of a null linked list."));
	}
	
	return result;
}

template <class T>
inline T LinkedList<T>::getLast() const
{	
	T result;
	
	if (mpList)
	{
		ListEntry<T>* pEntry = mpList;
		result = pEntry->mObject;
	}
	else
	{
		ASSERT(false, ("Requested getFirst of a null linked list."));
	}
	
	return result;
}

template <class T>
inline int LinkedList<T>::find(T object) const
{
	int index = -1;
	
	const int count = getCount();
	
	for (int x = 0; x < count; x++)
	{
		if (get(x) == object)
		{
			// found it
			index = x;
			break;
		}
	}
	
	return index;
}

template <class T>
inline void LinkedList<T>::removeEntry(ListEntry<T>* pEntry)
{
	if (pEntry)
	{
		if (mCount == 1)
		{
			// we're removing the only item.
			mpList = NULL;
		}
		else if (pEntry == mpList)
		{
			mpList = pEntry->mpPrevious;
		}
		
		pEntry->mpPrevious->mpNext = pEntry->mpNext;
		pEntry->mpNext->mpPrevious = pEntry->mpPrevious;
		
		
		delete pEntry;
		
		mCount--;
		
		clearAccessCache();
	}
}


template <class T>
inline ListEntry<T>* LinkedList<T>::findEntry(const T& object) const
{
	// linear search, ugh
	ListEntry<T>* pEntry = NULL;
	ListEntry<T>* pCurrentEntry = mpList;
	int index = 0;
	
	// check the cache
	if (mpLastAccessedEntry && mpLastAccessedEntry->mObject == object)
	{
		index = mLastAccessedIndex;
		pEntry = mpLastAccessedEntry;
	}
	else if (mpLastAccessedEntry && mpLastAccessedEntry->mpNext->mObject == object)
	{
		index = mLastAccessedIndex + 1;
		if (index >= getCount())
		{
			index = 0;
		}
		
		pEntry = mpLastAccessedEntry->mpNext;
	}
	else if (mpLastAccessedEntry && mpLastAccessedEntry->mpPrevious->mObject == object)
	{
		index = mLastAccessedIndex - 1;
		if (index < 0)
		{
			index = getCount() - 1;
		}
		
		pEntry = mpLastAccessedEntry->mpPrevious;
	}
	else
	{
		// not in the cache, linear search!
		while (pCurrentEntry && pEntry == NULL)
		{
			pCurrentEntry = pCurrentEntry->mpNext;
			
			if (pCurrentEntry->mObject == object)
			{
				pEntry = pCurrentEntry;
			}
			
			index++;
			
			if (pCurrentEntry == mpList)
			{
				// we've covered the entire list, stop.
				break;
			}
		}
	}
	
	setAccessCache(index, pCurrentEntry);

	
	return pEntry;
}

template <class T>
inline ListEntry<T>* LinkedList<T>::findEntryIndex(int index) const
{
	ListEntry<T>* pEntry = NULL;
	
	if (index < mCount && mCount > 0)
	{
		// check the cache
		if (mLastAccessedIndex == index)
		{
			pEntry = mpLastAccessedEntry;
		}
		else if (mLastAccessedIndex == index + 1 && mLastAccessedIndex > 0)
		{
			pEntry = mpLastAccessedEntry->mpPrevious;
		}
		else if (mLastAccessedIndex == index - 1 && mLastAccessedIndex >= 0)
		{
			pEntry = mpLastAccessedEntry->mpNext;
		}
		else
		{
			// not in the cache, linear search
			ListEntry<T>* pCurrentEntry = mpList->mpNext;
		
			for (int x = 0; x < index; x++)
			{
				pCurrentEntry = pCurrentEntry->mpNext;
			}
			
			pEntry = pCurrentEntry;
		}
		
		setAccessCache(index, pEntry);
	}
	
	return pEntry;
}

template <class T>
void LinkedList<T>::setAccessCache(const int index, ListEntry<T>* pEntry) const
{
	mpLastAccessedEntry = pEntry;
	mLastAccessedIndex = index;
}

template <class T>
void LinkedList<T>::clearAccessCache() const
{
	setAccessCache(-1, NULL);
}	

// Conversion to and from arrays.	These functions do not affect the access cache.
template <class T>
T* LinkedList<T>::convertToArray()
{
	T* array = NULL;
	if (getCount() > 0)
	{
		array = new T[getCount()];
		
		ListEntry<T>* pEntry = mpList;
		int index = 0;
		while (pEntry)
		{
			pEntry = pEntry->mpNext;
			array[index] = pEntry->mObject;
			index++;
			
			if (pEntry == mpList)
			{
				// we've covered the entire list, stop.
				break;
			}
		}
	}
	
	return array;
}

template <class T>
void LinkedList<T>::setFromArray(T* array, int count)
{
	removeAll();
	if (array && count > 0)
	{
		for (int x = 0; x < count; x++)
		{
			add(array[x]);
		}
	}
}
		
template <class T>
void LinkedList<T>::printList() const
{
	for (int x = 0; x < getCount(); x++)
	{
		ListEntry<T>* pEntry = findEntryIndex(x);
		printf("%d)\t%p <-- %p --> %p\n", x, pEntry->mpPrevious, pEntry, pEntry->mpNext);
	}
}

#endif