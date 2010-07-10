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


#include "core/cross/client.h"
#include "core/cross/image_utils.h"
#include "tests/common/win/testing_common.h"
#include "base/file_path.h"
#include "utils/cross/file_path_utils.h"
#include "core/cross/math_utilities.h"

namespace o3d {

namespace {

void ConvertToHalf(const float* src, size_t count, uint16* dst) {
  for (; count != 0; --count) {
    *dst++ = Vectormath::Aos::FloatToHalf(*src++);
  }
}

bool CompareFloats(const float* src_1, const float* src_2, size_t num_floats) {
  const float kEpsilon = 0.000001f;
  for (; num_floats != 0; --num_floats) {
    if (fabs(*src_1 - *src_2) > kEpsilon) {
      return false;
    }
    ++src_1;
    ++src_2;
  }
  return true;
}

bool CompareHalfs(const uint16* src_1, const uint16* src_2, size_t num_floats) {
  const float kEpsilon = 0.001f;
  for (; num_floats != 0; --num_floats) {
    float s1 = Vectormath::Aos::HalfToFloat(*src_1);
    float s2 = Vectormath::Aos::HalfToFloat(*src_2);
    if (fabs(s1 - s2) > kEpsilon) {
      return false;
    }
    ++src_1;
    ++src_2;
  }
  return true;
}

}  // anonymous namespace.

class ImageTest : public testing::Test {
};

TEST_F(ImageTest, GetNumComponentsForFormat) {
  EXPECT_EQ(4u, image::GetNumComponentsForFormat(Texture::XRGB8));
  EXPECT_EQ(4u, image::GetNumComponentsForFormat(Texture::ARGB8));
  EXPECT_EQ(4u, image::GetNumComponentsForFormat(Texture::ABGR16F));
  EXPECT_EQ(4u, image::GetNumComponentsForFormat(Texture::ABGR32F));
  EXPECT_EQ(1u, image::GetNumComponentsForFormat(Texture::R32F));
  EXPECT_EQ(0u, image::GetNumComponentsForFormat(Texture::DXT1));
  EXPECT_EQ(0u, image::GetNumComponentsForFormat(Texture::DXT3));
  EXPECT_EQ(0u, image::GetNumComponentsForFormat(Texture::DXT5));
}

TEST_F(ImageTest, IsPOT) {
  EXPECT_TRUE(image::IsPOT(2, 2));
  EXPECT_FALSE(image::IsPOT(3, 2));
  EXPECT_FALSE(image::IsPOT(2, 3));
}

TEST_F(ImageTest, CheckImageDimensions) {
  EXPECT_TRUE(image::CheckImageDimensions(1u, 1u));
  EXPECT_TRUE(image::CheckImageDimensions(image::kMaxImageDimension,
                                          image::kMaxImageDimension));
  EXPECT_FALSE(image::CheckImageDimensions(0u, image::kMaxImageDimension + 1));
  EXPECT_FALSE(image::CheckImageDimensions(image::kMaxImageDimension + 1, 0u));
}

TEST_F(ImageTest, ComputeMipMapCount) {
  EXPECT_EQ(image::ComputeMipMapCount(1, 1), 1u);
  EXPECT_EQ(image::ComputeMipMapCount(2, 2), 2u);
  EXPECT_EQ(image::ComputeMipMapCount(2, 1), 2u);
  EXPECT_EQ(image::ComputeMipMapCount(256, 1), 9u);
  EXPECT_EQ(image::ComputeMipMapCount(256, 256), 9u);
}

TEST_F(ImageTest, ComputePOTSize) {
  EXPECT_EQ(image::ComputePOTSize(1), 1u);
  EXPECT_EQ(image::ComputePOTSize(2), 2u);
  EXPECT_EQ(image::ComputePOTSize(3), 4u);
  EXPECT_EQ(image::ComputePOTSize(4), 4u);
  EXPECT_EQ(image::ComputePOTSize(5), 8u);
  EXPECT_EQ(image::ComputePOTSize(7), 8u);
  EXPECT_EQ(image::ComputePOTSize(8), 8u);
  EXPECT_EQ(image::ComputePOTSize(9), 16u);
  EXPECT_EQ(image::ComputePOTSize(128), 128u);
  EXPECT_EQ(image::ComputePOTSize(129), 256u);
  EXPECT_EQ(image::ComputePOTSize(255), 256u);
  EXPECT_EQ(image::ComputePOTSize(4096), 4096u);
}

TEST_F(ImageTest, CanMakeMips) {
  EXPECT_TRUE(image::CanMakeMips(Texture::XRGB8));
  EXPECT_TRUE(image::CanMakeMips(Texture::ARGB8));
  EXPECT_TRUE(image::CanMakeMips(Texture::ABGR16F));
  EXPECT_TRUE(image::CanMakeMips(Texture::R32F));
  EXPECT_TRUE(image::CanMakeMips(Texture::ABGR32F));
  EXPECT_FALSE(image::CanMakeMips(Texture::DXT1));
  EXPECT_FALSE(image::CanMakeMips(Texture::DXT3));
  EXPECT_FALSE(image::CanMakeMips(Texture::DXT5));
};

static const uint8 kScaleUPDataNPOT[] = {
  // This is a 3x3 image.
  0x75, 0x58, 0x7b, 0x76,
  0x8a, 0x54, 0x85, 0x6f,
  0x93, 0x56, 0x74, 0x7d,

  0x3f, 0x58, 0x7a, 0x64,
  0x7a, 0x90, 0x75, 0x8f,
  0xb4, 0x7c, 0x71, 0x6b,

  0x84, 0x84, 0x85, 0x6c,
  0xb1, 0x73, 0x4f, 0x7c,
  0x97, 0x87, 0x78, 0xa2,
};

static const uint8 kScaleUPDataPOT[] = {
  // This is the 4x4 scaled-up version of the above.
  0x75, 0x58, 0x7b, 0x76,
  0x8a, 0x54, 0x85, 0x6f,
  0x8a, 0x54, 0x85, 0x6f,
  0x93, 0x56, 0x74, 0x7d,

  0x3f, 0x58, 0x7a, 0x64,
  0x7a, 0x90, 0x75, 0x8f,
  0x7a, 0x90, 0x75, 0x8f,
  0xb4, 0x7c, 0x71, 0x6b,

  0x3f, 0x58, 0x7a, 0x64,
  0x7a, 0x90, 0x75, 0x8f,
  0x7a, 0x90, 0x75, 0x8f,
  0xb4, 0x7c, 0x71, 0x6b,

  0x84, 0x84, 0x85, 0x6c,
  0xb1, 0x73, 0x4f, 0x7c,
  0xb1, 0x73, 0x4f, 0x7c,
  0x97, 0x87, 0x78, 0xa2,
};

// Scales up a NPOT texture, compare with expected results.
TEST_F(ImageTest, ScaleUpToPOT) {
  const unsigned int kWidth = 3;
  const unsigned int kHeight = 3;
  const unsigned int kPOTWidth = image::ComputePOTSize(kWidth);
  ASSERT_EQ(kPOTWidth, 4u);
  const unsigned int kPOTHeight = image::ComputePOTSize(kHeight);
  ASSERT_EQ(kPOTHeight, 4u);
  const Texture::Format format = Texture::ARGB8;
  size_t src_size = image::ComputeBufferSize(kWidth, kHeight, format);
  ASSERT_EQ(sizeof(kScaleUPDataNPOT), src_size);
  size_t dst_size = image::ComputeBufferSize(kPOTWidth, kPOTHeight, format);
  ASSERT_EQ(sizeof(kScaleUPDataPOT), dst_size);
  scoped_array<uint8> data(new uint8[dst_size]);
  ASSERT_TRUE(data.get() != NULL);
  // Check that scaling works when source and destination don't alias
  image::ScaleUpToPOT(kWidth, kHeight, format, kScaleUPDataNPOT, data.get(),
                      4 * 4);
  EXPECT_EQ(0, memcmp(data.get(), kScaleUPDataPOT, dst_size));

  // Check that scaling works when source and destination do alias
  memset(data.get(), 0, dst_size);
  memcpy(data.get(), kScaleUPDataNPOT, src_size);
  image::ScaleUpToPOT(kWidth, kHeight, format, data.get(), data.get(), 4 * 4);
  EXPECT_EQ(0, memcmp(data.get(), kScaleUPDataPOT, dst_size));
}

// NOTE: untested ffile types are:
//    png grayscale textures
//    dds cube maps
//    dds mipmapped cube maps
//    dds 1D textures
//    dds 3D textures


static const uint8 kMipmapDataPOT[] = {
  // This is a 4x4 image
  0x7D, 0xE4, 0x0F, 0xff, 0x71, 0x7B, 0x9C, 0xff,
  0xDD, 0xF0, 0x9D, 0xff, 0xFA, 0x08, 0x49, 0xff,
  0xEA, 0x28, 0xF6, 0xff, 0x73, 0x10, 0x64, 0xff,
  0x8B, 0x36, 0x58, 0xff, 0x7A, 0x3E, 0x21, 0xff,
  0x64, 0xCE, 0xB1, 0xff, 0x36, 0x4D, 0xC5, 0xff,
  0xF3, 0x99, 0x7E, 0xff, 0x5C, 0x56, 0x1E, 0xff,
  0x59, 0x8C, 0x41, 0xff, 0x39, 0x24, 0x1B, 0xff,
  0x5D, 0x4D, 0x96, 0xff, 0x5E, 0xF8, 0x8B, 0xff,
  // Followed by its 2x2 mip level
  0x92, 0x65, 0x81, 0xff, 0xb7, 0x5b, 0x57, 0xff,
  0x4b, 0x72, 0x74, 0xff, 0x82, 0x8d, 0x6f, 0xff,
  // Followed by its 1x1 mip level
  0x85, 0x6f, 0x6e, 0xff,
};

// Generates mip-maps from a known power-of-two image, compare with expected
// results.
TEST_F(ImageTest, GenerateMipmapsPOTUInt8) {
  const unsigned int kWidth = 4;
  const unsigned int kHeight = 4;
  const Texture::Format format = Texture::ARGB8;
  unsigned int mipmaps = image::ComputeMipMapCount(kWidth, kHeight);
  EXPECT_EQ(3u, mipmaps);
  size_t size = image::ComputeMipChainSize(kWidth, kHeight, format, mipmaps);
  ASSERT_EQ(sizeof(kMipmapDataPOT), size);
  scoped_array<uint8> data(new uint8[size]);
  ASSERT_TRUE(data.get() != NULL);
  // Copy first level into the buffer.
  size_t base_size = image::ComputeMipChainSize(kWidth, kHeight, format, 1);
  memcpy(data.get(), kMipmapDataPOT, base_size);
  image::GenerateMipmap(
      kWidth, kHeight, format,
      data.get(), image::ComputeMipPitch(format, 0, kWidth),
      data.get() + image::ComputeMipChainSize(kWidth, kHeight, format, 1),
      image::ComputeMipPitch(format, 1, kWidth));
  image::GenerateMipmap(
      image::ComputeMipDimension(1, kWidth),
      image::ComputeMipDimension(1, kHeight),
      format,
      data.get() + image::ComputeMipChainSize(kWidth, kHeight, format, 1),
      image::ComputeMipPitch(format, 1, kWidth),
      data.get() + image::ComputeMipChainSize(kWidth, kHeight, format, 2),
      image::ComputeMipPitch(format, 2, kWidth));
  // Check the result.
  EXPECT_EQ(0, memcmp(data.get(), kMipmapDataPOT, size));
}

TEST_F(ImageTest, GenerateMipmapsPOTFloat) {
  static const float original[] = { 0.0f, 2.0f, 3.0f, 5.0f, };
  static const float expected_mip1[] = { 1.0f, 4.0f, };
  static const float expected_mip2[] = { 2.5f, };

  const unsigned int kWidth = 4;
  const unsigned int kHeight = 1;
  const Texture::Format kFormat = Texture::R32F;
  unsigned int mipmaps = image::ComputeMipMapCount(kWidth, kHeight);
  EXPECT_EQ(3u, mipmaps);
  float mip1[2 + 1];
  float mip2[1 + 1];
  // Put sentinels at the ends
  const float kSentinel = 123.12345f;
  mip1[2] = kSentinel;
  mip2[1] = kSentinel;
  image::GenerateMipmap(
      kWidth, kHeight, kFormat,
      original, image::ComputeMipPitch(kFormat, 0, kWidth),
      mip1,
      image::ComputeMipPitch(kFormat, 1, kWidth));
  image::GenerateMipmap(
      image::ComputeMipDimension(1, kWidth),
      image::ComputeMipDimension(1, kHeight),
      kFormat,
      mip1,
      image::ComputeMipPitch(kFormat, 1, kWidth),
      mip2,
      image::ComputeMipPitch(kFormat, 2, kWidth));
  // Check the result.
  EXPECT_TRUE(CompareFloats(mip1, expected_mip1, arraysize(expected_mip1)));
  EXPECT_TRUE(CompareFloats(mip2, expected_mip2, arraysize(expected_mip2)));
  EXPECT_EQ(mip1[2], kSentinel);
  EXPECT_EQ(mip2[1], kSentinel);
}

TEST_F(ImageTest, GenerateMipmapsPOTHalf) {
  static const float original_f[] = {
    0.0f, 0.0f, 0.0f, 0.0f,
    2.0f, 2.0f, 2.0f, 2.0f,
    3.0f, 3.0f, 3.0f, 3.0f,
    5.0f, 5.0f, 5.0f, 5.0f,
  };
  static const float expected_mip1_f[] = {
    1.0f, 1.0f, 1.0f, 1.0f,
    4.0f, 4.0f, 4.0f, 4.0f,
  };
  static const float expected_mip2_f[] = {
    2.5f, 2.5f, 2.5f, 2.5f,
  };

  uint16 original[arraysize(original_f)];
  uint16 expected_mip1[arraysize(expected_mip1_f)];
  uint16 expected_mip2[arraysize(expected_mip2_f)];

  ConvertToHalf(original_f, arraysize(original_f), original);
  ConvertToHalf(expected_mip1_f, arraysize(expected_mip1_f), expected_mip1);
  ConvertToHalf(expected_mip2_f, arraysize(expected_mip2_f), expected_mip2);

  const unsigned int kWidth = 4;
  const unsigned int kHeight = 1;
  const Texture::Format kFormat = Texture::ABGR16F;
  unsigned int mipmaps = image::ComputeMipMapCount(kWidth, kHeight);
  EXPECT_EQ(3u, mipmaps);
  uint16 mip1[2 * 4 + 1];
  uint16 mip2[1 * 4 + 1];
  // Put sentinels at the ends
  const float kSentinel = 123.12345f;
  uint16 sentinel = Vectormath::Aos::FloatToHalf(kSentinel);
  mip1[2 * 4] = sentinel;
  mip2[1 * 4] = sentinel;
  image::GenerateMipmap(
      kWidth, kHeight, kFormat,
      original, image::ComputeMipPitch(kFormat, 0, kWidth),
      mip1,
      image::ComputeMipPitch(kFormat, 1, kWidth));
  image::GenerateMipmap(
      image::ComputeMipDimension(1, kWidth),
      image::ComputeMipDimension(1, kHeight),
      kFormat,
      mip1,
      image::ComputeMipPitch(kFormat, 1, kWidth),
      mip2,
      image::ComputeMipPitch(kFormat, 2, kWidth));
  // Check the result.
  EXPECT_TRUE(CompareHalfs(mip1, expected_mip1, arraysize(expected_mip1)));
  EXPECT_TRUE(CompareHalfs(mip2, expected_mip2, arraysize(expected_mip2)));
  EXPECT_EQ(mip1[2 * 4], sentinel);
  EXPECT_EQ(mip2[1 * 4], sentinel);
}

static const uint8 kMipmapDataNPOT[] = {
  // This is a 7x7 image
  0x0d, 0x16, 0x68, 0x1b, 0xe6, 0x09, 0x89, 0x55,
  0xda, 0x28, 0x56, 0x55, 0x3e, 0x00, 0x6f, 0x16,
  0x98, 0x11, 0x50, 0x72, 0xe7, 0x17, 0x24, 0xca,
  0x05, 0xe9, 0x92, 0x43, 0xd6, 0xc4, 0x57, 0xcd,
  0x34, 0x9b, 0x86, 0xcf, 0x50, 0x65, 0xc4, 0x83,
  0xaf, 0xa3, 0xaa, 0xe3, 0x7c, 0xab, 0x5f, 0x08,
  0xc1, 0x2e, 0xd1, 0xe9, 0xa8, 0x2b, 0x56, 0x64,
  0x12, 0x74, 0x92, 0x56, 0x30, 0x16, 0xa0, 0x03,
  0x5a, 0x3a, 0x88, 0xb9, 0xe8, 0xa3, 0xfd, 0xf6,
  0xa1, 0x3b, 0x7b, 0x2d, 0xfd, 0x71, 0xc0, 0x0b,
  0x22, 0x31, 0x41, 0x5a, 0x45, 0x6f, 0x6b, 0x1b,
  0x10, 0x5a, 0x16, 0x6e, 0x02, 0x89, 0x12, 0xb1,
  0x67, 0xfc, 0x43, 0x78, 0xc0, 0x55, 0x59, 0xa3,
  0xf8, 0xe2, 0x6b, 0xb7, 0xad, 0x5f, 0x3c, 0x14,
  0xe1, 0x0e, 0x84, 0x89, 0x25, 0xa7, 0xea, 0xc6,
  0x63, 0x20, 0xf9, 0x84, 0xa1, 0xcd, 0x62, 0x0f,
  0x22, 0xab, 0x59, 0xde, 0xbd, 0xfa, 0xab, 0x4d,
  0xca, 0x07, 0x85, 0xdf, 0x83, 0x23, 0x80, 0x8b,
  0x5e, 0xe4, 0x57, 0x45, 0x81, 0x34, 0x52, 0x65,
  0xf0, 0x14, 0x32, 0x33, 0xfe, 0xe4, 0x31, 0x90,
  0x15, 0x51, 0x57, 0x89, 0xed, 0xcf, 0x88, 0xc9,
  0x7b, 0xbb, 0xc6, 0x41, 0xd5, 0x93, 0x7c, 0x65,
  0x39, 0x80, 0x20, 0xa2, 0xe5, 0xca, 0x9b, 0x7e,
  0xb2, 0x1f, 0x0d, 0xdc, 0x5c, 0xab, 0x6b, 0x5b,
  0xc5, 0x57, 0xc0, 0xd2,
  // Followed by its 3x3 mip level
  0x75, 0x58, 0x7b, 0x76, 0x8a, 0x54, 0x85, 0x6f,
  0x93, 0x56, 0x74, 0x7d, 0x3f, 0x58, 0x7a, 0x64,
  0x7a, 0x90, 0x75, 0x8f, 0xb4, 0x7c, 0x71, 0x6b,
  0x84, 0x84, 0x85, 0x6c, 0xb1, 0x73, 0x4f, 0x7c,
  0x97, 0x87, 0x78, 0xa2,
  // Followed by its 1x1 mip level
  0x88, 0x6e, 0x75, 0x7a,
};

// Generates mip-maps from a known non-power-of-two image, compare with expected
// results.
TEST_F(ImageTest, GenerateMipmapsNPOT) {
  const unsigned int kWidth = 7;
  const unsigned int kHeight = 7;
  const Texture::Format format = Texture::ARGB8;
  unsigned int mipmaps = image::ComputeMipMapCount(kWidth, kHeight);
  EXPECT_EQ(3u, mipmaps);
  size_t size = image::ComputeMipChainSize(kWidth, kHeight, format, mipmaps);
  ASSERT_EQ(sizeof(kMipmapDataNPOT), size);
  scoped_array<uint8> data(new uint8[size]);
  ASSERT_TRUE(data.get() != NULL);
  // Copy first level into the buffer.
  size_t base_size = image::ComputeMipChainSize(kWidth, kHeight, format, 1);
  memcpy(data.get(), kMipmapDataNPOT, base_size);
  image::GenerateMipmap(
      kWidth, kHeight, format,
      data.get(), image::ComputeMipPitch(format, 0, kWidth),
      data.get() + image::ComputeMipChainSize(kWidth, kHeight, format, 1),
      image::ComputeMipPitch(format, 1, kWidth));
  image::GenerateMipmap(
      image::ComputeMipDimension(1, kWidth),
      image::ComputeMipDimension(1, kHeight),
      format,
      data.get() + image::ComputeMipChainSize(kWidth, kHeight, format, 1),
      image::ComputeMipPitch(format, 1, kWidth),
      data.get() + image::ComputeMipChainSize(kWidth, kHeight, format, 2),
      image::ComputeMipPitch(format, 2, kWidth));
  // Check the result.
  EXPECT_EQ(0, memcmp(data.get(), kMipmapDataNPOT, size));
}

// Checks that filenames are detected as the correct type.
TEST_F(ImageTest, GetFileTypeFromFilename) {
  EXPECT_EQ(image::TGA, image::GetFileTypeFromFilename("foo.tga"));
  EXPECT_EQ(image::TGA, image::GetFileTypeFromFilename("BAR.TGA"));
  EXPECT_EQ(image::PNG, image::GetFileTypeFromFilename("foo.png"));
  EXPECT_EQ(image::PNG, image::GetFileTypeFromFilename("BAR.PNG"));
  EXPECT_EQ(image::JPEG, image::GetFileTypeFromFilename("foo.jpeg"));
  EXPECT_EQ(image::JPEG, image::GetFileTypeFromFilename("BAR.JPEG"));
  EXPECT_EQ(image::JPEG, image::GetFileTypeFromFilename("foo.jpg"));
  EXPECT_EQ(image::JPEG, image::GetFileTypeFromFilename("BAR.JPG"));
  EXPECT_EQ(image::JPEG, image::GetFileTypeFromFilename("foo.jpe"));
  EXPECT_EQ(image::JPEG, image::GetFileTypeFromFilename("BAR.JPE"));
  EXPECT_EQ(image::DDS, image::GetFileTypeFromFilename("foo.dds"));
  EXPECT_EQ(image::DDS, image::GetFileTypeFromFilename("BAR.DDS"));
  EXPECT_EQ(image::UNKNOWN, image::GetFileTypeFromFilename("foo.bar"));
  EXPECT_EQ(image::UNKNOWN, image::GetFileTypeFromFilename("FOO.BAR"));
}

// Checks that mime types are detected as the correct type.
TEST_F(ImageTest, GetFileTypeFromMimeType) {
  EXPECT_EQ(image::PNG, image::GetFileTypeFromMimeType("image/png"));
  EXPECT_EQ(image::JPEG, image::GetFileTypeFromMimeType("image/jpeg"));
  EXPECT_EQ(image::UNKNOWN, image::GetFileTypeFromFilename("text/plain"));
  EXPECT_EQ(image::UNKNOWN,
            image::GetFileTypeFromFilename("application/x-123"));
}

TEST_F(ImageTest, LanczosScaleFloat) {
  static const float original[] = { 0.0f, 2.0f, 3.0f, 5.0f, };
  static const float expected_mip1[] = { 0.84352076f, 4.1564794f, };
  static const float expected_mip2[] = { 2.5f, };

  const unsigned int kWidth = 4;
  const unsigned int kHeight = 1;
  const Texture::Format kFormat = Texture::R32F;
  unsigned int mipmaps = image::ComputeMipMapCount(kWidth, kHeight);
  EXPECT_EQ(3u, mipmaps);
  float mip1[2 + 1];
  float mip2[1 + 1];
  // Put sentinels at the ends
  const float kSentinel = 123.12345f;
  mip1[2] = kSentinel;
  mip2[1] = kSentinel;
  image::LanczosScale(
      kFormat,
      original, image::ComputeMipPitch(kFormat, 0, kWidth),
      0, 0, 4, 1,
      mip1, image::ComputeMipPitch(kFormat, 1, kWidth),
      0, 0, 2, 1,
      1);
  image::LanczosScale(
      kFormat,
      mip1,
      image::ComputeMipPitch(kFormat, 1, kWidth),
      0, 0, 2, 1,
      mip2, image::ComputeMipPitch(kFormat, 2, kWidth),
      0, 0, 1, 1,
      1);
  // Check the result.
  EXPECT_TRUE(CompareFloats(mip1, expected_mip1, arraysize(expected_mip1)));
  EXPECT_TRUE(CompareFloats(mip2, expected_mip2, arraysize(expected_mip2)));
  EXPECT_EQ(mip1[2], kSentinel);
  EXPECT_EQ(mip2[1], kSentinel);
}

TEST_F(ImageTest, LanczosScaleHalf) {
  static const uint16 original[] = {
    0x0000, 0x0000, 0x0000, 0x0000,
    0x4000, 0x4000, 0x4000, 0x4000,
    0x4200, 0x4200, 0x4200, 0x4200,
    0x4500, 0x4500, 0x4500, 0x4500,
  };
  static const uint16 expected_mip1[] = {
    0x3abf, 0x3abf, 0x3abf, 0x3abf,
    0x4428, 0x4428, 0x4428, 0x4428,
  };
  static const uint16 expected_mip2[] = {
    0x40ff, 0x40ff, 0x40ff, 0x40ff,
  };

  const unsigned int kWidth = 4;
  const unsigned int kHeight = 1;
  const Texture::Format kFormat = Texture::ABGR16F;
  unsigned int mipmaps = image::ComputeMipMapCount(kWidth, kHeight);
  EXPECT_EQ(3u, mipmaps);
  uint16 mip1[2 * 4 + 1];
  uint16 mip2[1 * 4 + 1];
  // Put sentinels at the ends
  const float kSentinel = 123.12345f;
  uint16 sentinel = Vectormath::Aos::FloatToHalf(kSentinel);
  mip1[2 * 4] = sentinel;
  mip2[1 * 4] = sentinel;
  image::LanczosScale(
      kFormat,
      original, image::ComputeMipPitch(kFormat, 0, kWidth),
      0, 0, 4, 1,
      mip1, image::ComputeMipPitch(kFormat, 1, kWidth),
      0, 0, 2, 1,
      4);
  image::LanczosScale(
      kFormat,
      mip1,
      image::ComputeMipPitch(kFormat, 1, kWidth),
      0, 0, 2, 1,
      mip2, image::ComputeMipPitch(kFormat, 2, kWidth),
      0, 0, 1, 1,
      4);
  // Check the result.
  EXPECT_TRUE(CompareHalfs(mip1, expected_mip1, arraysize(expected_mip1)));
  EXPECT_TRUE(CompareHalfs(mip2, expected_mip2, arraysize(expected_mip2)));
  EXPECT_EQ(mip1[2 * 4], sentinel);
  EXPECT_EQ(mip2[1 * 4], sentinel);
}

TEST_F(ImageTest, AdjustForSetRect) {
  int src_y = 1;
  int src_pitch = 2;
  int dst_y = 3;
  int dst_height = 10;
  // Different widths
  EXPECT_FALSE(image::AdjustForSetRect(&src_y, 10, 10, &src_pitch,
                                       &dst_y, 11, &dst_height));
  // Different heights
  EXPECT_FALSE(image::AdjustForSetRect(&src_y, 10, 11, &src_pitch,
                                       &dst_y, 10, &dst_height));
  // width < 0
  EXPECT_FALSE(image::AdjustForSetRect(&src_y, -10, 10, &src_pitch,
                                       &dst_y, -10, &dst_height));
  // SH > 0, DH > 0
  EXPECT_TRUE(image::AdjustForSetRect(&src_y, 10, 10, &src_pitch,
                                      &dst_y, 10, &dst_height));
  EXPECT_EQ(1, src_y);
  EXPECT_EQ(2, src_pitch);
  EXPECT_EQ(3, dst_y);
  EXPECT_EQ(10, dst_height);
  // SH > 0, DH < 0
  dst_y = 9;
  dst_height = -10;
  EXPECT_TRUE(image::AdjustForSetRect(&src_y, 10, 10, &src_pitch,
                                      &dst_y, 10, &dst_height));
  EXPECT_EQ(10, src_y);
  EXPECT_EQ(-2, src_pitch);
  EXPECT_EQ(0, dst_y);
  EXPECT_EQ(10, dst_height);
  // SH < 0, DH < 0
  src_y = 10;
  src_pitch = 2;
  dst_y = 15;
  dst_height = -10;
  EXPECT_TRUE(image::AdjustForSetRect(&src_y, 10, -10, &src_pitch,
                                      &dst_y, 10, &dst_height));
  EXPECT_EQ(1, src_y);
  EXPECT_EQ(2, src_pitch);
  EXPECT_EQ(6, dst_y);
  EXPECT_EQ(10, dst_height);
  // SH < 0, DH > 0
  src_y = 10;
  src_pitch = 2;
  dst_y = 3;
  dst_height = 10;
  EXPECT_TRUE(image::AdjustForSetRect(&src_y, 10, -10, &src_pitch,
                                      &dst_y, 10, &dst_height));
  EXPECT_EQ(10, src_y);
  EXPECT_EQ(-2, src_pitch);
  EXPECT_EQ(3, dst_y);
  EXPECT_EQ(10, dst_height);
}

}  // namespace


