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
#include "base/cross/log.h"

#if defined(OS_ANDROID)
// Android only provide syslog() for kernel debugging,
// so we can't just use log_posix.inl.cc
#include "base/cross/log_android.inl.cc"
#elif defined(OS_POSIX)
// Standard syslog() implementation
#include "base/cross/log_posix.inl.cc"
#else
// Simple STDERR output
#include "base/cross/log_default.inl.cc"
#endif
