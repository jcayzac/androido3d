//	ClassGenerator.h
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


#ifndef _CLASSGENERATOR_H
#define _CLASSGENERATOR_H

#include <list>
#include <string>
#include <sstream>

#include "ParseElement.h"

class ClassGenerator
{
	public:
		ClassGenerator() { };
		~ClassGenerator() { };
	
		std::string parseTree(const ParseElement* pRoot);
		
		int getErrorCount();
		
	protected:
		void parseNode(const ParseElement* pElement);
		void parseDocument(const ParseElement* pElement);
		void parseElement(const ParseElement* pElement);
		void parseChildObjects(const ParseElement* pElement);
		void parseChildAttributes(const ParseElement* pElement);
		
		void createClass(const ParseElement* pElement);
		void createField(const ParseElement* pElement);
		void createAccessor(const ParseElement* pElement, const bool generateGet, const bool generateSet);
		void createFunction(const ParseElement* pElement);
		void createEnum(const ParseElement* pElement);

		void createInlineFunctions(const std::string& pClassName, const std::string& pClassBase, const std::string& pScopeString, const bool isAbstract);
		
		
		// utility
		std::string lowerCase(const std::string& input) const;
		std::string baseName(const std::string& input) const;
		std::string indentWhitespace(int indentOffset = 0) const;
		
		std::string getChildString(const ParseElement* pElement, const ElementType type) const;
		const ParseElement* findChildByString(const ParseElement* pElement, const ElementType type, const std::string& name) const;
		const ParseElement* findChild(const ParseElement* pElement, const ElementType type) const;
		
		bool isPointerType(const std::string& typeName, unsigned int* position);
		
		void printScope(ScopeType scope);
		void resetScope();
		
		void error(const char* message, int line);
		void warning(const char* message, int line);

	private:
		std::stringstream mOutputString;
		std::stringstream mInlineFunctionString;
		std::stringstream mMetaFieldDefinitionString;
		std::stringstream mMetaFieldArrayString;
		std::stringstream mFactoryFieldConstructionString;
		std::stringstream mAccessorFunctionString;
		
		int mFieldCount;
		int mIndent;
		ScopeType mCurrentScope;
		std::list<char*> mClassNameList;
		std::list<char*> mChildClassNameList;
		std::string mCurrentFileName;
		int mErrorCount;
		bool mRequiresStringMetaField;
		bool mRequiresObjectHandleMetaField;
		bool mRequiresDynamicArrayMetaField;
		
		
};

inline int ClassGenerator::getErrorCount()
{
	return mErrorCount;
}

#endif