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

// This file contains the image codec operations for DDS files.
#include "core/cross/error.h"
#include "core/cross/bitmap.h"
#include "core/cross/ddsurfacedesc.h"
#include "base/cross/file_util.h"
#include "import/cross/memory_buffer.h"
#include "import/cross/memory_stream.h"
#include <cstring>

namespace o3d {

	namespace {
		struct color565_t {
			uint16_t b: 5;
			uint16_t g: 6;
			uint16_t r: 5;
		} O3D_PACKED;

		struct dxt_block_t {
			union {
				uint32_t   b32;
				uint16_t   b16[2];
				uint8_t    b8[4];
				color565_t c565[2];
			} colors;
			union {
				uint32_t b32;
				uint8_t  b8[4];
			} bits;
		};

		static inline O3D_ALWAYS_INLINE void dxt_decode_block(
		    const uint64_t* restrict src,
		    int i,
		    int j,
		    unsigned int dxt_type,
		    uint8_t* restrict rgba
		) {
			union {
				uint64_t raw;
				dxt_block_t block;
			};
			raw = src[0];

			if(!is_little_endian()) {
				switch_endianness32(block.colors.b32);
				switch_endianness32(block.bits.b32);
				std::swap(block.colors.b16[0], block.colors.b16[1]);
			}

			const uint16_t& color0(block.colors.b16[0]);

			const uint16_t& color1(block.colors.b16[1]);

			const unsigned int& bits(block.bits.b32);

			const uint8_t bit_pos(2 * (j * 4 + i));

			uint8_t code((bits >> bit_pos) & 3);

			const uint8_t alpha(((code == 3) && (dxt_type == 1) && (color0 <= color1)) ? 0 : 255);

			code += (color0 <= color1) ? 0 : 4;

			static const uint16_t N0[] = {1, 0, 1, 0, 1, 0, 2, 1};

			static const uint16_t N1[] = {0, 1, 1, 0, 0, 1, 1, 2};

			// premultiplying by these values before right-shifting by
			// 8 bits is the same as dividing by 1 (256), 2 (128) or 3 (86)
			// (except there is no such thing as an integer divide).
			static const uint16_t M[] = {256, 256, 128, 0, 256, 256, 86, 86};

			const uint16_t& n0(N0[code]);

			const uint16_t& n1(N1[code]);

			const uint16_t& m(M[code]);

			rgba[0] = ((uint32_t(block.colors.c565[0].r) * n0 + uint32_t(block.colors.c565[1].r) * n1) * m) >> 5;

			rgba[1] = ((uint32_t(block.colors.c565[0].g) * n0 + uint32_t(block.colors.c565[1].g) * n1) * m) >> 6;

			rgba[2] = ((uint32_t(block.colors.c565[0].b) * n0 + uint32_t(block.colors.c565[1].b) * n1) * m) >> 5;

			rgba[3] = alpha;
		}

		struct DXT1Helper {
			enum { BLOCK_BYTES = 8 };
			static inline O3D_ALWAYS_INLINE void FetchTexel(
			    int srcRowStride,
			    const uint64_t* restrict pixdata,
			    int i,
			    int j,
			    uint8_t* restrict rgba
			) {
				const uint64_t* restrict blksrc(pixdata + (((srcRowStride + 3) >> 2) * (j >> 2) + (i >> 2)));
				dxt_decode_block(blksrc, (i & 3), (j & 3), 1, rgba);
			}

			// Flips a full DXT1 block in the y direction.
			static inline O3D_ALWAYS_INLINE void FlipFull(uint8_t* restrict block) {
				// A DXT1 block layout is:
				// [0-1] color0.
				// [2-3] color1.
				// [4-7] color bitmap, 2 bits per pixel.
				// So each of the 4-7 bytes represents one line, flipping a block is just
				// flipping those bytes.
				std::swap(block[4], block[7]);
				std::swap(block[5], block[6]);
			}

			// Flips the first 2 lines of a DXT1 block in the y direction.
			static inline O3D_ALWAYS_INLINE void FlipHalf(uint8_t* restrict block) {
				// See layout above.
				std::swap(block[4], block[5]);
			}
		};

