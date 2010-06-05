%{
/*
//	classgen_parser.y
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ClassGenGlue.h"
#include "ParseElement.h"

/* we found the following required for some yacc implementations. */
/* #define YYSTYPE int */

#define YYDEBUG 1				// Generate debug code; needed for YYERROR_VERBOSE
#define YYERROR_VERBOSE // Give a more specific parse error message 

int linecount = 0;
void yyerror(const char* s);

%}

%union 
{
	ParseElement* element;
	char* string;
}

%token <element> T_METACLASS 
%token T_DYNAMIC
%token <string> T_STRING 
%token <element> T_BASE 
%token <element> T_GET 
%token <element> T_SET 
%token <element> T_FUNCTION 
%token <element> T_FIELD
%token <element> T_TYPE 
%token <element> T_VALUE 
%token <element> T_PUBLIC 
%token <element> T_PROTECTED 
%token <element> T_PRIVATE
%token <element> T_REFERENCE
%token <element> T_POINTER 
%token <element> T_CRUFT
%token <element> T_ABSTRACT 
%token <element> T_LENGTH
%token <element> T_CONSTRUCT
%token <element> T_COMMENT
%token <element> T_ENUM
%token <element> T_REFCOUNT
%token T_BY T_CONST T_OPEN_PAREN T_CLOSE_PAREN T_COMMA T_SMART

%type <element> Abstract
%type <element> ArgumentOptions
%type <element> ArgumentType
%type <element> Base
%type <element> ClassAttribute
%type <element> ClassAttributeList
%type <element> Comment
%type <element> Construct
%type <element> Cruft
%type <element> Enum
%type <element> EnumAttribute
%type <element> EnumAttributeList
%type <element> EnumValue
%type <element> Field
%type <element> FieldAttribute
%type <element> FieldAttributeList
%type <element> Function
%type <element> FunctionAttribute
%type <element> FunctionAttributeList
%type <element> Get
%type <element> GetAttribute
%type <element> GetAttributeList
%type <element> Length
%type <element> MetaClass
%type <element> RefCount
%type <element> Scope
%type <element> Set
%type <element> SetAttribute
%type <element> SetAttributeList
%type <element> Type
%type <element> Value


%%

Document: Document MetaClass { AppendChild(GetRootElement(), $2); }
	| Document Cruft { AppendChild(GetRootElement(), $2); }
	| MetaClass { AppendChild(GetRootElement(), $1); }
	| Cruft { AppendChild(GetRootElement(), $1); };
	
Cruft: Cruft T_CRUFT { AppendSibling($1, $2); $$ = $1; }
	| T_CRUFT { $$ = $1; };
	
MetaClass: T_METACLASS T_STRING T_OPEN_PAREN ClassAttributeList T_CLOSE_PAREN	 
	{ 
		UpdateElement(ELEMENT_metaClass, $1, $2);
		AppendChild($1, $4);
		$$ = $1;
		printf("Parsed metaclass!\n"); 
	} ; 

ClassAttributeList: ClassAttributeList ClassAttribute { AppendSibling($1, $2); $$ = $1; } 
	| ClassAttribute { $$ = $1; } ;
					
ClassAttribute: Base | Abstract | Function | Field | Get | Set | Enum | Comment | Scope | MetaClass ;

Base: T_BASE T_STRING { UpdateElement(ELEMENT_base, $1, $2);	$$ = $1; } ;

Abstract: T_ABSTRACT { $$ = $1; } ;

Function: T_FUNCTION T_STRING T_OPEN_PAREN FunctionAttributeList T_CLOSE_PAREN	
	{ 
		UpdateElement(ELEMENT_function, $1, $2);	
		AppendChild($1, $4); 
		$$ = $1; 
	} ;

Field: T_FIELD T_STRING T_OPEN_PAREN FieldAttributeList T_CLOSE_PAREN 
	{
		UpdateElement(ELEMENT_field, $1, $2); /* set the field name */
		AppendChild($1, $4); 
		$$ = $1;
	} ;

FunctionAttributeList: FunctionAttributeList T_COMMA FunctionAttribute { AppendSibling($1, $3); $$ = $1; }
	| FunctionAttribute { $$ = $1; } ;

FunctionAttribute: Scope /* |	empty */ ;

FieldAttributeList: FieldAttributeList T_COMMA FieldAttribute { AppendSibling($1, $3); $$ = $1; }
	| FieldAttribute { $$ = $1; } ;

FieldAttribute: Type | Value | Scope | Length | Construct | RefCount;

Scope: T_PUBLIC		{ SetScope($1, SCOPE_public); $$ = $1; }
	| T_PROTECTED	{ SetScope($1, SCOPE_protected); $$ = $1; }
	| T_PRIVATE		{ SetScope($1, SCOPE_private); $$ = $1; };

Type: T_TYPE T_STRING { UpdateElement(ELEMENT_type, $1, $2); $$ = $1; } ;

Value: T_VALUE T_STRING { UpdateElement(ELEMENT_value, $1, $2); $$ = $1; } ;
	
Length: T_LENGTH T_DYNAMIC { UpdateElement(ELEMENT_length, $1, "-1"); $$ = $1; }
	| T_LENGTH T_STRING { UpdateElement(ELEMENT_length, $1, $2); $$ = $1; } ;

