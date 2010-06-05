//	Array.h
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


#ifndef _ARRAY_H
#define _ARRAY_H

#include "Serializable.h"

#include <new>
#include <string.h>	// for memcpy

#include "my_assert.h"

// ArrayBase defines an anonymous interface for manipulating templated arrays without knowing what the template
// type is.  These functions should not be used for normal everyday array operations!
class ArrayBase : public Serializable
{
	public:
		virtual int getCount() const = 0;
		virtual size_t getElementSize() const = 0;
		virtual void* getElement(const int index) const = 0;
		virtual void setElement(const int index, void* data) = 0;
		virtual void appendElement(void* data) = 0;
		virtual void* getMemory() = 0;
		virtual void setMemory(void* address, int count) = 0;
};

template <class T>
class Array : public ArrayBase
{
	public:
		Array();
		Array(T* data, int count);
		Array(int initialCapacity);
		~Array();
		
		int getCapacity() const;
		virtual int getCount() const;
		
		void setCapacity(const int count);
		void reserve(const int count);
		
		void append(T data);
		void set(int index, T data);
		
		void remove(const int index);
		void removeAll();
		
		T get(const int index);
		T get(const int index) const;
		T* getSlot(const int index);
		T* getSlot(const int index) const;
		
		void copyFrom(const Array<T>& other);
		void copyFrom(const T* rawArray, const int count);
		
		// prevents resizing.
		void lock();
		void unlock();
		
	protected:
		enum
		{
			MAX_resizeIncrement = 1024,
			MIN_initialCapacity = 4,
		};
		
		void resize(const int newCapacity);
		void construct(const int index);
		void destruct(const int index);
		
		virtual size_t getElementSize() const;
		virtual void* getElement(const int index) const;
		virtual void setElement(const int index, void* data);
		virtual void appendElement(void* data);
		virtual void* getMemory();
		virtual void setMemory(void* address, int count);
		
	private:
		T* mArray;
		int mCount;
		int mCapacity;
		bool mResizable;
		bool mMemoryOwned;
};

template <class T>
inline Array<T>::Array()
{
	if (!getCreateInPlace())
	{
		mArray = NULL;
		mCount = 0;
		mCapacity = 0;
		mResizable = true;
		mMemoryOwned = true;
	}
}

template <class T>
inline Array<T>::Array(T* data, int count)
:	mArray(data),
	mCount(count),
	mCapacity(count),
	mResizable(false),
	mMemoryOwned(false)
{
}

template <class T>
inline Array<T>::Array(int initialCapacity)
{
	mArray = NULL;
	mCount = 0;
	mCapacity = 0;
	mResizable = true;
	mMemoryOwned = true;
	
	reserve(initialCapacity);
}

template <class T>
inline Array<T>::~Array()
{
	if (mMemoryOwned && mArray != NULL)
	{
		resize(0);
	}
}
		
template <class T>
inline int Array<T>::getCapacity() const
{
	return mCapacity;
}

template <class T>
inline int Array<T>::getCount() const
{
	return mCount;
}

template <class T>
inline void Array<T>::setCapacity(const int count)
{
	resize(count);
}

template <class T>
inline void Array<T>::reserve(const int count)
{
	if (count > getCapacity())
	{
		resize(count);
	}
}

template <class T>
inline void Array<T>::append(T data)
{
	if (getCount() == getCapacity())
	{
		if (getCount() < MAX_resizeIncrement)
		{
			int newSize = MIN_initialCapacity;
			if (getCount() != 0)
			{
				newSize = getCount() * 2;
			}
			
			reserve(newSize);
		}
		else
		{
			reserve(getCount() + MAX_resizeIncrement);
		}
	}
	
	construct(mCount);
	mArray[mCount] = data;
	mCount++;
}

template <class T>
inline void Array<T>::set(int index, T data)
{
	if (index < getCount())
	{
		mArray[index] = data;
	}
}

template <class T>
inline void Array<T>::remove(const int index)
{
	if (index >= 0 && index < getCount())
	{
		destruct(index);
		if (index + 1 < getCount())
		{
			::memcpy(mArray + index, mArray + index + 1, sizeof(T) * (getCount() - (index + 1)));
		}
		mCount--;
	}
}

template <class T>
inline void Array<T>::removeAll()
{
	for (int x = 0; x < mCount; x++)
	{
		destruct(x);
	}
	
	mCount = 0;
}

template <class T>
inline T Array<T>::get(const int index)
{
	return mArray[index];
}

template <class T>
inline T Array<T>::get(const int index) const
{
	return mArray[index];
}

template <class T>
inline T* Array<T>::getSlot(const int index)
{
	return &mArray[index];
}

template <class T>
inline T* Array<T>::getSlot(const int index) const
{
	return &mArray[index];
}

template <class T>
inline void Array<T>::copyFrom(const Array<T>& other)
{
	removeAll();
	const int count = other.getCount();
	reserve(count);
	for (int x = 0; x < count; x++)
	{
		append(other.get(x));
	}
}

