
#include "base/logging.h"

#include <sstream>

namespace logging {
  
  Logger& getLogger() {
    static Logger sLogStream;
    return sLogStream;
  }
  
  Logger Logger::printLog(LogSeverity severity, const char* tag) {
    mSeverity = severity;
    mTag = tag;
  }
  
  std::ostream& Logger::operator<<(const std::string& s) {
   if (mSeverity == FATAL) {
      __android_log_assert("", mTag, s.c_str());
    } else {
      __android_log_write(mSeverity, mTag, s.c_str());
    }
    
    return nullStream();
  }
  
  std::ostream& nullStream() {
    static std::ostringstream null_stream;
    null_stream.clear(std::ios::failbit);
    return null_stream;
  }

};
