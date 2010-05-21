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


// This file contains the image file codec operations for OpenGL texture
// loading.

#include <stdio.h>
#include "core/cross/bitmap.h"
#include "utils/cross/file_path_utils.h"
#include "base/file_path.h"
#include "base/file_util.h"
#include "import/cross/memory_buffer.h"
#include "import/cross/memory_stream.h"

using file_util::OpenFile;
using file_util::CloseFile;

namespace o3d {

// Loads the header information and raw RGB{A} data from an uncompressed
// 24-bit or 32-bit TGA stream into the Bitmap object.
bool Bitmap::LoadFromTGAStream(ServiceLocator* service_locator,
                               MemoryReadStream *stream,
                               const String &filename,
                               BitmapRefArray* bitmaps) {
  // Read the magic header.
  uint8 file_magic[12];
  if (stream->Read(file_magic, sizeof(file_magic)) != sizeof(file_magic)) {
    DLOG(ERROR) << "Targa file magic not loaded \"" << filename << "\"";
    return false;
  }
  // Match the first few bytes of the TGA header to confirm we can read this
  // format. Multibyte values are stored little endian.
  static const uint8 kTargaMagic[12] = {
    0,     // ID Length (0 = no ID string present)
    0,     // Color Map Type ( 0 = no color map)
    2,     // Image Type (2 = Uncompressed True Color)
    0, 0,  // Color Map: First Entry Index (2 bytes)
    0, 0,  // Color Map: Table Length (2 bytes)
    0,     // Color Map: Entry Size
    0, 0,  // X-origin of image
    0, 0,  // Y-origin of image
           // MATCHED LATER: Image Width  (2 bytes)
           // MATCHED LATER: Image Height (2 bytes)
           // MATCHED LATER: Pixel Depth (1 byte)
           // MATCHED LATER: Image Descriptor (1 byte, alpha:4bit, origin:2bit)
  };

  // TODO(gman): The most common targa format is compressed! We should support
  // that format or remove targa support completely. If we are keeping targa
  // format we should also support grayscale, 8bit indexed and 16bit formats.
  if (memcmp(kTargaMagic, file_magic, sizeof(kTargaMagic)) != 0) {
    DLOG(ERROR) << "Targa file subtype not recognized \"" << filename << "\"";
    return false;
  }
  // Read the image header.
  uint8 header[6];
  if (stream->Read(header, sizeof(header)) != sizeof(header)) {
    DLOG(ERROR) << "Targa file header not read \"" << filename << "\"";
    return false;
  }
  // Calculate image width and height, stored as little endian.
  unsigned int tga_width  = header[1] * 256 + header[0];
  unsigned int tga_height = header[3] * 256 + header[2];
  if (!image::CheckImageDimensions(tga_width, tga_height)) {
    DLOG(ERROR) << "Failed to load " << filename
                << ": dimensions are too large (" << tga_width
                << ", " << tga_height << ").";
    return false;
  }
  unsigned int components = header[4] >> 3;
  // NOTE: Image Descriptor byte is skipped.
  if (components != 3 && components != 4) {
    DLOG(ERROR) << "Targa file  \"" << filename
                << "\"has an unsupported number of components";
    return false;
  }
  // pixels contained in the file.
  unsigned int pixel_count = tga_width * tga_height;
  // Allocate storage for the pixels.
  Texture::Format format = components == 3 ? Texture::XRGB8 : Texture::ARGB8;
  // Allocate storage for the pixels. Bitmap requires we allocate enough
  // memory for all mips even if we don't use them.
  size_t image_size = Bitmap::ComputeMaxSize(tga_width, tga_height, format);
  scoped_array<uint8> image_data(new uint8[image_size]);
  if (image_data.get() == NULL) {
    DLOG(ERROR) << "Targa file memory allocation error \"" << filename << "\"";
    return false;
  }
  // Read in the bitmap data.
  size_t bytes_to_read = pixel_count * components;
  if (stream->Read(image_data.get(), bytes_to_read) != bytes_to_read) {
    DLOG(ERROR) << "Targa file read failed \"" << filename << "\"";
    return false;
  }

  if (components == 3) {
    // Fixup the image by inserting an alpha value of 1 (BGR->BGRX).
    image::XYZToXYZA(image_data.get(), pixel_count);
  }

  // Success.
  Bitmap::Ref bitmap(new Bitmap(service_locator));
  bitmap->SetContents(format, 1, tga_width, tga_height, IMAGE, &image_data);
  bitmaps->push_back(bitmap);

  // Targas are generally bottom first in memory so flip it.
  //
  // TODO(gman): In truth a targa can be any orientation. We should check
  // that orientation and flip or not flip accordingly.
  bitmap->FlipVertically();

  return true;
}

}  // namespace o3d
