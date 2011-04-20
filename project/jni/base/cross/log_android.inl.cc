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
#include <android/log.h>

namespace o3d {
namespace base {

inline static int translateLogLevel(LogLevel s) {
  switch(s) {
    default:
    case INFO:    return ANDROID_LOG_INFO;
    case WARNING: return ANDROID_LOG_WARN;
    case ERROR:   return ANDROID_LOG_ERROR;
    case FATAL:   return ANDROID_LOG_FATAL;
  }
}

FullLogger::~FullLogger() {
  // Sometimes, __android_log_assert() traps the process *before* the message
  // gets sent to the log, so I don't use it.
  __android_log_write(translateLogLevel(mLevel), mTag.c_str(), mStream.str().c_str());
  if (mLevel==FATAL) abort();
}

} // namespace base
} // namespace o3d
