
#include "base/logging.h"

#include <sstream>
#include <string>

#if !defined(TARGET_OS_IPHONE)
#define  LOGASSERT(_severity, _tag, _va_args)  __android_log_assert(_severity,_tag,_va_args)
#define  LOGPRINT(_severity, _tag, _va_args)  __android_log_print(_severity,_tag,_va_args)
#else
#define  LOGASSERT(_severity, _tag, _msg)  LOGE("%s [%s] %s\n",_tag, strrchr(mLocation,'/')+1, _msg)
#define  LOGPRINT(_severity, _tag, _msg)  LOGW("%s [%s] %s\n",_tag, strrchr(mLocation,'/')+1, _msg)
#endif

namespace logging {
 
Logger::Logger(LogSeverity severity, const char* tag, const char* file, int line, bool showLocation)
        : mShowLocation(showLocation),
          mSeverity(severity),
          mTag(tag) { 
    std::ostringstream location;
    location << file << ":" << line; 
    mLocation = location.str().c_str(); 
  }

 Logger::~Logger() {
    if (mShowLocation || mSeverity == FATAL) {
      mInput << "\t" << mLocation;
    }
    
	 	LOGPRINT(mSeverity, mTag, mInput.str().c_str());

    if (mSeverity == FATAL) {
      //LOGASSERT(false, mTag, mInput.str().c_str());
      exit(0);
    } 
  }
  
  std::ostringstream& Logger::printLog() {
    return mInput;
  }
  
  std::ostream& nullStream() {
    static std::ostringstream null_stream;
    null_stream.clear(std::ios::failbit);
    return null_stream;
  }

};
