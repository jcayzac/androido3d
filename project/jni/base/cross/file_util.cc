// Copyright (c) 2006-2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/cross/log.h"
#include "base/cross/file_util.h"
#include "base/cross/file_path.h"

#include <sys/param.h> // realpath (legacy)
#include <stdlib.h>    // realpath
#include <sys/stat.h>  // stat
#include <unistd.h>    // getcwd

namespace file_util {

// TODO(jcayzac): Replace these with boost::filesystem

bool AbsolutePath(FilePath* path) {
  char full_path[PATH_MAX];
  if (realpath(path->value().c_str(), full_path) == NULL)
    return false;
  *path = FilePath(full_path);
  return true;
}

bool PathExists(const FilePath& path) {
  struct stat file_info;
  return stat(path.value().c_str(), &file_info) == 0;
}

bool GetCurrentDirectory(FilePath* dir) {
  char system_buffer[PATH_MAX] = "";
  if (!getcwd(system_buffer, sizeof(system_buffer))) {
    O3D_NEVER_REACHED();
    return false;
  }
  *dir = FilePath(system_buffer);
  return true;
}

}  // namespace
