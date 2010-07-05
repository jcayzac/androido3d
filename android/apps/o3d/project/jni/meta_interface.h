/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _META_INTERFACE_H
#define _META_INTERFACE_H

#include "Array.h"

#include <jni.h>

class MetaBase;
class MetaField;

class MetaInterface {
  public:
    MetaInterface(JNIEnv* env, jobjectArray path, const jsize element_count);
    ~MetaInterface();
    
    jobjectArray parsePath();
    
    static jobjectArray getSystems(JNIEnv* env);
     
  protected:
    jobjectArray parseObject(MetaBase const* root, const int current_index);
    jobjectArray printObject(MetaBase const* root);
    jobjectArray printArray(MetaBase const* root, const MetaField* field);

    bool isNumber(const char* string);
    void getStringValue(const MetaBase* object, const MetaField* field, const int index, char* buffer);


  private:
    JNIEnv* mEnv;
    Array<char const*> mElements;
    Array<jstring> mJStrings;
 
 
};

#endif //_META_INTERFACE_H