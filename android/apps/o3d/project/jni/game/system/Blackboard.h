//	Blackboard.h
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


#ifndef _BLACKBOARD_H
#define _BLACKBOARD_H

#include "DataTable.h"
#include "Object.h"
#include "Vector3.h"

class Blackboard : public Object
{
	public:
		enum
		{
			DEFAULT_maxElements = 32,
		};
		
		Blackboard();
		Blackboard(int elementCount);
		~Blackboard() { flush(); };
		
		void insertInt(int value, const char* name);
		void insertFloat(float value, const char* name);
		void insertVector(Vector3 vector, const char* name);
		void insertString(const char* string, const char* name);

		
		int getInt(const char* name) const;
		float getFloat(const char* name) const;
		Vector3 getVector(const char* name) const;
		char const* getString(const char* name) const;
		
		bool isInt(const char* name) const;
		bool isFloat(const char* name) const;
		bool isVector(const char* name) const;
		bool isString(const char* name) const;
		
		bool exists(const char* name) const;
		void remove(const char* name);
		
		void flush();
		void dump() const;
	
	protected:
		struct BlackboardEntry
		{
			enum Type
			{
				TYPE_invalid,
				TYPE_float,
				TYPE_int,
				TYPE_vector,
				TYPE_string,
			};
			
			union Data
			{
				int mInt;
				float mFloat;
				float mVector[3];
				char const* mString;
			};
			
			BlackboardEntry() : mType(TYPE_invalid) {};
			
			Data mData;
			Type mType;
		};

		void insert(BlackboardEntry& entry, const char* name);
		BlackboardEntry* find(const char* name) const;

	private:
		DataTable<BlackboardEntry> mDataTable;
};

#endif // _BLACKBOARD