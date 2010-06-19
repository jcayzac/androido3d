//	Blackboard.cpp
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


#include "Blackboard.h" // self

#include "my_assert.h"

Blackboard::Blackboard() 
	: mDataTable(true, DEFAULT_maxElements, false)
{

}

Blackboard::Blackboard(int elementCount) 
	: mDataTable(true, elementCount, false) 
{

};
		
void Blackboard::insertInt(int value, const char* name)
{
	BlackboardEntry pEntry;
	pEntry.mType = BlackboardEntry::TYPE_int;
	pEntry.mData.mInt = value;
	
	insert(pEntry, name);
}

void Blackboard::insertFloat(float value, const char* name)
{
	BlackboardEntry pEntry;
	pEntry.mType = BlackboardEntry::TYPE_float;
	pEntry.mData.mFloat = value;
	
	insert(pEntry, name);
}

void Blackboard::insertVector(Vector3 vector, const char* name)
{
	BlackboardEntry pEntry;
	pEntry.mType = BlackboardEntry::TYPE_vector;
	pEntry.mData.mVector[0] = vector[0];
	pEntry.mData.mVector[1] = vector[1];
	pEntry.mData.mVector[2] = vector[2];
	
	insert(pEntry, name);
}

void Blackboard::insertString(const char* string, const char* name)
{
	BlackboardEntry pEntry;
	pEntry.mType = BlackboardEntry::TYPE_string;
	pEntry.mData.mString = string;
	insert(pEntry, name);
}

int Blackboard::getInt(const char* name) const
{
	BlackboardEntry* entry = find(name);
	ASSERT(entry == NULL || (entry && entry->mType == BlackboardEntry::TYPE_int), ("Attempting to read non-int value as int from blackboard!"));
	
	int result = 0;
	if (entry)
	{
		result = entry->mData.mInt;
	}
	
	return result;
}

float Blackboard::getFloat(const char* name) const
{
	BlackboardEntry* entry = find(name);
	ASSERT(entry == NULL || (entry && entry->mType == BlackboardEntry::TYPE_float), ("Attempting to read non-float value as float from blackboard!"));
	
	float result = 0.0f;
	if (entry)
	{
		result = entry->mData.mFloat;
	}
	
	return result;
}

Vector3 Blackboard::getVector(const char* name) const
{
	BlackboardEntry* entry = find(name);
	ASSERT(entry == NULL || (entry && entry->mType == BlackboardEntry::TYPE_vector), ("Attempting to read non-vector value as vector from blackboard!"));
	
	Vector3 result = Vector3::ZERO;
	if (entry)
	{
		result = Vector3(entry->mData.mVector[0], entry->mData.mVector[1], entry->mData.mVector[2]);
	}
	
	return result;
}

char const* Blackboard::getString(const char* name) const
{
	BlackboardEntry* entry = find(name);
	ASSERT(entry == NULL || (entry && entry->mType == BlackboardEntry::TYPE_string), ("Attempting to read non-string value as string from blackboard!"));
	
	char const* string = NULL;
	if (entry)
	{
		string = entry->mData.mString;
	}
	
	return string;
}


bool Blackboard::isInt(const char* name) const
{
	BlackboardEntry* entry = find(name);
	return (entry && entry->mType == BlackboardEntry::TYPE_int);
}

bool Blackboard::isFloat(const char* name) const
{
	BlackboardEntry* entry = find(name);
	return (entry && entry->mType == BlackboardEntry::TYPE_float);
}

bool Blackboard::isVector(const char* name) const
{
	BlackboardEntry* entry = find(name);
	return (entry && entry->mType == BlackboardEntry::TYPE_vector);
}

bool Blackboard::isString(const char* name) const
{
	BlackboardEntry* entry = find(name);
	return (entry && entry->mType == BlackboardEntry::TYPE_string);
}

bool Blackboard::exists(const char* name) const
{
	BlackboardEntry* entry = find(name);
	return (entry != NULL);
}

void Blackboard::remove(const char* name)
{
	const int index = mDataTable.findIndex(name);
	if (index > -1)
	{
		mDataTable.removeIndex(index);
	}
}

void Blackboard::flush()
{
	mDataTable.removeAll();
}

void Blackboard::dump() const
{
	int entryCount = mDataTable.getElementCount();
	
	printf("Blackboard Output\n");
	printf("-------------------------------------------------------------------------");
	printf("Index\t\tValue\t\tName");
	printf("-------------------------------------------------------------------------");
	for (int x = 0; x < entryCount; x++)
	{
		BlackboardEntry entry = mDataTable.get(x);
		
		printf("%d\t\t", x);
		
		switch(entry.mType)
		{
			case BlackboardEntry::TYPE_int:
				printf("%d", entry.mData.mInt);
				break;
			case BlackboardEntry::TYPE_float:
				printf("%g", entry.mData.mFloat);
				break;
			case BlackboardEntry::TYPE_vector:
				printf("(%g, %g, %g)", entry.mData.mVector[0], entry.mData.mVector[1], entry.mData.mVector[2]);
				break;
			case BlackboardEntry::TYPE_string:
				printf("%s", entry.mData.mString);
				break;
			default:
				printf("** UNKNOWN ** (%d)", static_cast<int>(entry.mType));
				break;
		}
		
		printf("\t\t%s", mDataTable.getKey(x));
	}
}


void Blackboard::insert(Blackboard::BlackboardEntry& entry, const char* name)
{
	// insert a new entry only if one already doesn't exist
	const int index = mDataTable.findIndex(name);
	if (index >= 0)
	{
		mDataTable.set(index, entry);
	}
	else
	{
		mDataTable.add(name, entry);
	}
}

Blackboard::BlackboardEntry* Blackboard::find(const char* name) const
{
	const int index = mDataTable.findIndex(name);
	BlackboardEntry* result = NULL;
	if (index > -1)
	{
		result = mDataTable.getEntry(index);
	}
	
	return result;
}
