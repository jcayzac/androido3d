%{
/*
//	classgen_lexer.l
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


#include <string.h>

#include "ParseElement.h"

#include "classgen_parser.h"	/* token codes from the parser */

#define PRINT_TOKENS 0
#if (PRINT_TOKENS)
	#define T_PRINT(x) printf(x)
#else
	#define T_PRINT(x)
#endif

static int startup = 1;
static int braceLevel = 0;

extern int linecount;

char* trimString(char* input);
char* stripQuotes(char* input);

%}

%x CRUFT
%x FUNCTION
%x COMMENT

%%

%{
	/* Force the lexer to start in CRUFT state */
	if (startup == 1) 
	{
		BEGIN CRUFT;
		startup = 0;
	}
%}


<CRUFT>^metaclass/.*		{	T_PRINT("TOKEN cruft -> metaclass\n"); 
								yylval.element = NewElement(ELEMENT_metaClass, yytext, linecount);
								BEGIN INITIAL;
								return T_METACLASS; }
								
<CRUFT>.*					{	T_PRINT("TOKEN cruft\n");
								yylval.element = NewElement(ELEMENT_cruft, yytext, linecount); 
								return T_CRUFT; }

<CRUFT>[\n\r]				{	T_PRINT("TOKEN cruft: newline\n"); 
								yylval.element = NewElement(ELEMENT_cruft, "\n", linecount);
								linecount++;
								return T_CRUFT; }


<FUNCTION>[^"{"]*			{	T_PRINT("TOKEN string (function name)\n"); 
								yylval.string = trimString(yytext);
								BEGIN INITIAL;
								return T_STRING; }
								
<COMMENT>.*"*/"				{	T_PRINT("TOKEN comment (c-style, end)\n");
								yylval.element = NewElement(ELEMENT_comment, trimString(yytext), linecount); 
								BEGIN INITIAL;
								return T_COMMENT; }
								
<COMMENT>.*					{	T_PRINT("TOKEN comment (c-style, line)\n");
								yylval.element = NewElement(ELEMENT_comment, trimString(yytext), linecount); 
								return T_COMMENT; }
								
<COMMENT>[\n\r]				{	T_PRINT("TOKEN comment (c-style, newline)\n"); 
								/* eat newlines in comments */ 
								linecount++; }


[\t ]+						;
[\n\r]						{ linecount++; }

metaclass					{	T_PRINT("TOKEN metaclass\n"); 
								yylval.element = NewElement(ELEMENT_metaClass, yytext, linecount);
								return T_METACLASS; }
base						{	T_PRINT("TOKEN base\n");
								yylval.element = NewElement(ELEMENT_base, yytext, linecount);
								return T_BASE; }
abstract					{	T_PRINT("TOKEN abstract\n");
								yylval.element = NewElement(ELEMENT_abstract, yytext, linecount);
								return T_ABSTRACT; }
get							{	T_PRINT("TOKEN get\n");
								yylval.element = NewElement(ELEMENT_get, yytext, linecount);
								return T_GET; }
set							{	T_PRINT("TOKEN set\n");
								yylval.element = NewElement(ELEMENT_set, yytext, linecount);
								return T_SET; }
function					{	T_PRINT("TOKEN function\n"); 
								yylval.element = NewElement(ELEMENT_function, yytext, linecount);
								BEGIN FUNCTION;
								return T_FUNCTION; }
field						{	T_PRINT("TOKEN field\n"); 
								yylval.element = NewElement(ELEMENT_field, yytext, linecount);
								return T_FIELD; }
array						{	T_PRINT("TOKEN array\n"); 
								// array is just an alias for field
								yylval.element = NewElement(ELEMENT_field, yytext, linecount);
								return T_FIELD; }
by							{	T_PRINT("TOKEN by\n"); return T_BY; }
const						{	T_PRINT("TOKEN const\n"); return T_CONST; }
value						{	T_PRINT("TOKEN value\n"); 
								yylval.element = NewElement(ELEMENT_value, yytext, linecount);
								return T_VALUE; }
reference					{	T_PRINT("TOKEN reference\n"); 
								yylval.element = NewElement(ELEMENT_argumentType, yytext, linecount);
								return T_REFERENCE; }
pointer						{	T_PRINT("TOKEN pointer\n"); 
								yylval.element = NewElement(ELEMENT_argumentType, yytext, linecount);
								return T_POINTER; }
type						{	T_PRINT("TOKEN type\n");
								yylval.element = NewElement(ELEMENT_type, yytext, linecount);
								return T_TYPE; }