		struct DXT3Helper {
			enum { BLOCK_BYTES = 16 };
			static inline O3D_ALWAYS_INLINE void FetchTexel(
			    int srcRowStride,
			    const uint64_t* restrict pixdata,
			    int i,
			    int j,
			    uint8_t* restrict rgba
			) {
				const uint64_t* restrict blksrc(pixdata + ((((srcRowStride + 3) >> 2) * (j >> 2) + (i >> 2)) << 1));
				union {
					uint64_t b64;
					uint8_t  b8[8];
				};
				i = i & 3;
				j = j & 3;
				b64 = blksrc[0];
				const uint8_t anibble((b8[((j << 2) + i) >> 1] >> ((i & 1) << 2)) & 0xf);
				dxt_decode_block(blksrc + 1, i, j, 2, rgba);
				rgba[3] = anibble | (anibble << 4);
			}

			// Flips a full DXT3 block in the y direction.
			static inline O3D_ALWAYS_INLINE void FlipFull(uint8_t* restrict block) {
				// A DXT3 block layout is:
				// [0-7]  alpha bitmap, 4 bits per pixel.
				// [8-15] a DXT1 block.
				// We can flip the alpha bits at the byte level (2 bytes per line).
				std::swap(block[0], block[6]);
				std::swap(block[1], block[7]);
				std::swap(block[2], block[4]);
				std::swap(block[3], block[5]);
				// And flip the DXT1 block
				std::swap(block[12], block[15]);
				std::swap(block[13], block[14]);
			}

			// Flips the first 2 lines of a DXT3 block in the y direction.
			static inline O3D_ALWAYS_INLINE void FlipHalf(uint8_t* restrict block) {
				// See layout above.
				std::swap(block[0], block[2]);
				std::swap(block[1], block[3]);
				std::swap(block[12], block[13]);
			}
		};

		struct DXT5Helper {
			enum { BLOCK_BYTES = 16 };
			static inline O3D_ALWAYS_INLINE void FetchTexel(
			    int srcRowStride,
			    const uint64_t* restrict pixdata,
			    int i,
			    int j,
			    uint8_t* restrict rgba
			) {
				const uint64_t* restrict blksrc(pixdata + ((((srcRowStride + 3) >> 2) * (j >> 2) + (i >> 2)) << 1));
				union {
					uint64_t b64;
					uint8_t  b8[8];
				};
				i = i & 3;
				j = j & 3;
				b64 = blksrc[0];
				const int bit_pos(((j << 2) + i) * 3);
				const int acodelow(b8[2 + (bit_pos >> 3)]);
				const int acodehigh(b8[3 + (bit_pos >> 3)]);
				const int code(((acodelow >> (bit_pos & 0x7) |
				                 (acodehigh  << (8 - (bit_pos & 0x7)))) & 0x7) + (b8[0] <= b8[1]) ? 0 : 8);
				dxt_decode_block(blksrc + 1, i, j, 2, rgba);
				static const int N0[] = { 1, 0, 4, 3, 2, 1, 0, 0, 1, 0, 6, 5, 4, 3, 2, 1 };
				static const int N1[] = { 0, 1, 1, 2, 3, 4, 0, 0, 0, 1, 1, 2, 3, 4, 5, 6 };
				static const int M[] =  { 256, 256, 52, 52, 52, 52, 0, 0, 256, 256, 37, 37, 37, 37, 37, 37 };
				static const int P[] =  { 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0 };
				rgba[3] = P[code] | (((N0[code] * b8[0] + N1[code] * b8[1]) * M[code]) >> 8);
			}

			// Flips a full DXT5 block in the y direction.
			static inline O3D_ALWAYS_INLINE void FlipFull(uint8_t* restrict block) {
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
				// And flip the DXT1 block
				std::swap(block[12], block[15]);
				std::swap(block[13], block[14]);
			}

			// Flips the first 2 lines of a DXT5 block in the y direction.
			static inline O3D_ALWAYS_INLINE void FlipHalf(uint8_t* restrict block) {
				// See layout above.
				unsigned int line_0_1 = block[2] + 256 * (block[3] + 256 * block[4]);
				unsigned int line_1_0 = ((line_0_1 & 0x000fff) << 12) |
				                        ((line_0_1 & 0xfff000) >> 12);
				block[2] = line_1_0 & 0xff;
				block[3] = (line_1_0 & 0xff00) >> 8;
				block[4] = (line_1_0 & 0xff0000) >> 8;
				std::swap(block[12], block[13]);
			}
		};

