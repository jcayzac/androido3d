/*
 * Copyright 2010, Tonchidot Corporation.
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
 *     * Neither the name of Tonchidot Corporation. nor the names of its
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

// This file contains the image codec operations for JPG files.

#include <fstream>
#include "core/cross/bitmap.h"
#include "core/cross/error.h"
#include "core/cross/types.h"
#include "utils/cross/file_path_utils.h"
#include "base/file_path.h"
#include "base/file_util.h"
#include "import/cross/memory_buffer.h"
#include "import/cross/memory_stream.h"
#include "utils/cross/dataurl.h"

#include <CoreGraphics/CoreGraphics.h>

using file_util::OpenFile;
using file_util::CloseFile;

namespace o3d {
	
namespace {
size_t jpgProviderGetBytes (
						   void *info,
						   void *buffer,
						   size_t count
						   )
{
	MemoryReadStream *stream = static_cast<MemoryReadStream*>(info);
	return stream->Read(buffer, count);
}

off_t jpgProviderSkipForwardBytes (
								  void *info,
								  off_t count
								  )
{
	MemoryReadStream *stream = static_cast<MemoryReadStream*>(info);
	stream->Skip(count);
	return count;
}

void jpgProviderRewind (
					   void *info
					   )
{
	MemoryReadStream *stream = static_cast<MemoryReadStream*>(info);
	stream->Seek(0);
}

void jpgProviderReleaseInfo (
							void *info
							)
{
}
}  // anonymous namespace

// Loads the raw RGB data from a compressed JPG file.
bool Bitmap::LoadFromJPEGStream(ServiceLocator* service_locator,
								MemoryReadStream *stream,
								const String &filename,
								BitmapRefArray* bitmaps) {
	DCHECK(bitmaps);
	CGDataProviderSequentialCallbacks jpgProviderCallbacks;
	jpgProviderCallbacks.version = 0;
	jpgProviderCallbacks.getBytes = jpgProviderGetBytes;
	jpgProviderCallbacks.skipForward = jpgProviderSkipForwardBytes;
	jpgProviderCallbacks.rewind = jpgProviderRewind;
	jpgProviderCallbacks.releaseInfo = jpgProviderReleaseInfo;
	
	CGDataProviderRef jpgProvider = CGDataProviderCreateSequential(stream, &jpgProviderCallbacks);
	CGImageRef jpgImage = CGImageCreateWithJPEGDataProvider(jpgProvider, NULL, false, kCGRenderingIntentDefault);

	CFDataRef data = CGDataProviderCopyData(CGImageGetDataProvider(jpgImage));
	if (!data) {
		return false;
	}

	scoped_array<uint8> image_data(new uint8[CFDataGetLength(data)]);
	CFDataGetBytes(data, CFRangeMake(0,CFDataGetLength(data)), image_data.get());
	
	Texture::Format format = Texture::UNKNOWN_FORMAT;
	// 24-bit RGB image or 32-bit RGBA image.
	if (CGImageGetAlphaInfo(jpgImage)) {
		format = Texture::ARGB8;
	} else {
		format = Texture::XRGB8;
	}
	
	// Success.
	Bitmap::Ref bitmap(new Bitmap(service_locator));
	unsigned int width = CGImageGetWidth(jpgImage);
	unsigned int height = CGImageGetHeight(jpgImage);
	bitmap->SetContents(format, 1, width, height, IMAGE, &image_data);
	bitmaps->push_back(bitmap);

	CFRelease(data);
	CGImageRelease(jpgImage);
	return true;
}

}  // namespace o3d
