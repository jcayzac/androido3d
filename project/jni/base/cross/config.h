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
#pragma once
#define O3D_LIBRARY_VERSION "0.1.99.1"

// C++ requires these to be defined before including
// stdint.h for minmax/constant macros to be declared
#undef __STDC_LIMIT_MACROS
#undef __STDC_CONSTANT_MACROS
#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS
#include <stdint.h>

#include <stdarg.h>
#include <stddef.h>

// OS detection
#if defined(__APPLE__)
#include <TargetConditionals.h>
#if defined(TARGET_OS_IPHONE)
#define OS_IPHONE 1
#if defined(TARGET_IPHONE_SIMULATOR)
#define SIMULATOR 1
#endif
#else
#define OS_MACOSX 1
#endif
#elif defined(__ANDROID__)
//#define OS_LINUX 1
#define OS_ANDROID 1
#if !defined(__arm__)
#define SIMULATOR 1
#endif
#elif defined(linux) || defined(__linux__)
#define OS_LINUX 1
#elif defined(__FreeBSD__)
#define OS_FREEBSD 1
#elif defined(__OpenBSD__)
#define OS_OPENBSD 1
#elif defined(__CYGWIN__)
#define OS_CYGWIN 1
#else
#error Unsupported platform
#endif

// OS family
#if defined(OS_MACOSX) || defined(OS_FREEBSD) || defined(OS_OPENBSD)
#define OS_BSD 1
#endif
#if defined(OS_BSD) || defined(OS_LINUX) || defined(OS_CYGWIN) || defined(OS_ANDROID)
#define OS_POSIX 1
#endif

// Features
#if defined(__GNUC__)
#if defined(__GXX_RTTI)
#define HAVE_RTTI 1
#endif
#endif

// Attributes
#if defined(__GNUC__)
#define O3D_ALLOW_UNUSED_RESULT   __attribute__((unused))
#define O3D_WARN_UNUSED_RESULT    __attribute__((warn_unused_result))
#define O3D_PRINTF_FORMAT(f, a)   __attribute__((format(printf, f, a)))
#define O3D_DEPRECATED            __attribute__((deprecated))
#define O3D_ALWAYS_INLINE         __attribute__((always_inline))
#define O3D_FLATTEN               __attribute__((flatten))
#define O3D_WARN_IF_USED(x)       __attribute__((warning(x)))
#define O3D_CHECK_NULL_ARGS(x...) __attribute__((nonnull(x)))
#define O3D_NORETURN              __attribute__((noreturn))
#define O3D_PURE                  __attribute__((pure))
#define O3D_HOT                   __attribute__((hot))
#define O3D_COLD                  __attribute__((cold))
#define O3D_PACKED                __attribute__((packed))
#elif defined(__ARM_CC)
#define O3D_ALLOW_UNUSED_RESULT   __attribute__((unused))
#define O3D_WARN_UNUSED_RESULT
#define O3D_PRINTF_FORMAT(x, y)
#define O3D_DEPRECATED            __attribute__((deprecated))
#define O3D_ALWAYS_INLINE         __attribute__((always_inline))
#define O3D_FLATTEN
#define O3D_WARN_IF_USED(x)
#define O3D_CHECK_NULL_ARGS(x...) __attribute__((nonnull(x)))
#define O3D_NORETURN              __attribute__((noreturn))
#define O3D_PURE                  __attribute__((pure))
#define O3D_HOT
#define O3D_COLD
#define O3D_PACKED                __attribute__((packed))
#else
#define O3D_ALLOW_UNUSED_RESULT
#define O3D_WARN_UNUSED_RESULT
#define O3D_PRINTF_FORMAT(x, y)
#define O3D_DEPRECATED
#define O3D_ALWAYS_INLINE
#define O3D_FLATTEN
#define O3D_WARN_IF_USED(x)
#define O3D_CHECK_NULL_ARGS(x...)
#define O3D_NORETURN
#define O3D_PURE
#define O3D_HOT
#define O3D_COLD
#define O3D_PACKED
#endif

#if defined(__cplusplus)
#include <new>
#include <algorithm>

// C99's 'restrict' keyword is not part of std C++
#if defined(__GNUC__) || defined(__ARM_CC)
#define restrict __restrict__
#else
#define restrict
#endif

// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define O3D_DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

// The o3d_arraysize(arr) macro returns the # of elements in an array arr.
// The expression is a compile-time constant, and therefore can be
// used in defining new arrays, for example.  If you use o3d_arraysize on
// a pointer by mistake, you will get a compile-time error.
//
// One caveat is that o3d_arraysize() doesn't accept any array of an
// anonymous type or a type defined inside a function.  In these rare
// cases, you have to use the unsafe O3D_ARRAYSIZE_UNSAFE() macro below.  This is
// due to a limitation in C++'s template system.  The limitation might
// eventually be removed, but it hasn't happened yet.

// This template function declaration is used in defining o3d_arraysize.
// Note that the function doesn't need an implementation, as we only
// use its type.
namespace o3d {
	namespace base {
		template <typename T, size_t N>
		char(&ArraySizeHelper(T(&array)[N]))[N];
	}
}

#define o3d_arraysize(array) (sizeof(o3d::base::ArraySizeHelper(array)))

