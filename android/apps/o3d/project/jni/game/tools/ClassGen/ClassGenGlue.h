//	ClassGenGlue.h
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

#ifndef CLASSGEN_GLUE_H
#define CLASSGEN_GLUE_H

#include "ParseElement.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int RunClassGen(const ParseElement* pRoot, const char* pOutputFile, int quietMode);

#ifdef __cplusplus
}
#endif

#endif // CLASSGEN_GLUE_H
