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
#include "base/cross/string_util.h"
#include <syslog.h>

namespace o3d {
namespace base {

inline static int translateLogLevel(LogLevel s) {
  switch(s) {
    default:
    case INFO:    return LOG_INFO;
    case WARNING: return LOG_WARNING;
    case ERROR:   return LOG_ERR;
    case FATAL:   return LOG_ALERT;
  }
}
FullLogger::~FullLogger() {
  openlog(mTag.c_str(), LOG_PID|LOG_PERROR, LOG_USER);
  std::vector<std::string> lines;
  SplitString(mStream.str(), '\n', &lines);
  for (size_t i(0); i<lines.size(); ++i)
    syslog(translateLogLevel(mLevel), lines[i].c_str());
  closelog();
  if (mLevel==FATAL) abort();
}

} // namespace base
} // namespace o3d