template <class T>
inline void Array<T>::copyFrom(const T* rawArray, const int count)
{
	ASSERT(rawArray != NULL, "NULL array passed to copyFrom()!");
	removeAll();
	reserve(count);
	for (int x = 0; x < count; x++)
	{
		append(rawArray[x]);
	}
}
// Resize the internal storage array.	This function defines the core semantics of the Array class.
// Problem: we need internal storage to move around in memory without invoking constructors or destructors.
// This is fairly involved in C++.	We want the user to treat this class as if the array is
// actually growing and shrinking in place, even though what is really happening is that new
// arrays are being allocated and copied to when a change in size is requested.	We must obey the following
// semantic rules:
//	- Constructors are called for all objects within internal storage that are added by the users, but
//		not those in "empty" slots.	Each empty slot will be constructed just before assignment.	Otherwise
//		objects that define copy constructors and expect to have been constructed will be undefined upon assignment.
//	- Destructors are not called when the array resizes UNLESS it gets smaller (in which case the items
//		cut off the end of the array are destructed).
//	- Copy constructors and operator= overrides must not be invoked during resize.	Objects that privatize these
//		functions must still be insertable into the array.
template <class T>
void Array<T>::resize(const int newCapacity)
{
	ASSERT(mResizable, "Can't resize an array that is locked!");
	ASSERT(mMemoryOwned, "Can't resize an array that doesn't own its own memory!");
	
	if (mMemoryOwned && mResizable && newCapacity != getCapacity())
	{
		if (newCapacity == 0)
		{
			// Resizing to 0 causes all internal storage to be freed. 
			if (mArray)
			{
				// Manually destruct everything.
				for (int x = 0; x < mCount; x++)
				{
					destruct(x);
				}
				// Cast back to the original array type so that delete [] works correctly.
				char* oldSpace = reinterpret_cast<char*>(mArray);
				delete [] oldSpace;
				mArray = NULL;
			}
			
			mCount = 0;
			mCapacity = 0;
		}
		else
		{
			// Grow or shrink the internal storage buffer.
			// Allocate a new buffer, copy relevant items, and swap.
			// Internal storage must be allocated as a basic type so that we can delete it later
			// without invoking destructors.
			char* newSpace = new char[(sizeof(T) / sizeof(char)) * newCapacity];
			T* newArray = newArray = reinterpret_cast<T*>(newSpace);
			ASSERT((int)newArray == (int)newSpace, "Array addresses differ!");

			if (newSpace == NULL)
			{
				ASSERT(false, "Array allocation failed!");
			}
			else if (mArray == NULL)
			{
				// There is no existing buffer.	Construct the buffer elements in place.
				mArray = newArray;
				mCapacity = newCapacity;
				mCount = 0;
			}
			else
			{
				// Copy relevant items into the new space.	Memcpy will move the memory without invoking
				// user object code.
				if (mCapacity > newCapacity)
				{
					// Shrinking, so no consruction is necessary
					::memcpy(newArray, mArray, sizeof(T) * newCapacity);
					if (mCount > newCapacity)
					{
						// Instead, destruct all of the constructed elements that just got cut off the end of the array.
						for (int x = newCapacity; x < mCount; x++)
						{
							destruct(x);
						}
						mCount = newCapacity;
					}
				}
				else
				{
					// Growing, need to construct only the newly added elements.
					// Existing elements have already been constructed.
					::memcpy(newArray, mArray, sizeof(T) * mCapacity);
				}
				
				// Now for the tricky part.	We need to deallocate mArray without calling destructors on
				// the items within it.	Cast it back to its original type for destruction.	
				char* oldSpace = reinterpret_cast<char*>(mArray);
				delete [] oldSpace;
				
				mArray = newArray;
				mCapacity = newCapacity;
			}
		}
	}
}

template <class T>
void Array<T>::construct(const int index)
{
	new (mArray + index) T;
}

template <class T>
void Array<T>::destruct(const int index)
{
	mArray[index].~T();
}

template <class T>
size_t Array<T>::getElementSize() const
{
	return sizeof(T);
}

template <class T>
void* Array<T>::getElement(const int index) const
{
	ASSERT(mArray != NULL, "getElement() called on array with no storage!");
	ASSERT(index < mCount, "getElement() called with invalid index!");

	void* element = static_cast<void*>(mArray + index);
	return element;
}

template <class T>
void Array<T>::appendElement(void* data)
{
	T* element = static_cast<T*>(data);
	append(*element);
}

template <class T>
void Array<T>::setElement(const int index, void* data)
{
	ASSERT(mArray != NULL, "setElement() called on array with no storage!");
	ASSERT(index < mCount, "setElement() called with invalid index!");

	T* element = static_cast<T*>(data);
	set(index, *element);
}

template <class T>
void* Array<T>::getMemory()
{
	return &mArray;
}

template <class T>
void Array<T>::setMemory(void* address, int count)
{
	mArray = static_cast<T*>(address);
	mCount = count;
	if (mArray)
	{
		mResizable = false;
		mMemoryOwned = false;
	}
}

#endif //_ARRAY_H