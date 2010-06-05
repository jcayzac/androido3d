//	KeyValueMap.h
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


#ifndef _KEYVALUEMAP_H
#define _KEYVALUEMAP_H

#include "Array.h"

// This shouldn't be in the global namespace, but the compiler (gcc4, anyway) has
// trouble finding it for derived instances of KeyValueMap if the struct appears within
// the class.	Promoting it out mysteriously solves the problem.
template <typename Key, typename Value>
struct KeyMapElementEntry
{
	KeyMapElementEntry() {};
	KeyMapElementEntry(Value element, Key key) : mElement(element), mKey(key) {};
	
	Value mElement;
	Key mKey;
};
		
template <typename Key, typename Value>
class KeyValueMap
{
	public:
		enum
		{
			DEFAULT_maxElements = 64,
		};
		
		KeyValueMap(bool keepSorted, int maxElements);
		~KeyValueMap();
		
		void add(Key key, Value element);
		int addUnique(Key key, Value element);
		
		void remove(Key key);
		void removeIndex(const int index);
		void removeAll();
		
		void sort();
		
		bool find(Key key, Value& result) const;
		int findIndex(Key key) const;

		Value get(const int index) const;
		Value* getEntry(int index) const;
		Value* getEntry(int index);
		
		Key getKey(const int index) const;
		void set(const int index, Value element);
	
		bool getKeepSorted() const;
		int getElementCount() const;
		int getMaxElements() const;

	protected:
		// This is sort of a hack.	If the ElementEntry is inlined in the class,
		// the compiler can't seem to find it for derived classes.	So it is
		// unfortunately pushed out into the global namespace.
		typedef KeyMapElementEntry<Key, Value> ElementEntry;
		
		virtual int compare(Key key1, Key key2) const;
		
		// override these to do more complex management of entries.
		// base class does nothing.
		virtual void constructElement(ElementEntry* pEntry);	// Called after element is added.
		virtual void destroyElement(ElementEntry* pEntry);		// Called before element is destroyed.
	
	private:
		Array<ElementEntry> mTable;
		int mMaxElements;
		
		bool mKeepSorted;
};


template <typename Key, typename Value>
inline KeyValueMap<Key, Value>::KeyValueMap(bool keepSorted, int maxElements)
:	mTable(maxElements), // TODO: it's no longer really necessary that this object be of fixed size.  Would it be better to just let it grow?
	mMaxElements(maxElements),
	mKeepSorted(keepSorted)
{
}

template <typename Key, typename Value>
inline KeyValueMap<Key, Value>::~KeyValueMap()
{
}

template <typename Key, typename Value>
inline void KeyValueMap<Key, Value>::add(Key key, Value element)
{
	assert(getElementCount() + 1 <= mMaxElements);
	
	if (getElementCount() + 1 <= mMaxElements)
	{
		ElementEntry entry(element, key);
		
		// Give derived classes a chance to do some setup work.
		constructElement(&entry);
		
		mTable.append(entry);
		
		if (getKeepSorted())
		{
			sort();
		}
	}
}

template <typename Key, typename Value>
inline int KeyValueMap<Key, Value>::addUnique(Key key, Value element)
{
	int index = findIndex(key);
	
	if (index == -1)
	{
		add(key, element);
		index = findIndex(key);
	}
	
	return index;
}


template <typename Key, typename Value>
inline void KeyValueMap<Key, Value>::remove(Key key)
{
	assert(getElementCount() > 0);
	
	int index = findIndex(key);
	
	if (index > -1)
	{
		removeIndex(index);
	}
}

template <typename Key, typename Value>
inline void KeyValueMap<Key, Value>::removeIndex(const int index)
{
	assert(getElementCount() > 0);
	assert(index >= 0);
	
	if (index >= 0)
	{
		// Give derived classes a chance to do special tear-down.
		ElementEntry entry = mTable.get(index);
		destroyElement(&entry);
		mTable.remove(index);
		// Array<> supports in-placce removal, so no sorting is necessary here.
	}
}

template <typename Key, typename Value>
inline void KeyValueMap<Key, Value>::removeAll()
{
	for (int x = getElementCount() - 1; x >= 0; x--)
	{
		removeIndex(x);
	}
}

template <typename Key, typename Value>
inline bool KeyValueMap<Key, Value>::find(Key key, Value& result) const
{
	assert(getElementCount() > 0);
		
	int index = findIndex(key);
	
	bool foundResult = false;
	if (index != -1)
	{
		foundResult = true;
		result = get(index);
	}
	
	return foundResult;
}

template <typename Key, typename Value>
inline Value KeyValueMap<Key, Value>::get(const int index) const
{
	assert(getElementCount() > 0);
	assert(index >= 0);
	
	return mTable.get(index).mElement;
}

template <typename Key, typename Value>
inline Key KeyValueMap<Key, Value>::getKey(const int index) const
{
	assert(getElementCount() > 0);
	assert(index >= 0);
	assert(index < mTable.getCount());
	
	return mTable.get(index).mKey;
}

