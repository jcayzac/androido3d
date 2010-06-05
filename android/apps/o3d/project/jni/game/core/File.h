//	File.h
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


#ifndef _FILE_H
#define _FILE_H

#include "Object.h"

#include <stdio.h>

class File : public Object
{
	public:
				
		static FILE* open(const char* pFileName, const char* mode);
		static void close(FILE* pFile);
		
		static int getFileLength(FILE* pFile);
		static void rawRead(const char* pFileName, char** pDestBuffer, int& destBufferLength);
		static char* readFile(FILE* pFile, int length, int startPosition = 0);
		static bool writeFile(FILE* pFile, char* buffer, int bufferSize);
};



#endif //_FILE_H