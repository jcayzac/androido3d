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

#include "base/cross/config.h"
#include "base/cross/log.h"
#include "vectormath/scalar/cpp/vectormath_aos.h"
#include <algorithm>
#include <cmath>
#include <tr1/functional>

namespace o3d {
namespace vmath = ::Vectormath::Aos;

typedef uint32_t Id;
typedef vmath::Vector3 Vector3;
typedef vmath::Vector4 Vector4;
typedef vmath::Point3 Point3;
typedef vmath::Quat Quaternion;
typedef vmath::Matrix3 Matrix3;
typedef vmath::Matrix4 Matrix4;
typedef vmath::Transform3 Transform3;
typedef vmath::Quat Quat;
}  // namespace o3d


namespace std {
// Fuzzy implementations of std::equal_to and std::not_equal_to,
// to compare floats in a safer way than just using the == and !=
// C operators.
#if !defined(O3D_NO_FUZZY_STD_EQUALITY_SPECIALIZATION_FOR_FLOATS)
template<>
struct not_equal_to<float>: binary_function<float, float, bool> {
  bool operator()(const float& a, const float& b) const {
    struct float_bits {
      float_bits(float v): value(v) { } 
      union { float value; int bits; };
    } ua(a), ub(b);
    const int x(ua.bits-ub.bits);
    return (x&0x7ffffffc) || ((x&0x80000000) && ua.bits&0x7fffffff);
  }
};

template<>
struct equal_to<float>: binary_function<float, float, bool> {
  bool operator()(const float& a, const float& b) const {
    return !not_equal_to<float>()(a,b);
  }
};
#endif


// Fuzzy implementations of std::equal_to and std::not_equal_to,
// to compare o3d::Matrix4 objects.
// Together with the std::tr1::hash<> specialization below, it allows
// the use of standard associative containers with Matrix4 keys.
template<>
struct equal_to<o3d::Matrix4>: binary_function<o3d::Matrix4, o3d::Matrix4, bool> {
  bool operator()(const o3d::Matrix4& a, const o3d::Matrix4& b) const {
    static const float tolerance(1e-10f);
    bool res(true);
    for (size_t i(0); i<4; ++i)
      res = res && (dot(a[i]-b[i],a[i]-b[i]) < tolerance);
    return res;
  }
};

template<>
struct not_equal_to<o3d::Matrix4>: binary_function<o3d::Matrix4, o3d::Matrix4, bool> {
  bool operator()(const o3d::Matrix4& a, const o3d::Matrix4& b) const {
    return !equal_to<o3d::Matrix4>()(a, b);
  }
};


// TR1 additions
namespace tr1 {
// Specialize std::tr1::hash<> for o3d::Matrix4.
// Together with the std::equal_to<> specialization above, it allows
// the use of standard associative containers with Matrix4 keys.
template<>
struct hash<o3d::Matrix4> {
  static inline unsigned int compute_hash_value(const unsigned int& v, const unsigned int& s) {
    return v + 0x9e3779b9u + (s<<6) + (s>>2);
  }
  size_t operator()(const o3d::Matrix4& m) const {
    static const float epsilon(1e-05f);
    unsigned int res(0);
    for (size_t i(0); i<16; ++i) {
      union { float value; unsigned int bits; } x;
      x.value = m[i>>2][i&3];
      // if value is negligible, make it zero
      if (std::abs(x.value) < epsilon) x.bits=0;
      else switch(fpclassify(x.value)) {
        case FP_ZERO:      x.bits=0; break;
        case FP_INFINITE:  x.bits=(unsigned int)(x.value>0?-1:-2); break;
        case FP_NAN:       x.bits=(unsigned int)(-3); break;
        // Right shift to remove some mantissa precision
        // (thus making the hash a bit fuzzy, to cope with ieee-754
        // equality checking problems)
        default: x.bits=x.bits>>5; break;
      }
      res = compute_hash_value(x.bits, res);
    }
    return (size_t) res;
  }
};
} // namespace tr1
} // namespace std

namespace Vectormath {
namespace Aos {

inline bool operator==(const Matrix4& a, const Matrix4& b) {
  return ::std::equal_to<Matrix4>()(a,b);
}

inline bool operator!=(const Matrix4& a, const Matrix4& b) {
  return ::std::not_equal_to<Matrix4>()(a,b);
}


} // namespace Aos
} // namespace Vectormath