		template<typename Helper>
		struct DXTHelper {
			static void FlipImage(uint32_t width, uint32_t height, uint32_t levels, Texture::Format format, uint8_t* data) {
				unsigned int mip_width = width;
				unsigned int mip_height = height;

				for(unsigned int i = 0; i < levels; ++i) {
					unsigned int blocks_per_row = (mip_width + 3) >> 2;
					unsigned int blocks_per_col = (mip_height + 3) >> 2;
					unsigned int blocks = blocks_per_row * blocks_per_col;

					if(mip_height == 1) {
						// no flip to do, and we're done.
						break;
					}
					else if(mip_height == 2) {
						// flip the first 2 lines in each block.
						const uint8_t* restrict end(data + blocks_per_row * Helper::BLOCK_BYTES);

						for(uint8_t * restrict p(data); p < end; p += Helper::BLOCK_BYTES) {
							Helper::FlipHalf(p);
						}
					}
					else {
						// flip each block.
						const uint8_t* restrict end(data + blocks * Helper::BLOCK_BYTES);

						for(uint8_t * restrict p(data); p < end; p += Helper::BLOCK_BYTES) {
							Helper::FlipFull(p);
						}

						// swap each block line in the first half of the image with the
						// corresponding one in the second half.
						// note that this is a no-op if mip_height is 4.
						const unsigned int row_longs((Helper::BLOCK_BYTES >> 3) * blocks_per_row);
						uint64_t* restrict line1(reinterpret_cast<uint64_t*>(data));
						uint64_t* restrict line2(reinterpret_cast<uint64_t*>(data + (blocks_per_col - 1) * (row_longs << 3)));

						for(unsigned int y = 0; y < blocks_per_col >> 1; ++y, line1 += row_longs, line2 -= row_longs) {
							for(unsigned int x(0); x < row_longs; ++x)
								std::swap(line1[x], line2[x]);
						}
					}

					// mip levels are contiguous.
					data += Helper::BLOCK_BYTES * blocks;
					mip_width = std::max(1U, mip_width >> 1);
					mip_height = std::max(1U, mip_height >> 1);
				}
			}
		};

// Flips a DXTC image, by flipping and swapping DXTC blocks as appropriate.
		static void FlipDXTCImage(uint32_t width,
		                          uint32_t height,
		                          uint32_t levels,
		                          Texture::Format format,
		                          uint8_t* data) {
			O3D_ASSERT(image::CheckImageDimensions(width, height));
			// Height must be a power-of-two.
			O3D_ASSERT((height & (height - 1)) == 0u);

			switch(format) {
			case Texture::DXT1:
				DXTHelper<DXT1Helper>::FlipImage(width, height, levels, format, data);
				break;
			case Texture::DXT3:
				DXTHelper<DXT3Helper>::FlipImage(width, height, levels, format, data);
				break;
			case Texture::DXT5:
				DXTHelper<DXT5Helper>::FlipImage(width, height, levels, format, data);
				break;
			default:
				O3D_NEVER_REACHED();
				return;
			}
		}

// Flips a BGRA image, by simply swapping pixel rows.
		static void FlipBGRAImage(unsigned int width,
		                          unsigned int height,
		                          unsigned int levels,
		                          Texture::Format format,
		                          uint8_t* data) {
			O3D_ASSERT(image::CheckImageDimensions(width, height));
			O3D_ASSERT(format != Texture::DXT1 && format != Texture::DXT3 &&
			           format != Texture::DXT5);
			size_t pixel_bytes = image::ComputeMipChainSize(1, 1, format, 1);
			// pixel_bytes is always a multiple of 4 (see inside image::ComputeMipChainSize())
			size_t pixel_int32s(pixel_bytes >> 2);
			unsigned int mip_width = width;
			unsigned int mip_height = height;

			for(unsigned int i(0); i < levels; ++i) {
				const unsigned int row_int32s(pixel_int32s * mip_width);
				uint32_t* restrict line1(reinterpret_cast<uint32_t*>(data));
				uint32_t* restrict line2(reinterpret_cast<uint32_t*>(data + (mip_height - 1) * (row_int32s << 2)));

				for(unsigned int y(0); y < mip_height >> 1; ++y, line1 += row_int32s, line2 -= row_int32s) {
					for(unsigned int x(0); x < row_int32s; ++x)
						std::swap(line1[x], line2[x]);
				}

				// mip levels are contiguous.
				data += (row_int32s << 2) * mip_height;
				mip_width = std::max(1U, mip_width >> 1);
				mip_height = std::max(1U, mip_height >> 1);
			}
		}

#if defined(O3D_DECOMPRESS_DXT)

