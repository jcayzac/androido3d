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

#include "meta_interface.h" //self
 
#include "Array.h"
#include "MetaObject.h"
#include "MetaField.h"
#include "MetaRegistry.h"
#include "System.h"
#include "SystemRegistry.h"
#include "Vector3.h"

#include "third_party/loggingshim/base/logging.h"

#include <jni.h>

MetaInterface::MetaInterface(JNIEnv* env, jobjectArray path, const jsize element_count) 
  : mEnv(env), mElements(element_count), mJStrings(element_count) {

  for (int x = 0; x < element_count; x++) {
    const jstring jni_string = (jstring)mEnv->GetObjectArrayElement(path, x);
    const char* c_string = mEnv->GetStringUTFChars(jni_string, NULL);
    mElements.append(c_string);
    mJStrings.append(jni_string);
  }
  
}

MetaInterface::~MetaInterface() {
  int path_elements = mJStrings.getCount();
  // release the strings.
  for (int x = 0; x < path_elements; x++) {
    mEnv->ReleaseStringUTFChars(mJStrings.get(x), mElements.get(x));
  }

  mElements.removeAll();
  mJStrings.removeAll();
}


jobjectArray MetaInterface::parsePath() {
  // first element is "get" or, eventually, "set".
  const char* system_string = mElements.get(1);
  DLOG(INFO) << "Looking for system: " << system_string;
  const MetaObject* system_meta = MetaRegistry::getMetaRegistry()->getMetaObject(system_string);
  const System* system = SystemRegistry::getSystemRegistry()->getSystem(system_meta);
  MetaBase const* root = system;

  jobjectArray output = NULL;
  if (system != NULL) {
    DLOG(INFO) << "Found system: " << system->getMetaObject()->getName();
    if (mElements.getCount() > 2) {
      output = parseObject(root, 2);
    } else {
      output = printObject(root);
    }
  }
  
  return output;
}

jobjectArray MetaInterface::parseObject(MetaBase const* root, const int current_index) {
  int next_index = current_index + 1;
  const int path_elements = mElements.getCount();
  jobjectArray result = NULL;
  
  
  const MetaObject* meta = root->getMetaObject();
  
  const char* field_string = mElements.get(current_index);
  const int field_count = meta->getFieldCount();
  for (int y = 0; y < field_count; y++) {
    const MetaField* metaField = meta->getField(y);

    if (strcmp(field_string, metaField->getName()) == 0) {
      DLOG(INFO) << "Found field: " << metaField->getName();
      // found the field!
  
      // test for array access
      int array_index = -1;
      if (current_index + 1 < path_elements) {
        const char* next_element = mElements.get(current_index + 1);
        if (isNumber(next_element)) {
          array_index = atoi(next_element);
          next_index++;
        }
      }

      // pull the object out of the root object
      void* object = NULL;
      bool is_array_terminator = false;
      
      if (array_index >= 0) {
        if (array_index >= 0 && array_index <= metaField->getElementCount(root)) {
          if (metaField->getStorageType() == MetaField::TYPE_pointer) {
            // array of pointers
            object = *((void**)metaField->getElement(root, array_index));
          } else {
            // inline array
            object = metaField->getElement(root, array_index);
          }
        }
      } else if (metaField->getElementCount(root) > 1 && next_index >= path_elements) {
        // This is an array request.
        is_array_terminator = true;
      } else {
        if (metaField->getStorageType() == MetaField::TYPE_pointer) {
          object = *((void**)metaField->get(root));
        } else {
          object = metaField->get(root);
        }
      }


      if (object && MetaBase::authenticatePointer(object)) {
        // safe to cast!
        MetaBase* new_root = static_cast<MetaBase*>(object);
        if (next_index < path_elements) {
          // recurse
          result = parseObject(new_root, next_index);
        } else {
          // leaf
          result = printObject(new_root);
        }
      } else if (is_array_terminator) {
        // This is the leaf, but it's an array.
        result = printArray(root, metaField);
      } else {
        // we found the field but can't go any further.
        DLOG(INFO) << "Field null or unknown type: " << metaField->getTypeName() << " (" << reinterpret_cast<int>(object) << ")";
      }
    }
  }
  return result;
}


