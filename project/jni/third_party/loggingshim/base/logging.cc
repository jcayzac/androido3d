
#include "base/logging.h"

#include <sstream>

namespace logging {
  
 Logger::~Logger() {
    if (mShowLocation) {
      mInput << "\t" << mLocation;
    }
    
    if (mSeverity == FATAL) {
      __android_log_assert("", mTag, mInput.str().c_str());
    } else {
      __android_log_print(mSeverity, mTag, mInput.str().c_str());
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