		template<typename Helper>
		static bool DecompressDXTCBitmap(
		    ServiceLocator* service_locator,
		    bool is_cube_map,
		    Bitmap::Ref src_bitmap,
		    Bitmap::Ref dst_bitmap
		) {
			for(unsigned level = 0; level < src_bitmap->num_mipmaps(); ++level) {
				int pitch = src_bitmap->GetMipPitch(level) >> 1;
				uint64_t* data = (uint64_t*)src_bitmap->GetMipData(level);
				int width  = std::max(1U, src_bitmap->width() >> level);
				int height = std::max(1U, src_bitmap->height() >> level);
				::o3d::base::scoped_array<uint32_t> decompressed_data(new uint32_t[width * height]);

				for(int src_y = 0; src_y < height; ++src_y) {
					const int dest_y(is_cube_map ? height - src_y - 1 : src_y);

					for(int x = 0; x < width; ++x) {
						union {
							uint32_t pixel;
							uint8_t  argb8[4];
						} tmp;
						Helper::FetchTexel(pitch, data, x, src_y, tmp.argb8);
						// Swap the red and blue channels
						std::swap(tmp.argb8[0], tmp.argb8[2]);
						decompressed_data.get()[width * dest_y + x] = tmp.pixel;
					}
				}

				dst_bitmap->SetRect(level, 0, 0, width, height,
				                    decompressed_data.get(),
				                    width * sizeof(uint32_t));
			}

			return true;
		}

		static bool DecompressDDSBitmaps(ServiceLocator* service_locator, BitmapRefArray& bitmaps) {
			bool is_cube_map(bitmaps.size() == 6);
			bool ok(true);

			for(uint32_t i(0); i < bitmaps.size(); ++i) {
				Bitmap::Ref src_bitmap(bitmaps[i]);
				Bitmap::Ref dst_bitmap(new Bitmap(service_locator));
				dst_bitmap->Allocate(Texture::ARGB8,
				                     src_bitmap->width(),
				                     src_bitmap->height(),
				                     src_bitmap->num_mipmaps(),
				                     src_bitmap->semantic());
				bitmaps[i] = dst_bitmap;
				Texture::Format src_format(src_bitmap->format());

				switch(src_format) {
				case Texture::DXT1:
					ok = ok && DecompressDXTCBitmap<DXT1Helper>(service_locator, is_cube_map, src_bitmap, dst_bitmap);
					break;
				case Texture::DXT3:
					ok = ok && DecompressDXTCBitmap<DXT3Helper>(service_locator, is_cube_map, src_bitmap, dst_bitmap);
					break;
				case Texture::DXT5:
					ok = ok && DecompressDXTCBitmap<DXT5Helper>(service_locator, is_cube_map, src_bitmap, dst_bitmap);
					break;
				case Texture::XRGB8:
				case Texture::ARGB8:

					// uncompressed bitmap
					for(uint32_t level(0); level < src_bitmap->num_mipmaps(); ++level) {
						const uint32_t pitch(src_bitmap->GetMipPitch(level));
						const uint8_t* restrict data(src_bitmap->GetMipData(level));
						const uint32_t width(std::max(1U, src_bitmap->width() >> level));
						const uint32_t height(std::max(1U, src_bitmap->height() >> level));
						const uint32_t row_width(width << 2);
						const uint32_t decompressed_size(width * height << 2);
						::o3d::base::scoped_array<uint8_t> decompressed_data(new uint8_t[decompressed_size]);

						for(uint32_t src_y(0); src_y < height; ++src_y) {
							int dest_y(is_cube_map ? height - src_y - 1 : src_y);
							memcpy(decompressed_data.get() + row_width * dest_y, data + pitch * src_y, row_width);
						}

						dst_bitmap->SetRect(level, 0, 0, width, height, decompressed_data.get(), row_width);
					}

					break;
				default:
					O3D_ERROR(service_locator) << "Unsupported DDS texture format: 0x" << std::hex << src_format << std::dec;
					return false;
				}
			}

			return ok;
		}
#endif

	} // anonymous namespace

