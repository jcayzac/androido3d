// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file contains utility functions for dealing with the local
// filesystem.

// TODO: remove this and use boost::filesystem

#ifndef BASE_FILE_UTIL_H_
#define BASE_FILE_UTIL_H_

class FilePath;
namespace file_util {

// Convert provided relative path into an absolute path.  Returns false on
// error. On POSIX, this function fails if the path does not exist.
bool AbsolutePath(FilePath* path);

// Returns true if the given path exists on the local filesystem,
// false otherwise.
bool PathExists(const FilePath& path);

// Gets the current working directory for the process.
bool GetCurrentDirectory(FilePath* path);

}  // namespace file_util

#endif  // BASE_FILE_UTIL_H_
