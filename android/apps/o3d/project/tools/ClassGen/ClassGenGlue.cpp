//  ClassGenGlue.cpp
//  Copyright (C) 2008 Chris Pruett.    c_pruett@efn.org
//
//  FarClip Engine
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.


#include "ClassGenGlue.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "ClassGenerator.h"

using namespace std;

int RunClassGen(const ParseElement* pRoot, const char* pOutputFile, int quietMode)
{
  int result = 0;
  ClassGenerator parser;
  string output = parser.parseTree(pRoot);

  if (parser.getErrorCount() == 0)
  {
    fstream outputFile(pOutputFile, ios::out | ios::trunc);

    outputFile << output;

    outputFile.close();

    if (!quietMode)
    {
      cout << output << endl;
    }

    result = 1;
  }

  return result;
}
