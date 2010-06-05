//	assert.h
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


#ifndef _ASSERT_H
#define _ASSERT_H

#if defined(DEBUG)
#define ASSERT(condition, message) if (!(condition)) { custom_assert(__FILE__, __LINE__, message); }
#else
#define ASSERT(condition, message)
#endif

void custom_assert(const char* pFileString, const int line, const char* pMessageString);

#endif