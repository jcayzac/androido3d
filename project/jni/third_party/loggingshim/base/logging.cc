
#include "base/logging.h"

#include <sstream>
#include <string>

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
    
	 	__android_log_print(mSeverity, mTag, mInput.str().c_str());

    if (mSeverity == FATAL) {
      //__android_log_assert(false, mTag, mInput.str().c_str());
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
