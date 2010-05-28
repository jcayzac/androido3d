// Copyright (c) 2006-2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "build/build_config.h"
#include "base/debug_util.h"


#include <iostream>
#include <string>

// static
bool DebugUtil::SpawnDebuggerOnProcess(unsigned /* process_id */) {
  return false;
}

bool DebugUtil::BeingDebugged() {
  return false;
}

// static
void DebugUtil::BreakDebugger() {
}

StackTrace::StackTrace() {
}

void StackTrace::PrintBacktrace() {
}

void StackTrace::OutputToStream(std::ostream* os) {
}
