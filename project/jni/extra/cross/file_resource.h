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
#include "extra/cross/external_resource.h"
#include <string>

namespace o3d {
namespace extra {

class FileResource: public ExternalResource {
 public:
  explicit FileResource(const std::string& path);
  virtual ~FileResource();
  const uint8* const data() const {
    return mData;
  }
  size_t size() const {
    return mSize;
  }
 protected:
  int          mFD;
  uint8*       mData;
  size_t       mSize;
  std::string  mURI;
};

} // namespace extra
} // namespace o3d

/* vim: set sw=2 ts=2 sts=2 expandtab ff=unix: */
