/*
 * Copyright (C) 2010 Tonchidot Corporation.
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
#pragma once

#include "base/cross/config.h"
#include <sstream>
#include <algorithm>
#include <stdlib.h>

// Minimum log level
#if !defined(O3D_MINIMUM_LOG_LEVEL)
  #if !defined(NDEBUG)
    #define O3D_MINIMUM_LOG_LEVEL INFO
  #else
    #define O3D_MINIMUM_LOG_LEVEL WARNING
  #endif
#endif

// Log tag
#if !defined(O3D_LOG_TAG)
  #define O3D_LOG_TAG "O3D"
#endif


enum LogLevel {
  INFO,
  WARNING,
  ERROR,
  FATAL
};

namespace o3d {
namespace base {

class FullLogger {
 public:
  FullLogger(LogLevel level, const char* tag): mLevel(level), mTag(tag) { }
  // The destructor will send mStream.str() to the right logging facility.
  ~FullLogger();
  template<typename T> FullLogger& operator<<(const T& t) {
    // Actually record stuff
    mStream << t;
    return *this;
  }
 private:
  std::ostringstream mStream;
  LogLevel           mLevel;
  std::string        mTag;
};

class FakeLogger {
 public:
  FakeLogger(LogLevel level, const char*) { if (level==FATAL) abort(); }
  template<typename T> FakeLogger& operator<<(const T& t) {
    // Does nothing.
    return *this;
  }
};

// Define LoggerForLevel<N>::type as FullLogger if N>=O3D_MINIMUM_LOG_LEVEL,
// or as FakeLogger otherwise.
template<int N> struct LoggerForLevel0    { typedef FakeLogger type; };
template<>      struct LoggerForLevel0<1> { typedef FullLogger type; };
template<int N> struct LoggerForLevel {
  enum { ACTIVE = ((N<O3D_MINIMUM_LOG_LEVEL)?0:1) };
  typedef typename LoggerForLevel0<ACTIVE>::type type;
};

} // namespace base
} // namespace o3d



#define O3D_PRIV_STRING0(x) #x
#define O3D_PRIV_STRING(x) O3D_PRIV_STRING0(x)
#define O3D_PRIV_FUNCTION_SUFFIX "in function [" << __PRETTY_FUNCTION__ << "] "
#define O3D_PRIV_UVAR x ## __LINE__

#define O3D_LOG_NAKED(level)         o3d::base::LoggerForLevel<level>::type(level, O3D_LOG_TAG)
#define O3D_LOG(level)               O3D_LOG_NAKED(level) << "[" << __FILE__ << ":" << O3D_PRIV_STRING(__LINE__) << "] "
#define O3D_LOG_IF(level, condition) if (condition) O3D_LOG(level)
#define O3D_LOG_FIRST_N(level, n)    for(static int O3D_PRIV_UVAR (1+2*(n)); (O3D_PRIV_UVAR=std::max(O3D_PRIV_UVAR-1,0));) if (O3D_PRIV_UVAR&1) break; else O3D_LOG(level)

#ifndef NDEBUG
  #define O3D_ASSERT(condition)      O3D_LOG_IF(FATAL, !(condition)) << "Assertion failed [" << O3D_PRIV_STRING(condition) << "] " << O3D_PRIV_FUNCTION_SUFFIX
  #define O3D_NEVER_REACHED()        O3D_LOG(FATAL) << "Reached supposedly-dead code " << O3D_PRIV_FUNCTION_SUFFIX
  #define O3D_NOTIMPLEMENTED()       O3D_LOG(ERROR) << "NOT IMPLEMENTED " << O3D_PRIV_FUNCTION_SUFFIX
#else
  #define O3D_ASSERT(condition)      O3D_LOG_IF(FATAL, (false && (condition)))
  #define O3D_NEVER_REACHED()        O3D_LOG_IF(FATAL, false)
  #define O3D_NOTIMPLEMENTED()       O3D_LOG_IF(ERROR, false)
#endif
