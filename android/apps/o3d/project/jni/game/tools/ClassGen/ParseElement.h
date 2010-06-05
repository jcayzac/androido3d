/*
//	ParseElement.h
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
*/

#ifndef _PARSE_ELEMENT_H
#define _PARSE_ELEMENT_H

typedef enum	
{
	ELEMENT_root,
	ELEMENT_metaClass,
	ELEMENT_base,
	ELEMENT_get,
	ELEMENT_set,
	ELEMENT_function,
	ELEMENT_field,
	ELEMENT_type,
	ELEMENT_value,
	ELEMENT_scope,
	ELEMENT_argumentType,
	ELEMENT_cruft,
	ELEMENT_abstract,
	ELEMENT_length,
	ELEMENT_construct,
	ELEMENT_comment,
	ELEMENT_enum,
	ELEMENT_refCount
} ElementType;

typedef enum	
{
	SCOPE_invalid = 0,
	SCOPE_public,
	SCOPE_protected,
	SCOPE_private
} ScopeType;

typedef enum 
{
	ARGUMENT_invalid = 0,
	ARGUMENT_value,
	ARGUMENT_reference,
	ARGUMENT_pointer
} ArgumentType;

#ifndef __cplusplus
// oh C
typedef enum 
{ 
	false = 0, 
	true = 1 
} bool;

#endif

typedef enum
{
	FLAG_const = 1 << 0,
	FLAG_smart = 1 << 1
} ElementFlags;

typedef struct ParseElement
{	
	ElementType mType;
	ScopeType mScope;
	ArgumentType mArgument;
	
	char* mpString;
	/* As of this writing, enums are the only elements that have both a name and a value in the same element. */
	char* mpValue;
	unsigned int mFlags;
	int mSourceFileLine;
	
	struct ParseElement* mpParent;
	struct ParseElement* mpChildren;
	struct ParseElement* mpNext;
} ParseElement;

#ifdef __cplusplus
extern "C" {
#endif

extern ParseElement* NewElement(ElementType type, const char* string, int line);
extern void UpdateElement(ElementType type, ParseElement* pElement, const char* string);
extern void AppendChild(ParseElement* pParent, ParseElement* pElement);
extern void AppendSibling(ParseElement* pElement, ParseElement* pSibling);
extern void SetType(ParseElement* pElement, ElementType type);
extern void SetConst(ParseElement* pElement);
extern void SetSmartPointer(ParseElement* pElement);
extern void SetScope(ParseElement* pElement, ScopeType scope);
extern void SetArgumentType(ParseElement* pElement, ArgumentType argument);
extern void SetValue(ParseElement* pElement, const char* string);
extern ParseElement* GetRootElement();
extern void PrintNode(ParseElement* pElement, int indent);

#ifdef __cplusplus
}
#endif

#endif // _PARSE_ELEMENT_H