public						{	T_PRINT("TOKEN public\n"); 
								yylval.element = NewElement(ELEMENT_scope, yytext, linecount);
								return T_PUBLIC; }
protected					{	T_PRINT("TOKEN protected\n");
								yylval.element = NewElement(ELEMENT_scope, yytext, linecount);
								return T_PROTECTED; }
private						{	T_PRINT("TOKEN private\n"); 
								yylval.element = NewElement(ELEMENT_scope, yytext, linecount);
								return T_PRIVATE; }
length						{	T_PRINT("TOKEN length\n"); 
								yylval.element = NewElement(ELEMENT_length, yytext, linecount);
								return T_LENGTH; }
construct					{	T_PRINT("TOKEN construct\n"); 
								yylval.element = NewElement(ELEMENT_construct, yytext, linecount);
								return T_CONSTRUCT; }
enum						{	T_PRINT("TOKEN enum\n"); 
								yylval.element = NewElement(ELEMENT_enum, yytext, linecount);
								return T_ENUM; }
"ref count"					{	T_PRINT("TOKEN ref count\n"); 
								yylval.element = NewElement(ELEMENT_refCount, yytext, linecount);
								return T_REFCOUNT; }
smart						{	T_PRINT("TOKEN smart\n"); return T_SMART; }
dynamic						{	T_PRINT("TOKEN dynamic\n"); return T_DYNAMIC; }
[0-9\-][0-9.f]*				{	T_PRINT("TOKEN string (number)\n"); 
								yylval.string = (char*)strdup(yytext);
								return T_STRING; }
"{"							{	T_PRINT("TOKEN {\n"); braceLevel++; return T_OPEN_PAREN; }
"}"							{	T_PRINT("TOKEN }\n"); 
								braceLevel--; 
								if (braceLevel == 0) 
								{
									BEGIN CRUFT;
								}
								return T_CLOSE_PAREN; }
","							{	T_PRINT("TOKEN ,\n"); return T_COMMA; }

[a-zA-Z_][a-zA-Z0-9*<>._:]*	{	T_PRINT("TOKEN string\n"); 
								yylval.string = (char*)strdup(yytext);
								return T_STRING; }
\"[^"]*\"					{	T_PRINT("TOKEN string (quoted)\n"); 
								yylval.string = stripQuotes(yytext);
								return T_STRING; }
"//".*						{	T_PRINT("TOKEN comment\n");
								yylval.element = NewElement(ELEMENT_comment, yytext, linecount); 
								return T_COMMENT; }
								
"/*".*"*/"					{	T_PRINT("TOKEN comment (c-style, contained)\n");
								yylval.element = NewElement(ELEMENT_comment, trimString(yytext), linecount); 
								return T_COMMENT; }
								
"/*".*						{	T_PRINT("TOKEN comment (c-style, start)\n");
								yylval.element = NewElement(ELEMENT_comment, trimString(yytext), linecount);
								BEGIN COMMENT; 
								return T_COMMENT; }

.							{ yyerror("Unknown Token"); }; 

%%

/* Cuts the leading and trailing whitespace off of a string. */
char* trimString(char* input)
{
	char* newString = NULL;
	char* sourceString = input;
	char* endOfString;
	if (input)
	{
		newString = (char*)malloc(sizeof(char) * (strlen(input) + 1));
		
		/* Locate first non-whitespace character */
		while (sourceString[0] == ' ' || sourceString[0] == '\t' || sourceString[0] == '\r' || sourceString[0] == '\n')
		{
			sourceString++;
		}
		
		/* Copy */
		strcpy(newString, sourceString);
		
		/* Cut off trailing whitespace.	We don't care about a little wasted memory in this case. */
		endOfString = newString + strlen(newString) - 1;
		while (endOfString[0] == ' ' || endOfString[0] == '\t' || endOfString[0] == '\r' || endOfString[0] == '\n')
		{
			endOfString[0] = 0;
			endOfString--;
		}
	}
	
	return newString;
}

/* Cuts the leading and trailing quotes off of a string. */
char* stripQuotes(char* input)
{
	char* newString = NULL;
	char* sourceString = input;
	char* endOfString;
	if (input)
	{
		newString = (char*)malloc(sizeof(char) * (strlen(input) + 1));
		
		/* Skip leading quote */
		if (sourceString[0] == '\"')
		{
			sourceString++;
		}
		
		/* Copy */
		strcpy(newString, sourceString);
		
		/* Cut off trailing quote. */
		endOfString = newString + strlen(newString) - 1;
		if (endOfString[0] == '\"')
		{
			endOfString[0] = 0;
		}
	}
	
	return newString;
}