Construct: T_CONSTRUCT { $$ = $1; }
	| T_CONSTRUCT T_SMART { SetSmartPointer($1); $$ = $1; };

RefCount: T_REFCOUNT { $$ = $1; };

Get: T_GET T_STRING { UpdateElement(ELEMENT_get, $1, $2); $$ = $1; } 
	| T_GET T_STRING T_OPEN_PAREN GetAttributeList T_CLOSE_PAREN 
	{ 
		UpdateElement(ELEMENT_get, $1, $2);	
		AppendChild($1, $4); 
		$$ = $1; 
	} ;

GetAttributeList: GetAttributeList T_COMMA GetAttribute { AppendSibling($1, $3); $$ = $1; }
	| GetAttribute { $$ = $1; } ;

GetAttribute: Scope | ArgumentType ;

ArgumentType: T_BY ArgumentOptions { $$ = $2; }
	| T_BY T_CONST ArgumentOptions { SetConst($3); $$ = $3; } ;

ArgumentOptions: T_VALUE 
		{ 
			// the "value" token can be used here or in the Value production
			// to mean two different things.	So here we'll change it from an
			// ELEMENT_value node to an ELEMENT_argumentType node.
			SetType($1, ELEMENT_argumentType); 
			SetArgumentType($1, ARGUMENT_value);
			$$ = $1; 
		}
	| T_REFERENCE { SetArgumentType($1, ARGUMENT_reference); $$ = $1; }
	| T_POINTER { SetArgumentType($1, ARGUMENT_pointer); $$ = $1; } ;

Set: T_SET T_STRING { UpdateElement(ELEMENT_set, $1, $2); $$ = $1; } 
	| T_SET T_STRING T_OPEN_PAREN SetAttributeList T_CLOSE_PAREN
	{ 
		UpdateElement(ELEMENT_set, $1, $2);	
		AppendChild($1, $4); 
		$$ = $1; 
	} ;

SetAttributeList: SetAttributeList T_COMMA SetAttribute { AppendSibling($1, $3); $$ = $1; }
	| SetAttribute { $$ = $1; } ;

SetAttribute: Scope | ArgumentType ;

Comment: T_COMMENT { $$ = $1; } ;

Enum: T_ENUM T_STRING T_OPEN_PAREN EnumAttributeList T_CLOSE_PAREN
	{ 
		UpdateElement(ELEMENT_enum, $1, $2);	
		AppendChild($1, $4); 
		$$ = $1; 
	} 
	| T_ENUM T_OPEN_PAREN EnumAttributeList T_CLOSE_PAREN
	{ 
		// anonymous enum.
		UpdateElement(ELEMENT_enum, $1, NULL);	
		AppendChild($1, $3); 
		$$ = $1; 
	} ;
	
EnumAttributeList: EnumAttributeList T_COMMA EnumAttribute { AppendSibling($1, $3); $$ = $1; }
	| EnumAttribute { $$ = $1; } ;

EnumAttribute: Scope | EnumValue | Comment ;

EnumValue: T_VALUE T_STRING T_STRING 
	{ 
		UpdateElement(ELEMENT_value, $1, $2); 
		SetValue($1, $3);
		$$ = $1; 
	}
	| T_VALUE T_STRING { UpdateElement(ELEMENT_value, $1, $2); $$ = $1; } ;

%%

extern char *yytext;
int errorCount = 0;
char* fileName = NULL;

int main (int argc, char * const argv[]) 
{
	int quietMode = 0;
	int printTree = 0;
	extern FILE *yyin;
	int returnCode = 0;

	if (argc < 3 || argc > 5)
	{
		fprintf(stderr, "USAGE: %s inputFile outputFile [-v] [-t] \n", argv[0]);	
		fprintf(stderr, "\t-v Verbose Mode. \n");
		fprintf(stderr, "\t-t Output Parse Tree. \n");			
		return 1;
	}
	
	fileName = argv[1];
	yyin = fopen(argv[1], "r");
	if(yyin == NULL) /* open failed */
	{
		fprintf(stderr,"%s: cannot open %s\n", argv[0], argv[1]);
		exit(1);
	}

	do 
	{
		yyparse();
	} while(!feof(yyin));
	
	fclose(yyin);
	
	/* Set the root string to be the filename */
	UpdateElement(ELEMENT_root, GetRootElement(), argv[1]);
	
	printTree = (argc > 3 && strcmp(argv[3], "-t") == 0) || (argc > 4 && strcmp(argv[4], "-t") == 0);
	quietMode = (argc > 3 && strcmp(argv[3], "-v") == 0) || (argc > 4 && strcmp(argv[4], "-v") == 0);
	
	if (printTree)
	{
		PrintNode(GetRootElement(), 0);
	}
	
	if (errorCount == 0)
	{
		if (RunClassGen(GetRootElement(), argv[2], !quietMode) == 0)
		{
			returnCode = 1;
		}
	}
	
	return returnCode;
}

void yyerror(const char* s)
{
	errorCount++;
		fprintf(stderr, "%s:%d: error: %s at '%s'\n", fileName, linecount, s, yytext);
}

// Flex requires us to define this ourselves
int yywrap()
{
	return 1;
}


