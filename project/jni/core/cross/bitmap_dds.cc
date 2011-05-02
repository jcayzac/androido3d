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

#include "base/cross/config.h"
#if defined(O3D_DECOMPRESS_DXT)
  #include "libtxc_dxtn/files/txc_dxtn.h"
#endif

// This file contains the image codec operations for DDS files.
#include "core/cross/error.h"
#include "core/cross/bitmap.h"
#include "core/cross/ddsurfacedesc.h"
#include "base/cross/file_util.h"
#include "import/cross/memory_buffer.h"
#include "import/cross/memory_stream.h"
#include <cstring>

namespace o3d {

// LoadFromDDSFile -------------------------------------------------------------

// A function that flips a DXTC block.
typedef void (* FlipBlockFunction)(uint8_t *block);

// Flips a full DXT1 block in the y direction.
static void FlipDXT1BlockFull(uint8_t *block) {
  // A DXT1 block layout is:
  // [0-1] color0.
  // [2-3] color1.
  // [4-7] color bitmap, 2 bits per pixel.
  // So each of the 4-7 bytes represents one line, flipping a block is just
  // flipping those bytes.
  uint8_t tmp = block[4];
  block[4] = block[7];
  block[7] = tmp;
  tmp = block[5];
  block[5] = block[6];
  block[6] = block[5];
}

// Flips the first 2 lines of a DXT1 block in the y direction.
static void FlipDXT1BlockHalf(uint8_t *block) {
  // See layout above.
  uint8_t tmp = block[4];
  block[4] = block[5];
  block[5] = tmp;
}

// Flips a full DXT3 block in the y direction.
static void FlipDXT3BlockFull(uint8_t *block) {
  // A DXT3 block layout is:
  // [0-7]  alpha bitmap, 4 bits per pixel.
  // [8-15] a DXT1 block.

  // We can flip the alpha bits at the byte level (2 bytes per line).
  uint8_t tmp = block[0];
  block[0] = block[6];
  block[6] = tmp;
  tmp = block[1];
  block[1] = block[7];
  block[7] = tmp;
  tmp = block[2];
  block[2] = block[4];
  block[4] = tmp;
  tmp = block[3];
  block[3] = block[5];
  block[5] = tmp;

  // And flip the DXT1 block using the above function.
  FlipDXT1BlockFull(block + 8);
}

// Flips the first 2 lines of a DXT3 block in the y direction.
static void FlipDXT3BlockHalf(uint8_t *block) {
  // See layout above.
  uint8_t tmp = block[0];
  block[0] = block[2];
  block[2] = tmp;
  tmp = block[1];
  block[1] = block[3];
  block[3] = tmp;
  FlipDXT1BlockHalf(block + 8);
}

// Flips a full DXT5 block in the y direction.
static void FlipDXT5BlockFull(uint8_t *block) {
  // A DXT5 block layout is:
  // [0]    alpha0.
  // [1]    alpha1.
  // [2-7]  alpha bitmap, 3 bits per pixel.
  // [8-15] a DXT1 block.

  // The alpha bitmap doesn't easily map lines to bytes, so we have to
  // interpret it correctly.  Extracted from
  // http://www.opengl.org/registry/specs/EXT/texture_compression_s3tc.txt :
  //
  //   The 6 "bits" bytes of the block are decoded into one 48-bit integer:
  //
  //     bits = bits_0 + 256 * (bits_1 + 256 * (bits_2 + 256 * (bits_3 +
  //                   256 * (bits_4 + 256 * bits_5))))
  //
  //   bits is a 48-bit unsigned integer, from which a three-bit control code
  //   is extracted for a texel at location (x,y) in the block using:
  //
  //       code(x,y) = bits[3*(4*y+x)+1..3*(4*y+x)+0]
  //
  //   where bit 47 is the most significant and bit 0 is the least
  //   significant bit.
  unsigned int line_0_1 = block[2] + 256 * (block[3] + 256 * block[4]);
  unsigned int line_2_3 = block[5] + 256 * (block[6] + 256 * block[7]);
  // swap lines 0 and 1 in line_0_1.
  unsigned int line_1_0 = ((line_0_1 & 0x000fff) << 12) |
      ((line_0_1 & 0xfff000) >> 12);
  // swap lines 2 and 3 in line_2_3.
  unsigned int line_3_2 = ((line_2_3 & 0x000fff) << 12) |
      ((line_2_3 & 0xfff000) >> 12);
  block[2] = line_3_2 & 0xff;
  block[3] = (line_3_2 & 0xff00) >> 8;
  block[4] = (line_3_2 & 0xff0000) >> 8;
  block[5] = line_1_0 & 0xff;
  block[6] = (line_1_0 & 0xff00) >> 8;
  block[7] = (line_1_0 & 0xff0000) >> 8;

  // And flip the DXT1 block using the above function.
  FlipDXT1BlockFull(block + 8);
}

// Flips the first 2 lines of a DXT5 block in the y direction.
static void FlipDXT5BlockHalf(uint8_t *block) {
  // See layout above.
  unsigned int line_0_1 = block[2] + 256 * (block[3] + 256 * block[4]);
  unsigned int line_1_0 = ((line_0_1 & 0x000fff) << 12) |
      ((line_0_1 & 0xfff000) >> 12);
  block[2] = line_1_0 & 0xff;
  block[3] = (line_1_0 & 0xff00) >> 8;
  block[4] = (line_1_0 & 0xff0000) >> 8;
  FlipDXT1BlockHalf(block + 8);
}

// Flips a DXTC image, by flipping and swapping DXTC blocks as appropriate.
static void FlipDXTCImage(unsigned int width,
                          unsigned int height,
                          unsigned int levels,
                          Texture::Format format,
                          uint8_t *data) {
  O3D_ASSERT(image::CheckImageDimensions(width, height));
  // Height must be a power-of-two.
  O3D_ASSERT((height & (height - 1)) == 0u);
  FlipBlockFunction full_block_function = NULL;
  FlipBlockFunction half_block_function = NULL;
  unsigned int block_bytes = 0;
  switch (format) {
    case Texture::DXT1:
      full_block_function = FlipDXT1BlockFull;
      half_block_function = FlipDXT1BlockHalf;
      block_bytes = 8;
      break;
    case Texture::DXT3:
      full_block_function = FlipDXT3BlockFull;
      half_block_function = FlipDXT3BlockHalf;
      block_bytes = 16;
      break;
    case Texture::DXT5:
      full_block_function = FlipDXT5BlockFull;
      half_block_function = FlipDXT5BlockHalf;
      block_bytes = 16;
      break;
    default:
      O3D_NEVER_REACHED();
      return;
  }
  unsigned int mip_width = width;
  unsigned int mip_height = height;
  for (unsigned int i = 0; i < levels; ++i) {
    unsigned int blocks_per_row = (mip_width + 3) / 4;
    unsigned int blocks_per_col = (mip_height + 3) / 4;
    unsigned int blocks = blocks_per_row * blocks_per_col;
    if (mip_height == 1) {
      // no flip to do, and we're done.
      break;
    } else if (mip_height == 2) {
      // flip the first 2 lines in each block.
      for (unsigned int i = 0; i < blocks_per_row; ++i) {
        half_block_function(data + i * block_bytes);
      }
    } else {
      // flip each block.
      for (unsigned int i = 0; i < blocks; ++i) {
        full_block_function(data + i * block_bytes);
      }
      // swap each block line in the first half of the image with the
      // corresponding one in the second half.
      // note that this is a no-op if mip_height is 4.
      unsigned int row_bytes = block_bytes * blocks_per_row;
      ::o3d::base::scoped_array<uint8_t> temp_line(new uint8_t[row_bytes]);
      for (unsigned int y = 0; y < blocks_per_col / 2; ++y) {
        uint8_t *line1 = data + y * row_bytes;
        uint8_t *line2 = data + (blocks_per_col - y - 1) * row_bytes;
        memcpy(temp_line.get(), line1, row_bytes);
        memcpy(line1, line2, row_bytes);
        memcpy(line2, temp_line.get(), row_bytes);
      }
    }
    // mip levels are contiguous.
    data += block_bytes * blocks;
    mip_width = std::max(1U, mip_width >> 1);
    mip_height = std::max(1U, mip_height >> 1);
  }
}

// Flips a BGRA image, by simply swapping pixel rows.
static void FlipBGRAImage(unsigned int width,
                          unsigned int height,
                          unsigned int levels,
                          Texture::Format format,
                          uint8_t *data) {
  O3D_ASSERT(image::CheckImageDimensions(width, height));
  O3D_ASSERT(format != Texture::DXT1 && format != Texture::DXT3 &&
         format != Texture::DXT5);
  size_t pixel_bytes = image::ComputeMipChainSize(1, 1, format, 1);
  unsigned int mip_width = width;
  unsigned int mip_height = height;
  // rows are at most as big as the first one.
  ::o3d::base::scoped_array<uint8_t> temp_line(
      new uint8_t[mip_width * pixel_bytes]);
  for (unsigned int i = 0; i < levels; ++i) {
    unsigned int row_bytes = pixel_bytes * mip_width;
    for (unsigned int y = 0; y < mip_height / 2; ++y) {
      uint8_t *line1 = data + y * row_bytes;
      uint8_t *line2 = data + (mip_height - y - 1) * row_bytes;
      memcpy(temp_line.get(), line1, row_bytes);
      memcpy(line1, line2, row_bytes);
      memcpy(line2, temp_line.get(), row_bytes);
    }
    // mip levels are contiguous.
    data += row_bytes * mip_height;
    mip_width = std::max(1U, mip_width >> 1);
    mip_height = std::max(1U, mip_height >> 1);
  }
}

#if defined(O3D_DECOMPRESS_DXT)
static bool DecompressDDSBitmaps(ServiceLocator* service_locator, BitmapRefArray& bitmaps) {
  bool is_cube_map(bitmaps.size() == 6);
  for (unsigned int i = 0; i < bitmaps.size(); i++) {
    Bitmap::Ref src_bitmap(bitmaps[i]);
    Bitmap::Ref dst_bitmap(new Bitmap(service_locator));
    dst_bitmap->Allocate(Texture::ARGB8,
                         src_bitmap->width(),
                         src_bitmap->height(),
                         src_bitmap->num_mipmaps(),
                         src_bitmap->semantic());
    bitmaps[i] = dst_bitmap;
    Texture::Format src_format(src_bitmap->format());
    bool is_compressed =
      (src_format == Texture::DXT1 ||
       src_format == Texture::DXT3 ||
       src_format == Texture::DXT5);
    // (jcayzac) select the right convertion function outside of the pixel loop
    void (*fetch_2d_texel_rgba)(GLint, const GLubyte*, GLint, GLint, GLvoid *) = 0;
    switch (src_format) {
      case Texture::DXT1: fetch_2d_texel_rgba=fetch_2d_texel_rgba_dxt1; break;
      case Texture::DXT3: fetch_2d_texel_rgba=fetch_2d_texel_rgba_dxt3; break;
      case Texture::DXT5: fetch_2d_texel_rgba=fetch_2d_texel_rgba_dxt5; break;
      default: O3D_ERROR(service_locator) << "Unsupported DDS compressed texture format " << src_format;
               return false;
    }
    for (unsigned level = 0; level < src_bitmap->num_mipmaps(); ++level) {
      int pitch = src_bitmap->GetMipPitch(level);
      if (is_compressed) {
        // The pitch returned by GetMipPitch for compressed textures
        // is the number of bytes across a row of DXT blocks where as
        // libtxc_dxtn wants the number of bytes across a row of pixels.
        pitch /= 2;  // there are 4 rows in a block so I don't understand why 2
                     // works.
      }
      uint8_t* data = src_bitmap->GetMipData(level);
      int width  = std::max(1U, src_bitmap->width() >> level);
      int height = std::max(1U, src_bitmap->height() >> level);
      int row_width = width * 4;
      int decompressed_size = width * height * 4;
      ::o3d::base::scoped_array<uint8_t> decompressed_data(new uint8_t[decompressed_size]);
      memset(decompressed_data.get(), 0, decompressed_size);
      if (is_compressed) {
        for (int src_y = 0; src_y < height; src_y++) {
          int dest_y = src_y;
          if (is_cube_map) {
            dest_y = height - src_y - 1;
          }
          for (int x = 0; x < width; ++x) {
            uint8_t* ptr =
                &decompressed_data.get()[row_width * dest_y + 4 * x];
            fetch_2d_texel_rgba(pitch, data, x, src_y, ptr);
            // Need to swap the red and blue channels.
            std::swap(ptr[0], ptr[2]);
          }
        }
      } else if (src_format == Texture::XRGB8 ||
                 src_format == Texture::ARGB8) {
        for (int src_y = 0; src_y < height; src_y++) {
          int dest_y = src_y;
          if (is_cube_map) {
            dest_y = height - src_y - 1;
          }
          memcpy(decompressed_data.get() + row_width * dest_y,
                 data + pitch * src_y,
                 row_width);
        }
      } else {
        O3D_ERROR(service_locator)
            << "Unsupported DDS uncompressed texture format "
            << src_bitmap->format();
        return false;
      }
      dst_bitmap->SetRect(level, 0, 0, width, height,
                          decompressed_data.get(),
                          row_width);
    }
  }
  return true;
}
#endif



void Bitmap::FlipVertically() {
  if (format() == Texture::DXT1 ||
      format() == Texture::DXT3 ||
      format() == Texture::DXT5) {
    FlipDXTCImage(width(), height(), num_mipmaps(), format(), image_data());
  } else {
    FlipBGRAImage(width(), height(), num_mipmaps(), format(), image_data());
  }
}


// Load the bitmap data as DXTC compressed data from a DDS stream into the
// Bitmap object. This routine only supports compressed DDS formats DXT1,
// DXT3 and DXT5.
bool Bitmap::LoadFromDDSStream(ServiceLocator* service_locator,
                               MemoryReadStream *stream,
                               const std::string &filename,
                               BitmapRefArray* bitmaps) {
  // Verify the file is a true .dds file
  char magic[4];
  size_t bytes_read = stream->Read(magic, sizeof(magic));
  if (bytes_read != sizeof(magic)) {
    O3D_ERROR(service_locator) << "DDS magic header not read \"" << filename << "\"";
    return false;
  }
  if (std::strncmp(magic, "DDS ", 4) != 0) {
    O3D_ERROR(service_locator) << "DDS magic header not recognized \"" << filename << "\"";
    return false;
  }
  // Get the DirectDraw Surface Descriptor
  DDSURFACEDESC2 dd_surface_descriptor;
  if (!stream->ReadAs<DDSURFACEDESC2>(&dd_surface_descriptor)) {
    O3D_ERROR(service_locator) << "DDS header not read \"" << filename << "\"";
    return false;
  }
  const unsigned int kRequiredFlags =
      DDSD_CAPS |
      DDSD_HEIGHT |
      DDSD_WIDTH |
      DDSD_PIXELFORMAT;
  if ((dd_surface_descriptor.dwFlags & kRequiredFlags) != kRequiredFlags) {
    O3D_ERROR(service_locator) << "Required DDS flags are absent in \"" << filename << "\".";
    return false;
  }
  // NOTE: Add permissible flags as appropriate here when supporting new
  // formats.
  const unsigned int kValidFlags = kRequiredFlags |
      DDSD_MIPMAPCOUNT |
      DDSD_LINEARSIZE;
  if (dd_surface_descriptor.dwFlags & ~kValidFlags) {
    O3D_ERROR(service_locator) << "Invalid DDS flags combination \"" << filename << "\".";
    return false;
  }
  unsigned int mip_count = (dd_surface_descriptor.dwFlags & DDSD_MIPMAPCOUNT) ?
      dd_surface_descriptor.dwMipMapCount : 1;
  unsigned int dds_width = dd_surface_descriptor.dwWidth;
  unsigned int dds_height = dd_surface_descriptor.dwHeight;
  if (!image::CheckImageDimensions(dds_width, dds_height)) {
    O3D_ERROR(service_locator) << "Failed to load " << filename
                << ": dimensions are too large (" << dds_width
                << ", " << dds_height << ").";
    return false;
  }

  if (mip_count > image::ComputeMipMapCount(dds_width, dds_height)) {
    O3D_ERROR(service_locator) << "Failed to load " << filename
                << ": mip count " << mip_count
                << "is inconsistent with image dimensions ("
                <<  dds_width<< ", " << dds_height << ").";
    return false;
  }

  // Check for cube maps
  bool is_cubemap =
      (dd_surface_descriptor.ddsCaps.dwCaps2 & DDSCAPS2_CUBEMAP) != 0;
  // Cube maps should have all the face flags set - otherwise the cube map is
  // incomplete.
  if (is_cubemap) {
    if ((dd_surface_descriptor.ddsCaps.dwCaps2 & DDSCAPS2_CUBEMAP_ALLFACES) !=
        DDSCAPS2_CUBEMAP_ALLFACES) {
      O3D_ERROR(service_locator) << "DDS file \"" << filename
                  << "\" is a cube map but doesn't have all the faces.";
      return false;
    }
    if (dds_width != dds_height) {
      O3D_ERROR(service_locator) << "DDS file \"" << filename
                  << "\" is a cube map but doesn't have square dimensions.";
      return false;
    }
  }

  // The size of the buffer needed to hold four-component per pixel
  // image data, including MIPMaps
  unsigned int components_per_pixel = 0;
  bool add_filler_alpha = false;
  bool rgb_to_bgr = false;

  Texture::Format format = Texture::UNKNOWN_FORMAT;
  bool is_dxtc = false;

  DDPIXELFORMAT &pixel_format = dd_surface_descriptor.ddpfPixelFormat;

  if (pixel_format.dwFlags & DDPF_FOURCC) {
    switch (pixel_format.dwFourCC) {
      case FOURCC_DXT1 : {
        format = Texture::DXT1;
        is_dxtc = true;
        break;
      }
      case FOURCC_DXT3 : {
        format = Texture::DXT3;
        is_dxtc = true;
        break;
      }
      case FOURCC_DXT5 : {
        format = Texture::DXT5;
        is_dxtc = true;
        break;
      }
      default : {
        O3D_ERROR(service_locator) << "DDS format not DXT1, DXT3 or DXT5. \"" << filename << "\"";
        return false;
      }
    }

    // Check that the advertised size is correct.
    if (dd_surface_descriptor.dwFlags & DDSD_LINEARSIZE) {
      size_t expected_size =
          image::ComputeBufferSize(dds_width, dds_height, format);
      if (expected_size != dd_surface_descriptor.dwLinearSize) {
        O3D_ERROR(service_locator) << "Advertised buffer size in \"" << filename
                    << "\" differs from expected size.";
        return false;
      }
    }
    if (is_dxtc) {
      // DirectX says the only valid DXT format base sizes are multiple-of-4.
      // OpenGL doesn't care, but we actually do because we need to flip them.
      // (and we can't flip them if they are not multiple-of-4).
      // This restriction actually exists for mip-map levels as well, so in
      // practice we need power-of-two restriction.
      if ((dds_width & (dds_width - 1)) != 0 ||
          (dds_height & (dds_height - 1)) != 0) {
        O3D_ERROR(service_locator) << "Invalid dimensions in DXTC file \""
                    << filename << "\": must be power-of-two.";
        return false;
      }
    }
  } else if (pixel_format.dwFlags & DDPF_RGB) {
    if (pixel_format.dwFlags & DDPF_ALPHAPIXELS) {
      // Pixel format with alpha. Check that the alpha bits are at the expected
      // place.
      if (pixel_format.dwRGBAlphaBitMask != 0xff000000) {
        O3D_ERROR(service_locator) << "unexpected alpha mask in DDS image format \""
                    << filename << "\"";
        return false;
      }
    } else {
      add_filler_alpha = true;
    }
    // uncompressed bitmap
    // try to determine the format
    if (pixel_format.dwRBitMask == 0x00ff0000 &&
        pixel_format.dwGBitMask == 0x0000ff00 &&
        pixel_format.dwBBitMask == 0x000000ff) {
      // BGR(A) format.
    } else if (pixel_format.dwRBitMask == 0x000000ff &&
               pixel_format.dwGBitMask == 0x0000ff00 &&
               pixel_format.dwBBitMask == 0x00ff0000) {
      // RGB(A) format. Convert to BGR(A).
      rgb_to_bgr = true;
    } else {
      O3D_ERROR(service_locator) << "unknown uncompressed DDS image format \""
                  << filename << "\"";
      return false;
    }
    // components per pixel in the file.
    components_per_pixel = add_filler_alpha ? 3 : 4;
    if (components_per_pixel * 8 != pixel_format.dwRGBBitCount) {
      O3D_ERROR(service_locator) << "unexpected bit count in DDS image format \""
                  << filename << "\"";
      return false;
    }
    format = add_filler_alpha ? Texture::XRGB8 : Texture::ARGB8;
  }

  unsigned int num_bitmaps = is_cubemap ? 6 : 1;
  // Bitmap requires we allocate enough memory for all mips even if we don't use
  // them.
  size_t face_size = Bitmap::ComputeMaxSize(dds_width, dds_height, format);

  BitmapRefArray temp_bitmaps;

  size_t disk_face_size =
      image::ComputeMipChainSize(dds_width, dds_height, format, mip_count);
  if (!is_dxtc) {
    // if reading uncompressed RGB, for example, we shouldn't read alpha channel
    // NOTE: here we assume that RGB data is packed - it may not be true
    // for non-multiple-of-4 widths.
    disk_face_size = components_per_pixel * disk_face_size / 4;
  }

  for (unsigned int face = 0; face < num_bitmaps; ++face) {
    // Allocate and load bitmap data.
    ::o3d::base::scoped_array<uint8_t> image_data(new uint8_t[face_size]);

    char *data = reinterpret_cast<char*>(image_data.get());
    bytes_read = stream->Read(data, disk_face_size);
    if (bytes_read != disk_face_size) {
      O3D_ERROR(service_locator) << "DDS failed to read image data \"" << filename << "\"";
      return false;
    }

    // Do pixel conversions on non-DXT images.
    if (!is_dxtc) {
      O3D_ASSERT(components_per_pixel == 3 || components_per_pixel == 4);
      unsigned int pixel_count = disk_face_size / components_per_pixel;
      // convert to four components per pixel if necessary
      if (add_filler_alpha) {
        O3D_ASSERT(components_per_pixel == 3u);
        image::XYZToXYZA(image_data.get(), pixel_count);
      } else {
        O3D_ASSERT(components_per_pixel == 4u);
      }
      if (rgb_to_bgr) {
        image::RGBAToBGRA(image_data.get(), pixel_count);
      }
    }
    Semantic semantic = is_cubemap ? static_cast<Semantic>(face) : IMAGE;

    Bitmap::Ref bitmap(new Bitmap(service_locator));
    bitmap->SetContents(format, mip_count, dds_width, dds_height, semantic,
                        &image_data);
    temp_bitmaps.push_back(bitmap);
  }

#if defined(O3D_DECOMPRESS_DXT)
  if (!DecompressDDSBitmaps(service_locator, temp_bitmaps)) {
    O3D_ERROR(service_locator) << "Failed to decompress DDS bitmaps";
    return false;
  }
#endif

  // Success.
  bitmaps->insert(bitmaps->end(), temp_bitmaps.begin(), temp_bitmaps.end());
  return true;
}

}  // namespace o3d
