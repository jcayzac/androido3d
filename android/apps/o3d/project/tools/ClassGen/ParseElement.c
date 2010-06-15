/*
//	ParseElement.c
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

#include "ParseElement.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void yyerror(const char* s);

ParseElement* GetRootElement()
{
	static ParseElement root;
	root.mType = ELEMENT_root;
	
	return &root;
}

ParseElement* NewElement(ElementType type, const char* string, int line)
{
	ParseElement* element = (ParseElement*)malloc(sizeof(ParseElement));
	if (element == NULL)
	{
		printf("Out of memory!\n");
		exit(1);
	}
	
	memset(&element, sizeof(ParseElement), 0);
	
	element->mpParent = NULL;
	element->mpChildren = NULL;
	element->mpNext = NULL;
	element->mpString = (char*)strdup(string);
	element->mType = type;
	element->mFlags = 0;
	element->mpValue = NULL;
	element->mSourceFileLine = line;
	
	return element;
}

void UpdateElement(ElementType type, ParseElement* pElement, const char* string)
{
	if (pElement && pElement->mType == type)
	{
		if (string)
		{
			pElement->mpString = (char*)strdup(string);
		}
		else
		{
			pElement->mpString = NULL;
		}
	}
	else
	{
		yyerror("Attempt to update invalid element!");
	}
}

void AppendChild(ParseElement* pParent, ParseElement* pElement)
{
	ParseElement* pChild = NULL;
	ParseElement* pLastChild = NULL;
	ParseElement* pNode = NULL;

	if (pParent == NULL)
	{
		pParent = GetRootElement();
	}
	
	pChild = pParent->mpChildren;
	pLastChild = pParent->mpChildren;
	
	while (pChild != NULL)
	{
		pLastChild = pChild;
		pChild = pChild->mpNext;
	}
	
	if (pLastChild != NULL)
	{
		pLastChild->mpNext = pElement;
	}
	else
	{
		pParent->mpChildren = pElement;
	}
	
	// we might have just appended a tree of children, so make sure they all point to the correct parent
	for (pNode = pElement; pNode != NULL; pNode = pNode->mpNext)
	{
		pNode->mpParent = pParent;
	}
	
}

void AppendSibling(ParseElement* pElement, ParseElement* pSibling)
{
	if (pElement == NULL || pSibling == NULL)
	{
		yyerror("Null elements found during parsing!\n");
	}
	else if (pSibling->mpNext != NULL)
	{
		yyerror("Can't merge two sets of siblings!\n");
	}
	else
	{
		ParseElement* pChild = pElement;
		ParseElement* pLastChild = pElement;

		while (pChild != NULL)
		{
			pLastChild = pChild;
			pChild = pChild->mpNext;
		}
		
		pLastChild->mpNext = pSibling;
	}
}

void SetType(ParseElement* pElement, ElementType type)
{
	pElement->mType = type;
}

void SetConst(ParseElement* pElement)
{
	pElement->mFlags |= FLAG_const;
}

void SetSmartPointer(ParseElement* pElement)
{
	pElement->mFlags |= FLAG_smart;
}

void SetScope(ParseElement* pElement, ScopeType scope)
{
	pElement->mScope = scope;
}

void SetArgumentType(ParseElement* pElement, ArgumentType argument)
{
	pElement->mArgument = argument;
}

void SetValue(ParseElement* pElement, const char* string)
{
	pElement->mpValue = (char*)strdup(string);
}

void PrintNode(ParseElement* pElement, int indent)
{
	char* pElementString = "";
	int x = 0;
	ParseElement* pChild = NULL;
	
	switch (pElement->mType)
	{
		case ELEMENT_root:
			pElementString = "root";
			break;
		case ELEMENT_metaClass:
			pElementString = "metaclass";
			break;
		case ELEMENT_base:
			pElementString = "base";
			break;
		case ELEMENT_get:
			pElementString = "get";
			break;
		case ELEMENT_set:
			pElementString = "set";
			break;
		case ELEMENT_function:
			pElementString = "function";
			break;
		case ELEMENT_field:
			pElementString = "field";
			break;
		case ELEMENT_type:
			pElementString = "type";
			break;
		case ELEMENT_value:
			pElementString = "value";
			break;
		case ELEMENT_scope:
			pElementString = "scope";
			break;
		case ELEMENT_argumentType:
			pElementString = "argumentType";
			break;
		case ELEMENT_cruft:
			pElementString = "cruft";
			break;
		case ELEMENT_abstract:
			pElementString = "abstract";
			break;
		case ELEMENT_length:
			pElementString = "length";
			break;
		case ELEMENT_construct:
			pElementString = "construct";
			break;
		case ELEMENT_comment:
			pElementString = "comment";
			break;
	}
	
	for (x = 0; x < indent; x++)
	{
		printf("\t");
	}
	
	printf("%s: %s\n", pElementString, pElement->mpString);
	
	pChild = pElement->mpChildren;
	for (; pChild != NULL; pChild = pChild->mpNext)
	{
		PrintNode(pChild, indent + 1);
	}
	
}
