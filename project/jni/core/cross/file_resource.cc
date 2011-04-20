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

#include "core/cross/file_resource.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace o3d {

FileResource::FileResource(const std::string& path)
: mFD(-1), mData(0), mSize(0) {
  mName=path;
  mFD = open(path.c_str(), O_RDONLY);
  if (mFD>=0) {
    off_t start, end;
    start = lseek(mFD, 0L, SEEK_CUR);
    end = lseek(mFD, 0L, SEEK_END);
    lseek(mFD, start, SEEK_SET);
    mSize = (size_t) (end-start);
    mData = (uint8_t*) mmap(0, mSize, PROT_READ, MAP_FILE|MAP_SHARED, mFD, 0);
    if (mData==MAP_FAILED) {
      mData = 0;
      mSize = 0;
    }
  }
}

FileResource::~FileResource() {
  if (mData) {
    munmap(mData, mSize);
  }
  if (mFD>=0) {
    close(mFD);
  }
}

} // namespace o3d

/* vim: set sw=2 ts=2 sts=2 expandtab ff=unix: */