template <typename Key, typename Value>
inline void KeyValueMap<Key, Value>::set(const int index, Value element)
{
	assert(getElementCount() > 0);
	assert(index >= 0);
	assert(index <  mTable.getCount());
	
	mTable.getSlot(index)->mElement = element;
}

template <typename Key, typename Value>
inline bool KeyValueMap<Key, Value>::getKeepSorted() const
{
	return mKeepSorted;
}
		
template <typename Key, typename Value>
inline int KeyValueMap<Key, Value>::getElementCount() const
{
	return mTable.getCount();
}

template <typename Key, typename Value>
inline int KeyValueMap<Key, Value>::getMaxElements() const
{
	return mMaxElements;
}

template <typename Key, typename Value>
inline int KeyValueMap<Key, Value>::findIndex(Key key) const
{
	int testIndex = -1;

	if (getElementCount() > 0)
	{
		if (getKeepSorted())
		{
			// binary search
			int left = 0;
			int right = getElementCount() - 1;
		
			if (right == 0)
			{
				if (compare(mTable.get(0).mKey, key) == 0)
				{
					testIndex = 0;
				}
			}
			else
			{
				while (left <= right)
				{
					testIndex = (left + right) / 2;
					
					// test this index
					int value = compare(key, mTable.get(testIndex).mKey);
					
					if (value < 0) // left half
					{
						right = testIndex - 1;
					}
					else if (value > 0) // right half
					{
						left = testIndex + 1;
					}
					else if (value == 0) // match!
					{
						break;
					}
					
					testIndex = -1;
				}
			}
		}
		else // no sorting = slow linear search
		{
			int listSize = getElementCount();
			for (int x = 0; x < listSize; x++)
			{
				if (compare(mTable.get(x).mKey, key) == 0)
				{
					testIndex = x;
					break;
				}
			}
		}
	}
	
	return testIndex;
}


template <typename Key, typename Value>
inline void KeyValueMap<Key, Value>::sort()
{
	// insertion sort
	ElementEntry temp;
	
	for (int i = 1; i < getElementCount(); i++)
	{
		// This is weird syntax, but it avoids an odd copy constructor error that gcc4 generates
		// with Value types that themselves define copy constructors (like ObjectHandle).
		temp = *mTable.getSlot(i);
		int j = i;
		
		while ((j > 0) && (compare(mTable.get(j - 1).mKey, temp.mKey) > 0))
		{
			mTable.set(j, mTable.get(j - 1));
			j = j - 1;
		}
		
		mTable.set(j, temp);
	}
}

template <typename Key, typename Value>
inline Value* KeyValueMap<Key, Value>::getEntry(int index) const
{
	assert(getElementCount() > 0);
	assert(index >= 0);
	assert(index < getElementCount());
	
	return &mTable.getSlot(index)->mElement;
}

template <typename Key, typename Value>
inline Value* KeyValueMap<Key, Value>::getEntry(int index)
{
	assert(getElementCount() > 0);
	assert(index >= 0);
	assert(index < getElementCount());
	
	return &mTable.getSlot(index)->mElement;
}

template <typename Key, typename Value>
inline int KeyValueMap<Key, Value>::compare(Key key1, Key key2) const
{
	int compareResult = 0;
	
	if (key1 < key2)
	{
		compareResult = -1;
	}
	else if (key1 > key2)
	{
		compareResult = 1;
	}
	
	return compareResult;
}

// Base class requires no further setup.
template <typename Key, typename Value>
void KeyValueMap<Key, Value>::constructElement(ElementEntry* pEntry)
{

}

template <typename Key, typename Value>
void KeyValueMap<Key, Value>::destroyElement(ElementEntry* pEntry)
{

}
		
/*
template <typename Key, typename Value>
inline void KeyValueMap<Key, Value>::quicksort(int leftIndex, int rightIndex)
{
	if(leftIndex < rightIndex)  // do nothing if array contains fewer than two elements
	{
		swap(leftIndex, (leftIndex + rightIndex) / 2);

		int lastIndex = leftIndex;
		
		for(int i = leftIndex + 1; i <= rightIndex; i++)
		{
			if(compare(mTable.get(i).mKey, mTable.get(leftIndex).mKey) < 0 )
			{
				lastIndex++;
				swap(lastIndex, i);
			}
		}
		
		swap(leftIndex, lastIndex);
		quicksort(leftIndex, lastIndex - 1);
		quicksort(lastIndex + 1, rightIndex);
	}
}

template <typename Key, typename Value>
void KeyValueMap<Key, Value>::swap(int i, int j)
{
	ElementEntry temp;
	// This is weird syntax, but it avoids an odd copy constructor error that gcc4 generates
	// with Value types that themselves define copy constructors (like ObjectHandle).
	temp = *mTable.getSlot(i);
	mTable.set(i, mTable.get(j));
	mTable.set(j, temp);
}
*/
#endif //_KEYVALUEMAP_H