	void Bitmap::FlipVertically() {
		if(format() == Texture::DXT1 ||
		        format() == Texture::DXT3 ||
		        format() == Texture::DXT5) {
			FlipDXTCImage(width(), height(), num_mipmaps(), format(), image_data());
		}
		else {
			FlipBGRAImage(width(), height(), num_mipmaps(), format(), image_data());
		}
	}


// Load the bitmap data as DXTC compressed data from a DDS stream into the
// Bitmap object. This routine only supports compressed DDS formats DXT1,
// DXT3 and DXT5.
	bool Bitmap::LoadFromDDSStream(ServiceLocator* service_locator,
	                               MemoryReadStream* stream,
	                               const std::string& filename,
	                               BitmapRefArray* bitmaps) {
		// Verify the file is a true .dds file
		char magic[4];
		size_t bytes_read = stream->Read(magic, sizeof(magic));

		if(bytes_read != sizeof(magic)) {
			O3D_ERROR(service_locator) << "DDS magic header not read \"" << filename << "\"";
			return false;
		}

		if(std::strncmp(magic, "DDS ", 4) != 0) {
			O3D_ERROR(service_locator) << "DDS magic header not recognized \"" << filename << "\"";
			return false;
		}

		// Get the DirectDraw Surface Descriptor
		DDSURFACEDESC2 dd_surface_descriptor;

		if(!stream->ReadAs<DDSURFACEDESC2>(&dd_surface_descriptor)) {
			O3D_ERROR(service_locator) << "DDS header not read \"" << filename << "\"";
			return false;
		}

		const unsigned int kRequiredFlags =
		    DDSD_CAPS |
		    DDSD_HEIGHT |
		    DDSD_WIDTH |
		    DDSD_PIXELFORMAT;

		if((dd_surface_descriptor.dwFlags & kRequiredFlags) != kRequiredFlags) {
			O3D_ERROR(service_locator) << "Required DDS flags are absent in \"" << filename << "\".";
			return false;
		}

		// NOTE: Add permissible flags as appropriate here when supporting new
		// formats.
		const unsigned int kValidFlags = kRequiredFlags |
		                                 DDSD_MIPMAPCOUNT |
		                                 DDSD_LINEARSIZE;

		if(dd_surface_descriptor.dwFlags & ~kValidFlags) {
			O3D_ERROR(service_locator) << "Invalid DDS flags combination \"" << filename << "\".";
			return false;
		}

		unsigned int mip_count = (dd_surface_descriptor.dwFlags & DDSD_MIPMAPCOUNT) ?
		                         dd_surface_descriptor.dwMipMapCount : 1;
		unsigned int dds_width = dd_surface_descriptor.dwWidth;
		unsigned int dds_height = dd_surface_descriptor.dwHeight;

		if(!image::CheckImageDimensions(dds_width, dds_height)) {
			O3D_ERROR(service_locator) << "Failed to load " << filename
			                           << ": dimensions are too large (" << dds_width
			                           << ", " << dds_height << ").";
			return false;
		}

		if(mip_count > image::ComputeMipMapCount(dds_width, dds_height)) {
			O3D_ERROR(service_locator) << "Failed to load " << filename
			                           << ": mip count " << mip_count
			                           << "is inconsistent with image dimensions ("
			                           <<  dds_width << ", " << dds_height << ").";
			return false;
		}

		// Check for cube maps
		bool is_cubemap =
		    (dd_surface_descriptor.ddsCaps.dwCaps2 & DDSCAPS2_CUBEMAP) != 0;

		// Cube maps should have all the face flags set - otherwise the cube map is
		// incomplete.
		if(is_cubemap) {
			if((dd_surface_descriptor.ddsCaps.dwCaps2 & DDSCAPS2_CUBEMAP_ALLFACES) !=
			        DDSCAPS2_CUBEMAP_ALLFACES) {
				O3D_ERROR(service_locator) << "DDS file \"" << filename
				                           << "\" is a cube map but doesn't have all the faces.";
				return false;
			}

			if(dds_width != dds_height) {
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
		DDPIXELFORMAT& pixel_format = dd_surface_descriptor.ddpfPixelFormat;

		if(pixel_format.dwFlags & DDPF_FOURCC) {
			switch(pixel_format.dwFourCC) {
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
			if(dd_surface_descriptor.dwFlags & DDSD_LINEARSIZE) {
				size_t expected_size =
				    image::ComputeBufferSize(dds_width, dds_height, format);

				if(expected_size != dd_surface_descriptor.dwLinearSize) {
					O3D_ERROR(service_locator) << "Advertised buffer size in \"" << filename
					                           << "\" differs from expected size.";
					return false;
				}
			}

			if(is_dxtc) {
				// DirectX says the only valid DXT format base sizes are multiple-of-4.
				// OpenGL doesn't care, but we actually do because we need to flip them.
				// (and we can't flip them if they are not multiple-of-4).
				// This restriction actually exists for mip-map levels as well, so in
				// practice we need power-of-two restriction.
				if((dds_width & (dds_width - 1)) != 0 ||
				        (dds_height & (dds_height - 1)) != 0) {
					O3D_ERROR(service_locator) << "Invalid dimensions in DXTC file \""
					                           << filename << "\": must be power-of-two.";
					return false;
				}
			}
		}
		else if(pixel_format.dwFlags & DDPF_RGB) {
			if(pixel_format.dwFlags & DDPF_ALPHAPIXELS) {
				// Pixel format with alpha. Check that the alpha bits are at the expected
				// place.
				if(pixel_format.dwRGBAlphaBitMask != 0xff000000) {
					O3D_ERROR(service_locator) << "unexpected alpha mask in DDS image format \""
					                           << filename << "\"";
					return false;
				}
			}
			else {
				add_filler_alpha = true;
			}

			// uncompressed bitmap
			// try to determine the format
			if(pixel_format.dwRBitMask == 0x00ff0000 &&
			        pixel_format.dwGBitMask == 0x0000ff00 &&
			        pixel_format.dwBBitMask == 0x000000ff) {
				// BGR(A) format.
			}
			else if(pixel_format.dwRBitMask == 0x000000ff &&
			        pixel_format.dwGBitMask == 0x0000ff00 &&
			        pixel_format.dwBBitMask == 0x00ff0000) {
				// RGB(A) format. Convert to BGR(A).
				rgb_to_bgr = true;
			}
			else {
				O3D_ERROR(service_locator) << "unknown uncompressed DDS image format \""
				                           << filename << "\"";
				return false;
			}

			// components per pixel in the file.
			components_per_pixel = add_filler_alpha ? 3 : 4;

			if(components_per_pixel * 8 != pixel_format.dwRGBBitCount) {
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
		// face_size is always a multiple of 4
		BitmapRefArray temp_bitmaps;
		size_t disk_face_size =
		    image::ComputeMipChainSize(dds_width, dds_height, format, mip_count);

		if(!is_dxtc) {
			// if reading uncompressed RGB, for example, we shouldn't read alpha channel
			// NOTE: here we assume that RGB data is packed - it may not be true
			// for non-multiple-of-4 widths.
			disk_face_size = (components_per_pixel * disk_face_size) >> 2;
		}

		::o3d::base::scoped_array<uint8_t> image_data(new uint8_t[face_size]);

		for(unsigned int face = 0; face < num_bitmaps; ++face) {
			bytes_read = stream->Read(image_data.get(), disk_face_size);

			if(bytes_read != disk_face_size) {
				O3D_ERROR(service_locator) << "DDS failed to read image data \"" << filename << "\"";
				return false;
			}

			// Do pixel conversions on non-DXT images.
			if(!is_dxtc) {
				O3D_ASSERT(components_per_pixel == 3 || components_per_pixel == 4);

				// convert to four components per pixel if necessary
				if(add_filler_alpha) {
					O3D_ASSERT(components_per_pixel == 3u);
					image::XYZToXYZA(image_data.get(), disk_face_size / 3);
				}
				else {
					O3D_ASSERT(components_per_pixel == 4u);
				}

				if(rgb_to_bgr) {
					image::RGBAToBGRA(image_data.get(), disk_face_size >> 2);
				}
			}

			Semantic semantic = is_cubemap ? static_cast<Semantic>(face) : IMAGE;
			Bitmap::Ref bitmap(new Bitmap(service_locator));
			bitmap->SetContents(format, mip_count, dds_width, dds_height, semantic,
			                    &image_data);
			temp_bitmaps.push_back(bitmap);
		}

		image_data.reset();
#if defined(O3D_DECOMPRESS_DXT)

		if(!DecompressDDSBitmaps(service_locator, temp_bitmaps)) {
			O3D_ERROR(service_locator) << "Failed to decompress DDS bitmaps";
			return false;
		}

#endif
		// Success.
		bitmaps->insert(bitmaps->end(), temp_bitmaps.begin(), temp_bitmaps.end());
		return true;
	}

}  // namespace o3d
