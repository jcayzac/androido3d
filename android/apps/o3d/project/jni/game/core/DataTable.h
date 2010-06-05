//	DataTable.h
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


#ifndef DATATABLE_H
#define DATATABLE_H

#include <assert.h>
#include <string.h>

#include "KeyValueMap.h"

typedef char const* StringType;

template <typename T>
class DataTable : public KeyValueMap<StringType, T>
{
	public:
		typedef KeyValueMap<StringType, T> StringValueMap;

		DataTable(bool keepSorted, int maxElements, bool caseSensitive);
		
		bool getCaseSensitive() const;
	
	protected:
		typedef KeyMapElementEntry<StringType, T> ElementEntry;
		
		virtual int compare(const char* str1, const char* str2) const;
		virtual void constructElement(ElementEntry* pEntry);	// Called after element is added.
		virtual void destroyElement(ElementEntry* pEntry);		// Called before element is destroyed.
	
				
	private:
		bool mCaseSensitive;

};

template <typename T>
inline bool DataTable<T>::getCaseSensitive() const
{
	return mCaseSensitive;
}

template <typename T>
inline DataTable<T>::DataTable(bool keepSorted, int maxElements, bool caseSensitive)
:	StringValueMap(keepSorted, maxElements),
	mCaseSensitive(caseSensitive)
{
}

template <typename T>
int DataTable<T>::compare(const char* str1, const char* str2) const
{
	int compareResult = 0;
	
	if (mCaseSensitive)
	{
		compareResult = ::strcmp(str1, str2);
	}
	else
	{
		#if defined(WIN32)
			compareResult = ::stricmp(str1, str2);
		#else
			compareResult = ::strcasecmp(str1, str2);
		#endif
	}
	
	return compareResult;
}

template <typename T>
void DataTable<T>::constructElement(ElementEntry* pEntry)
{
	if (pEntry)
	{
		char* keyString = new char[strlen(pEntry->mKey) + 1];
		strcpy(keyString, pEntry->mKey);
		pEntry->mKey = keyString;
	}
	
}

template <typename T>
void DataTable<T>::destroyElement(ElementEntry* pEntry)
{
	if (pEntry)
	{
		delete pEntry->mKey;
		pEntry->mKey = NULL;
	}
}

#endif // DATATABLE_H