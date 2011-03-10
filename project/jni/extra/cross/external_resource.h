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
#include "core/cross/smart_ptr.h"

namespace o3d {
namespace extra {

/** @brief An external resource.
  *
  * This class is basically just a reference-counted
  * interface to a data holding object.
  *
  * It provides a simple way to handle arbitrary
  * data from various sources (mmapped file,
  * memory, database, etc)
  */
class ExternalResource: public RefCounted  {
 public:
  typedef SmartPointer<ExternalResource> Ref;

  virtual ~ExternalResource() { }

  /// @return Pointer to the data
  virtual const uint8* const data() const = 0;

  /// @return Size of the data
  virtual size_t size() const = 0;
};

} // namespace extra
} // namespace o3d

/* vim: set sw=2 ts=2 sts=2 expandtab ff=unix: */
