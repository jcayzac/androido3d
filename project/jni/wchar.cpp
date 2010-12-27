/*
 * Copyright (C) 2009 The Android Open Source Project
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

// These are here to provide working implementations for some of the
// wchar_t functions since they are missing in the android ndk version 4.

#include <stdlib.h>

extern "C" {

size_t wcslen(const wchar_t *str) {
  const wchar_t* s = str;
  for (;*s;s++);
  return s - str;
}

int wcscmp(
   const wchar_t *string1,
   const wchar_t *string2
) {
  while (*string1 == *string2) {
    if (!*string1) {
      return 0;
    }
    ++string1;
    ++string2;
  }
  return *string1 < *string2 ? -1 : 1;
}

wchar_t *wcsncpy(
   wchar_t *strDest,
   const wchar_t *strSource,
   size_t count
) {
  wchar_t* dst = strDest;
  while (count && *strSource) {
    *dst++ = *strSource++;
  }
  if (count > 0) {
    *dst = 0;
  }
  return strDest;
}

wchar_t *wcschr(
   const wchar_t *str,
   wchar_t c
) {
  while (*str) {
    if (*str == c) {
      return const_cast<wchar_t*>(str);
    }
    ++str;
  }
  return NULL;
}

wchar_t *wcsrchr(
   const wchar_t *str,
   wchar_t c
) {
  const wchar_t* end = str + wcslen(str);
  while (end != str) {
    --end;
    if (*end == c) {
      return const_cast<wchar_t*>(end);
    }
  }
  return NULL;
}

wchar_t *wmemmove(
   wchar_t *dest,
   const wchar_t *src,
   size_t count
) {
  memmove(dest, src, count * sizeof(*src));
  return dest;
}

wchar_t *wmemcpy(
   wchar_t *dest,
   const wchar_t *src,
   size_t count
) {
  memcpy(dest, src, count * sizeof(*src));
  return dest;
}

int wmemcmp(
   const wchar_t * buf1,
   const wchar_t * buf2,
   size_t count) {
  while (count) {
    if (*buf1 != *buf2) {
      return *buf1 < *buf2 ? -1 : 1;
    }
    ++buf1;
    ++buf2;
    --count;
  }
  return 0;
}
wchar_t *wmemset(
   wchar_t *dest,
   wchar_t c,
   size_t count
) {
  wchar_t* end = dest + count;
  while (dest != end) {
    *dest++ = c;
  }
}

// TODO(gman): implement these
//     snwprintf
//     vsnwprintf
//     _wcsupr
//     wcsicmp
//     wcslwr

}  // extern "C"