jobjectArray MetaInterface::printObject(MetaBase const* root) {
  jobjectArray result = NULL;
  DLOG(INFO) << "Return object: " << root->getMetaObject()->getName();
  const MetaObject* meta = root->getMetaObject();
  
  if (meta && meta->getFieldCount() > 0) {
    const int field_count = meta->getFieldCount();
    DLOG(INFO) << "Field count is " << field_count;

    jobjectArray row = (jobjectArray)mEnv->NewObjectArray(3,
      mEnv->FindClass("java/lang/String"), NULL);
      
    result = (jobjectArray)mEnv->NewObjectArray(field_count,
      mEnv->GetObjectClass(row), NULL);

    for (int x = 0; x < field_count; x++) {
      row = (jobjectArray)mEnv->NewObjectArray(3,
        mEnv->FindClass("java/lang/String"), NULL);
      
      char buffer[255];
      getStringValue(root, meta->getField(x), 0, buffer);
      
      DLOG(INFO) << meta->getField(x)->getTypeName() << "  " << meta->getField(x)->getName() << "  " << buffer;
      
      mEnv->SetObjectArrayElement(row, 0, mEnv->NewStringUTF(meta->getField(x)->getName()));
      mEnv->SetObjectArrayElement(row, 1, mEnv->NewStringUTF(meta->getField(x)->getTypeName()));
      mEnv->SetObjectArrayElement(row, 2, mEnv->NewStringUTF(buffer));
      
      mEnv->SetObjectArrayElement(result, x, row);
    }
  }
  return result;
}

jobjectArray MetaInterface::printArray(MetaBase const* root, const MetaField* field) {
  jobjectArray result = NULL;
  DLOG(INFO) << "Return array " << field->getName() << "from " << root->getMetaObject()->getName();
  
  const int element_count = field->getElementCount(root);
  DLOG(INFO) << "Element count is " << element_count;

  jobjectArray row = (jobjectArray)mEnv->NewObjectArray(3,
    mEnv->FindClass("java/lang/String"), NULL);
    
  result = (jobjectArray)mEnv->NewObjectArray(element_count,
    mEnv->GetObjectClass(row), NULL);

  for (int x = 0; x < element_count; x++) {
    row = (jobjectArray)mEnv->NewObjectArray(3,
      mEnv->FindClass("java/lang/String"), NULL);
    
    char buffer[255];
    getStringValue(root, field, x, buffer);
    
    char name[32];
    sprintf(name, "%d", x);
    
    mEnv->SetObjectArrayElement(row, 0, mEnv->NewStringUTF(name));
    mEnv->SetObjectArrayElement(row, 1, mEnv->NewStringUTF(field->getTypeName()));
    mEnv->SetObjectArrayElement(row, 2, mEnv->NewStringUTF(buffer));
    
    mEnv->SetObjectArrayElement(result, x, row);
  }

  return result;
}


bool MetaInterface::isNumber(const char* string) {
  bool result = (string != NULL);
  char const* character = string;
  while (character != NULL && *character != 0) {
    if ((*character < '0' || *character > '9') && (*character != ' ' && *character != '\t')) {
      result = false;
      break;
    } else {
      character++;
    }
  }
  
  return result;
}

void MetaInterface::getStringValue(const MetaBase* object, const MetaField* field, 
    const int index, char* buffer) {
  buffer[0] = 0;
  if (field->getStorageType() == MetaField::TYPE_value) {
    if (strcmp(field->getTypeName(), "int") == 0) {
      sprintf(buffer, "%d", *(static_cast<int*>(field->getElement(object, index))));    
    } else if (strcmp(field->getTypeName(), "float") == 0) {
      sprintf(buffer, "%g", *(static_cast<float*>(field->getElement(object, index))));    
    } else if (strcmp(field->getTypeName(), "double") == 0) {
      sprintf(buffer, "%gL", *(static_cast<double*>(field->getElement(object, index))));
    } else if (strcmp(field->getTypeName(), "bool") == 0) {
      sprintf(buffer, "%s", *(static_cast<bool*>(field->getElement(object, index))) ?  "true" : "false");
    } else if (strcmp(field->getTypeName(), "Vector3") == 0) {
      const Vector3* vector = static_cast<Vector3*>(field->getElement(object, index));
      sprintf(buffer, "(%g, %g, %g)", (*vector)[0], (*vector)[1], (*vector)[2]);    
    } else {
      sprintf(buffer, "???");    
    }
  } else {
    void** pointer = static_cast<void**>(field->getElement(object, index));
    if (*pointer == NULL) {
      sprintf(buffer, "NULL");    
    } else {
      sprintf(buffer, "...");    
    }
  }
}


jobjectArray MetaInterface::getSystems(JNIEnv* env) {
  const int systemCount = SystemRegistry::getSystemRegistry()->getCount();

  jobjectArray ret = (jobjectArray)env->NewObjectArray(systemCount,
    env->FindClass("java/lang/String"), NULL);

  for (int x = 0; x < systemCount; x++) {
    const char* name = SystemRegistry::getSystemRegistry()->get(x)->getMetaObject()->getName();
    env->SetObjectArrayElement(ret, x, env->NewStringUTF(name));
  }

  return ret;
}