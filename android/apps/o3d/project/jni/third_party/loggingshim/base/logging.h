// Simple logging shim to avoid linking in the mess that is Chromium

#ifndef LOGGING_SHIM_H_
#define LOGGING_SHIM_H_

#include <errno.h>

#include <algorithm>
#include <cstring>
#include <iostream>
#include <string>

#include <android/log.h>

enum LogSeverity {
    INFO = ANDROID_LOG_INFO,
    WARNING = ANDROID_LOG_WARN,
    ERROR = ANDROID_LOG_ERROR,
    FATAL = ANDROID_LOG_FATAL,
};

// This seems very wrong.
inline std::ostream& operator<<(std::ostream& stream, const std::wstring& s) {
  return stream << std::string(s.begin(), s.end());
}

namespace logging {

  class Logger {
    public: 
      Logger printLog(LogSeverity severity, const char* tag);
      std::ostream& operator<< (const std::string& s);
   private:
    LogSeverity mSeverity;
    char const* mTag;
  };
  
  Logger& getLogger();
  
  std::ostream& nullStream();
};

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define LOG(severity) \
  logging::getLogger().printLog(severity, __FILE__ ":" TOSTRING(__LINE__))

#define LOG_IF(severity, condition) \
  !(condition) ? logging::nullStream() : LOG(severity)
  
#define LOG_ASSERT(condition)  \
  LOG_IF(FATAL, !(condition)) << "Assert failed: " #condition ". "
  
#define CHECK(condition) \
  LOG_IF(FATAL, !(condition)) << "Check failed: " #condition ". "
  
  
#define PLOG(severity) LOG(severity) << ": " << errno
#define PLOG_IF(severity, condition) LOG_IF(severity) << ": " << errno
#define PCHECK(condition) CHECK(condition) << ": " << errno

// This is used by o3d/core/cross/types.h.
// We should probably unify all logging here.
#define NDEBUG_EAT_STREAM_PARAMETERS logging::nullStream()

#ifdef NDEBUG

#define DLOG(severity) LOG(severity)
#define DLOG_IF(severity, condition) LOG_IF(severity)
#define DLOG_ASSERT(condition) LOG_ASSERT(condition)
#define DCHECK(condition) CHECK(condition)
#define DCHECK_EQ(val1, val2) CHECK(val1 == val2)

#define DCHECK_NE(val1, val2) CHECK(val1 != val2)

#define DCHECK_LE(val1, val2) CHECK(val1 <= val2)

#define DCHECK_LT(val1, val2) CHECK(val1 < val2)

#define DCHECK_GE(val1, val2) CHECK(val1 >= val2)

#define DCHECK_GT(val1, val2) CHECK(val1 > val2)

#define DCHECK_STREQ(str1, str2) CHECK(str1.compare(str2) == 0)

#define DCHECK_STRCASEEQ(str1, str2) CHECK(CaseInsensitiveCompare(str1, str2))

#define DCHECK_STRNE(str1, str2) CHECK(str1.compare(str2) != 0)

#define DCHECK_STRCASENE(str1, str2) CHECK(!CaseInsensitiveCompare(str1, str2))

#else

#define DLOG(severity) logging::nullStream()
#define DLOG_IF(severity, condition) logging::nullStream()
#define DLOG_ASSERT(condition) logging::nullStream()
#define DCHECK(condition) logging::nullStream()
#define DCHECK_EQ(val1, val2) \
  while (false && (val1) == (val2)) logging::nullStream()

#define DCHECK_NE(val1, val2) \
  while (false && (val1) == (val2)) logging::nullStream()

#define DCHECK_LE(val1, val2) \
  while (false && (val1) == (val2)) logging::nullStream()

#define DCHECK_LT(val1, val2) \
  while (false && (val1) == (val2)) logging::nullStream()

#define DCHECK_GE(val1, val2) \
  while (false && (val1) == (val2)) logging::nullStream()

#define DCHECK_GT(val1, val2) \
  while (false && (val1) == (val2)) logging::nullStream()

#define DCHECK_STREQ(str1, str2) \
  while (false && (str1) == (str2)) logging::nullStream()

#define DCHECK_STRCASEEQ(str1, str2) \
  while (false && (str1) == (str2)) logging::nullStream()

#define DCHECK_STRNE(str1, str2) \
  while (false && (str1) == (str2)) logging::nullStream()

#define DCHECK_STRCASENE(str1, str2) \
  while (false && (str1) == (str2)) logging::nullStream()


#endif



// The NOTIMPLEMENTED() macro annotates codepaths which have
// not been implemented yet.
//
// The implementation of this macro is controlled by NOTIMPLEMENTED_POLICY:
//   0 -- Do nothing (stripped by compiler)
//   1 -- Warn at compile time
//   2 -- Fail at compile time
//   3 -- Fail at runtime (DCHECK)
//   4 -- [default] LOG(ERROR) at runtime
//   5 -- LOG(ERROR) at runtime, only once per call-site

#define NOTREACHED() DCHECK(false)

#ifndef NOTIMPLEMENTED_POLICY
// Select default policy: LOG(ERROR)
#define NOTIMPLEMENTED_POLICY 4
#endif

#if defined(COMPILER_GCC)
// On Linux, with GCC, we can use __PRETTY_FUNCTION__ to get the demangled name
// of the current function in the NOTIMPLEMENTED message.
#define NOTIMPLEMENTED_MSG "Not implemented reached in " << __PRETTY_FUNCTION__
#else
#define NOTIMPLEMENTED_MSG "NOT IMPLEMENTED"
#endif

#if NOTIMPLEMENTED_POLICY == 0
#define NOTIMPLEMENTED() ;
#elif NOTIMPLEMENTED_POLICY == 1
// TODO, figure out how to generate a warning
#define NOTIMPLEMENTED() COMPILE_ASSERT(false, NOT_IMPLEMENTED)
#elif NOTIMPLEMENTED_POLICY == 2
#define NOTIMPLEMENTED() COMPILE_ASSERT(false, NOT_IMPLEMENTED)
#elif NOTIMPLEMENTED_POLICY == 3
#define NOTIMPLEMENTED() NOTREACHED()
#elif NOTIMPLEMENTED_POLICY == 4
#define NOTIMPLEMENTED() LOG(ERROR) << NOTIMPLEMENTED_MSG
#elif NOTIMPLEMENTED_POLICY == 5
#define NOTIMPLEMENTED() do {\
  static int count = 0;\
  LOG_IF(ERROR, 0 == count++) << NOTIMPLEMENTED_MSG;\
} while(0)
#endif

#endif //LOGGING_SHIM_H_