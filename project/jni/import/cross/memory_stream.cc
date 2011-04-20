/*
 * Copyright 2009, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//
// MemoryReadStream and MemoryWriteStream are simple stream wrappers around
// memory buffers.

#include "import/cross/memory_stream.h"
#include "core/cross/types.h"

namespace o3d {

namespace {

inline bool is_little_endian() {
  union {
    uint32_t u;
    uint8_t  b[4];
  } x;
  x.u = 0xdeadbeef;
  return x.b[0]==0xef; 
}

union Int16Union {
  uint8_t c[2];
  int16_t i;
};

union UInt16Union {
  uint8_t c[2];
  uint16_t i;
};

union Int32Union {
  uint8_t c[4];
  int32_t i;
};

union UInt32Union {
  uint8_t c[4];
  uint32_t i;
};

union Int32FloatUnion {
  int32_t i;
  float f;
};

union UInt32FloatUnion {
  uint32_t i;
  float f;
};
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static int16_t SwapInt16(const int16_t *value) {
  Int16Union u;
  u.i=*value;
  std::swap(u.c[0], u.c[1]);
  return u.i;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static uint16_t SwapUInt16(const uint16_t *value) {
  UInt16Union u;
  u.i=*value;
  std::swap(u.c[0], u.c[1]);
  return u.i;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static int32_t SwapInt32(const int32_t *value) {
  Int32Union u;
  u.i=*value;
  std::swap(u.c[0], u.c[3]);
  std::swap(u.c[1], u.c[2]);
  return u.i;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static uint32_t SwapUInt32(const uint32_t *value) {
  UInt32Union u;
  u.i=*value;
  std::swap(u.c[0], u.c[3]);
  std::swap(u.c[1], u.c[2]);
  return u.i;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int16_t MemoryReadStream::ReadLittleEndianInt16() {
  Int16Union u;
  Read(u.c, 2);

#ifdef IS_BIG_ENDIAN
  return SwapInt16(&u.i);
#else
  return u.i;
#endif
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
uint16_t MemoryReadStream::ReadLittleEndianUInt16() {
  UInt16Union u;
  Read(u.c, 2);

#ifdef IS_BIG_ENDIAN
  return SwapUint16(&u.i);
#else
  return u.i;
#endif
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int16_t MemoryReadStream::ReadBigEndianInt16() {
  Int16Union u;
  Read(u.c, 2);
  if (is_little_endian()) return SwapInt16(&u.i);
  else return u.i;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
uint16_t MemoryReadStream::ReadBigEndianUInt16() {
  UInt16Union u;
  Read(u.c, 2);

  if (is_little_endian()) return SwapUInt16(&u.i);
  else return u.i;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int32_t MemoryReadStream::ReadLittleEndianInt32() {
  Int32Union u;
  Read(u.c, 4);

  if (is_little_endian()) return u.i;
  else return SwapInt32(&u.i);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
uint32_t MemoryReadStream::ReadLittleEndianUInt32() {
  UInt32Union u;
  Read(u.c, 4);

  if (is_little_endian()) return u.i;
  else return SwapUInt32(&u.i);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int32_t MemoryReadStream::ReadBigEndianInt32() {
  Int32Union u;
  Read(u.c, 4);

  if (is_little_endian()) return SwapInt32(&u.i);
  else return u.i;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
uint32_t MemoryReadStream::ReadBigEndianUInt32() {
  UInt32Union u;
  Read(u.c, 4);

  if (is_little_endian()) return SwapUInt32(&u.i);
  else return u.i;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
float MemoryReadStream::ReadLittleEndianFloat32() {
  // Read in as int32_t then interpret as float32
  Int32FloatUnion u;
  u.i = ReadLittleEndianInt32();
  return u.f;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
float MemoryReadStream::ReadBigEndianFloat32() {
  // Read in as int32_t then interpret as float32
  Int32FloatUnion u;
  u.i = ReadBigEndianInt32();
  return u.f;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MemoryWriteStream::WriteLittleEndianInt16(int16_t i) {
  Int16Union u;

  if (is_little_endian()) u.i=i;
  else u.i = SwapInt16(&i);

  Write(u.c, 2);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MemoryWriteStream::WriteLittleEndianUInt16(uint16_t i) {
  UInt16Union u;

  if (is_little_endian()) u.i = i;
  else u.i = SwapUInt16(&i);

  Write(u.c, 2);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MemoryWriteStream::WriteBigEndianInt16(int16_t i) {
  Int16Union u;

  if (is_little_endian()) u.i = SwapInt16(&i);
  else u.i = i;

  Write(u.c, 2);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MemoryWriteStream::WriteBigEndianUInt16(uint16_t i) {
  UInt16Union u;

  if (is_little_endian()) u.i = SwapUInt16(&i);
  else u.i = i;

  Write(u.c, 2);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MemoryWriteStream::WriteLittleEndianInt32(int32_t i) {
  Int32Union u;

  if (is_little_endian()) u.i = i;
  else u.i = SwapInt32(&i);

  Write(u.c, 4);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MemoryWriteStream::WriteLittleEndianUInt32(uint32_t i) {
  UInt32Union u;

  if (is_little_endian()) u.i = i;
  else u.i = SwapUInt32(&i);

  Write(u.c, 4);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MemoryWriteStream::WriteBigEndianInt32(int32_t i) {
  Int32Union u;

  if (is_little_endian()) u.i = SwapInt32(&i);
  else u.i = i;

  Write(u.c, 4);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MemoryWriteStream::WriteBigEndianUInt32(uint32_t i) {
  UInt32Union u;

  if (is_little_endian()) u.i = SwapUInt32(&i);
  else u.i = i;

  Write(u.c, 4);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MemoryWriteStream::WriteLittleEndianFloat32(float f) {
  // Interpret byte-pattern of f as int32_t and write out
  Int32FloatUnion u;
  u.f = f;
  WriteLittleEndianInt32(u.i);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MemoryWriteStream::WriteBigEndianFloat32(float f) {
  // Interpret byte-pattern of f as int32_t and write out
  Int32FloatUnion u;
  u.f = f;
  WriteBigEndianInt32(u.i);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// A couple of useful utility functions

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Returns the bytes pointed to by |value| as little-endian 16
int16_t MemoryReadStream::GetLittleEndianInt16(const int16_t *value) {
  if (is_little_endian()) return *value;
  else return SwapInt16(value);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Returns the bytes pointed to by |value| as little-endian 16
uint16_t MemoryReadStream::GetLittleEndianUInt16(const uint16_t *value) {
  if (is_little_endian()) return *value;
  else return SwapUInt16(value);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Returns the bytes pointed to by |value| as little-endian 32
int32_t MemoryReadStream::GetLittleEndianInt32(const int32_t *value) {
  if (is_little_endian()) return *value;
  else return SwapInt32(value);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Returns the bytes pointed to by |value| as little-endian 32
uint32_t MemoryReadStream::GetLittleEndianUInt32(const uint32_t *value) {
  if (is_little_endian()) return *value;
  else return SwapUInt32(value);
}

}  // namespace o3d
