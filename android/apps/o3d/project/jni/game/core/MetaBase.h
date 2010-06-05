//	MetaBase.h
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


#ifndef _METABASE_H
#define _METABASE_H

#include "Object.h"

class MetaObject;

class MetaBase : public Object
{
	public:
		virtual const MetaObject* getMetaObject() const = 0;
		const MetaBase* getObjectMemory() const { return this; };
		
		bool isOfType(const MetaObject* pOtherMeta);
		bool isType(const MetaObject* pOtherMeta);
		
		// field-by-field copy of another object to this
		void copy(const MetaBase* pOther);
		
		// we will use the meta signature to identify anonymous
		// pointers as MetaBase objects.	
		// if authenticatePointer() returns true, it's safe to cast this
		// pointer as a MetaBase and query its meta object.
		// For this to work, getMetaSignature must remain non-virtual.
		
		MetaBase() : mMetaSignature(getStaticMetaSignature()) { };
		
		unsigned int getMetaSignature();
		static bool authenticatePointer(void* pPointer);
		
	private:
		static unsigned int getStaticMetaSignature();
		
		unsigned int mMetaSignature;
		
};

inline unsigned int MetaBase::getMetaSignature()
{
	return mMetaSignature;
}

inline unsigned int MetaBase::getStaticMetaSignature()
{
	static int signature = 0;
	
	return reinterpret_cast<unsigned int>(&signature);
}

// compare some memory at pPointer to a known, non-common value.
// We can tell if this is a MetaBase object or not by checking to
// see if the data at the mMetaSignature field matches the
// static meta signature global to all MetaBase objects.
inline bool MetaBase::authenticatePointer(void* pPointer)
{
	return reinterpret_cast<MetaBase*>(pPointer)->getMetaSignature() == getStaticMetaSignature();
}

#endif //_METABASE_H