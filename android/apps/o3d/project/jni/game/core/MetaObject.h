//	MetaObject.h
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


#ifndef _METAOBJECT_H
#define _METAOBJECT_H

#include "MetaField.h"
#include "Object.h"

#include "my_assert.h"

// If METADATA_STATIC_REGISTRATION is defined, MetaObjects will automatically be registered with the
// meta registry on app startup.	This will probably prevent compilers from stripping out code from unused
// meta-laden objects.	If METADATA_STATIC_REGISTRATION is undefined, each used meta object will require 
// explicit registration.
#define METADATA_STATIC_REGISTRATION

typedef void* (*ConstructionCallbackFunctionPtr)(void*, bool) ;

class MetaObject : public Object
{
	public:
		MetaObject(	const char* pName, 
					const unsigned int typeID, 
					const unsigned int baseClassTypeID,
					const size_t size, 
					const int fieldCount, 
					const MetaField** pFields, 
					const MetaObject* pParentMeta, 
					ConstructionCallbackFunctionPtr pConstructionCallbackFunction)
			:	mpName(pName), 
				mTypeID(typeID), 
				mBaseTypeID(baseClassTypeID), 
				mSize(size), 
				mFieldCount(fieldCount), 
				mpFields(pFields), 
				mpParentMeta(pParentMeta), 
				mpConstructionFunction(pConstructionCallbackFunction) ,
				mTotalFieldCount(mFieldCount + (pParentMeta != NULL ? pParentMeta->getFieldCount() : 0))
			{
			
			};
		
		const char* getName() const;
		const unsigned int getTypeID() const;
		const size_t getSize() const;
		const MetaField* getField(const int index) const;
		const int getFieldCount() const;
		const MetaObject* getParentMetaObject() const;
		
		bool isOfType(const MetaObject* pType) const;
		
		void* allocateRaw(void* pAddress = 0) const;
		void* allocateAndConstruct(void* pAddress = 0) const;
		
		static unsigned int generateTypeIDFromString(const char* pTypeName);
		
		
	private:
		const char* mpName;
		const unsigned int mTypeID;
		const unsigned int mBaseTypeID;
		const size_t mSize;
		const int mFieldCount;
		const MetaField** mpFields;
		const MetaObject* mpParentMeta;
		const ConstructionCallbackFunctionPtr mpConstructionFunction;
		const int mTotalFieldCount;
	
};

inline const char* MetaObject::getName() const
{
	return mpName;
}

inline const unsigned int MetaObject::getTypeID() const
{
	return mTypeID;
}

inline const size_t MetaObject::getSize() const
{
	return mSize;
}
	
inline const int MetaObject::getFieldCount() const
{	
	return mTotalFieldCount;
}

inline const MetaObject* MetaObject::getParentMetaObject() const
{
	return mpParentMeta;
}

template<typename T>
T* constructByType()
{
	T::registerMetaData();
	const MetaObject* pMeta = T::getMetaObject();
	
	T* pObject = static_cast<T*>(pMeta->allocateAndConstruct());
	
	return pObject;
}


#endif //_METAOBJECT_H