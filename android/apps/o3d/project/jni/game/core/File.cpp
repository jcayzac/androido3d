//	File.cpp
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


#include "File.h"

#include "my_assert.h"


void File::rawRead(const char* pFileName, char** pDestBuffer, int& destBufferLength)
{
	FILE* pFile = open(pFileName, "rb");
	int length = getFileLength(pFile);
	char* pBuffer = readFile(pFile, length);
	close(pFile);
	
	*pDestBuffer = pBuffer;
	destBufferLength = length;
}

FILE* File::open(const char* pFileName, const char* mode)
{
	return fopen(pFileName, mode);
}

void File::close(FILE* pFile)
{
	if (pFile)
	{
		fclose(pFile);
	}
}

int File::getFileLength(FILE* pFile)
{
	int length = 0;

	if (pFile)
	{
		fseek(pFile, 0, SEEK_SET);
		int start = ftell(pFile);
		fseek(pFile, 0, SEEK_END);
		int end = ftell(pFile);
		
		length = end - start;
		rewind(pFile);
	}
	
	return length;
}


char* File::readFile(FILE* pFile, int length, int startPosition)
{
	char* buffer = NULL;

	if (pFile)
	{
		if (length > 0)
		{
			buffer = new char[length + 2];
			ASSERT(buffer, "Failed to allocate buffer for file read!");
			
			if (buffer)
			{
				fseek(pFile, startPosition, SEEK_SET);
				fread(buffer, 1, length + 1, pFile);
				
				buffer[length + 1] = 0;
			}
		}
		
	}
	
	return buffer;
}


bool File::writeFile(FILE* pFile, char* buffer, int bufferSize)
{
	bool wroteOk = false;
	
	if (pFile && buffer && bufferSize > 0)
	{
		fwrite(buffer, bufferSize, 1, pFile);
		fflush(pFile);
		wroteOk = true;
	}
	
	return wroteOk;
}