// O3D_ARRAYSIZE_UNSAFE performs essentially the same calculation as o3d_arraysize,
// but can be used on anonymous types or types defined inside
// functions.  It's less safe than o3d_arraysize as it accepts some
// (although not all) pointers.  Therefore, you should use o3d_arraysize
// whenever possible.
//
// The expression O3D_ARRAYSIZE_UNSAFE(a) is a compile-time constant of type
// size_t.
//
// O3D_ARRAYSIZE_UNSAFE catches a few type errors.  If you see a compiler error
//
//   "warning: division by zero in ..."
//
// when using O3D_ARRAYSIZE_UNSAFE, you are (wrongfully) giving it a pointer.
// You should only use O3D_ARRAYSIZE_UNSAFE on statically allocated arrays.
//
// The following comments are on the implementation details, and can
// be ignored by the users.
//
// O3D_ARRAYSIZE_UNSAFE(arr) works by inspecting sizeof(arr) (the # of bytes in
// the array) and sizeof(*(arr)) (the # of bytes in one array
// element).  If the former is divisible by the latter, perhaps arr is
// indeed an array, in which case the division result is the # of
// elements in the array.  Otherwise, arr cannot possibly be an array,
// and we generate a compiler error to prevent the code from
// compiling.
//
// Since the size of bool is implementation-defined, we need to cast
// !(sizeof(a) & sizeof(*(a))) to size_t in order to ensure the final
// result has type size_t.
//
// This macro is not perfect as it wrongfully accepts certain
// pointers, namely where the pointer size is divisible by the pointee
// size.  Since all our code has to go through a 32-bit compiler,
// where a pointer is 4 bytes, this means all pointers to a type whose
// size is 3 or greater than 4 will be (righteously) rejected.

#define O3D_ARRAYSIZE_UNSAFE(a) \
  ((sizeof(a) / sizeof(*(a))) / \
   static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))

namespace o3d {
// Use implicit_cast as a safe version of static_cast or const_cast
// for upcasting in the type hierarchy (i.e. casting a pointer to Foo
// to a pointer to SuperclassOfFoo or casting a pointer to Foo to
// a const pointer to Foo).
// When you use implicit_cast, the compiler checks that the cast is safe.
// Such explicit implicit_casts are necessary in surprisingly many
// situations where C++ demands an exact type match instead of an
// argument type convertable to a target type.
//
// The From type can be inferred, so the preferred syntax for using
// implicit_cast is the same as for static_cast etc.:
//
//   implicit_cast<ToType>(expr)
//
// implicit_cast would have been part of the C++ standard library,
// but the proposal was submitted too late.  It will probably make
// its way into the language in the future.
	template<typename To, typename From>
	inline To implicit_cast(From const& f) {
		return f;
	}

// When you upcast (that is, cast a pointer from type Foo to type
// SuperclassOfFoo), it's fine to use implicit_cast<>, since upcasts
// always succeed.  When you downcast (that is, cast a pointer from
// type Foo to type SubclassOfFoo), static_cast<> isn't safe, because
// how do you know the pointer is really of type SubclassOfFoo?  It
// could be a bare Foo, or of type DifferentSubclassOfFoo.  Thus, when
// you downcast, you should use this template.
//
// NOTE: We used to do a dynamic_cast in debug mode to make sure it
// was the right type, but now that RTTI is completely turned off, we
// just do the implicit_cast compile-time check.
//
// Use it like this: down_cast<T*>(foo);

	template<typename To, typename From>
	inline To down_cast(From* f) {  // Defined as From* so we only accept pointers.
		// Ensures that To is a sub-type of From *.  This test is here only
		// for compile-time type checking, and has no overhead in an
		// optimized build at run-time, as it will be optimized away
		// completely.
		if(false) {
			implicit_cast<From*, To>(0);
		}

		return static_cast<To>(f);
	}

// Return true if target is little endian
	static inline O3D_ALWAYS_INLINE bool is_little_endian() throw() {
		union {
			uint32_t b32;
			uint8_t  b8[4];
		};
		b32 = 0xdeadbabe;
		return b8[0] == 0xbe;
	}

// Functions to switch endianness
#if defined(__GNUC__) || defined(__ARM_CC)
	static inline O3D_ALWAYS_INLINE uint32_t switch_endianness32(uint32_t x) throw() {
		return __builtin_bswap32(x);
	}
	static inline O3D_ALWAYS_INLINE uint16_t switch_endianness16(uint16_t x) throw() {
		return __builtin_bswap32(x << 16);
	}
#else
// TODO: Test this with non-gcc compilers
	static inline O3D_ALWAYS_INLINE uint16_t switch_endianness16(uint16_t x) throw() {
		return ((x & 0xff) << 8) | (x >> 8)
	}
	static inline O3D_ALWAYS_INLINE uint32_t switch_endianness32(uint32_t x) throw() {
		uint16_t hi(switch_endianness16(x & 0xffff));
		uint16_t lo(switch_endianness16(x >> 16));
		return lo | (hi << 16);
	}
#endif

} // namespace o3d

#endif // __cplusplus

#include "base/cross/userconfig.h"

// Legacy macros that are still needed until we come up with a proper fix

// Decompress .DDS textures
// FIXME: this should go away, and decompression automatically
// turned on if DXT compression is not supported by the driver.
// (but we need a mechanism to query GL extensions, first)
#define O3D_DECOMPRESS_DXT

// Tells the GLES2 renderer to shadow textures so that it can
// restore them later, when the context gets restored.
// FIXME: AFAIK this is required by all GLES2 implementations,
// so why is it a macro anyway?
#define O3D_GLES2_MUST_SHADOW_TEXTURES

// Build without canvas support.
// Canvas is currently not supported by this version of O3D,
// althought it might be a desirable feature in the future.
// FIXME: port current canvas code so it works on Android/iOS too.
#define O3D_NO_CANVAS

// Build without triangulated paths support.
// FIXME: current code depends on GLU and SKIA, so it's currently
// turned off and not supported. Should we support it in a
// future version?
#define O3D_NO_GPU2D
