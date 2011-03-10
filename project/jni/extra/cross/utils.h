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
#include <core/cross/types.h>
#include <core/cross/object_base.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>
#include <third_party/loggingshim/base/logging.h>
#include <algorithm>
#include <functional>
#if defined (STLPORT)
  #include <unordered_map>
#else
  #include <tr1/unordered_map>
#endif

// These two macros are used in various places
// in this file, and get undefined at the end of it
#define EPSILON 1e-05f
#define SQUARE_EPSILON (EPSILON*EPSILON)

// A quick hash function
static inline unsigned int compute_hash_value(const unsigned int& v, const unsigned int& s) {
  return v + 0x9e3779b9u + (s<<6) + (s>>2);
}

// Standard additions
namespace std {
// Specialize std::hash<> for o3d::Matrix4
template<>
struct hash<o3d::Matrix4> {
  size_t operator()(const o3d::Matrix4& m) const {
    unsigned int res(0);
    for (size_t i(0); i<16; ++i) {
      union { float value; unsigned int bits; } x;
      x.value = m[i>>2][i&3];
      // if value is negligible, make it zero
      if (std::abs(x.value) < EPSILON) x.bits=0;
      else switch(fpclassify(x.value)) {
        case FP_ZERO:      x.bits=0; break;
        case FP_INFINITE:  x.bits=(unsigned int)(x.value>0?-1:-2); break;
        case FP_NAN:       x.bits=(unsigned int)(-3); break;
        // Right shift to remove some mantissa precision
        // (thus making the hash a bit fuzzy, to cope with ieee-754
        // equality checking problems)
        default: x.bits=x.bits>>6; break;
      }
      res = compute_hash_value(x.bits, res);
    }
    return (size_t) res;
  }
};

// Specialize std::equal_to<> for o3d::Matrix4
template<>
struct equal_to<o3d::Matrix4> {
  bool operator()(const o3d::Matrix4& a, const o3d::Matrix4& b) const {
    bool res(true);
    for (size_t i(0); i<4; ++i)
      res = res && (dot(a[i]-b[i],a[i]-b[i]) < SQUARE_EPSILON);
    return res;
  }
};
} // std

namespace o3d {

// o3d doesn't define a typedef for quaternions
typedef ::Vectormath::Aos::Quat Quat;

namespace extra {

// I'm getting really tired of GetApparentClass()
template<typename To>
static inline bool is_a(ObjectBase& f) {
  return f.IsA(To::GetApparentClass());
}

// Same goes for down_cast<>().
// I don't like having to write type names multiple times.
// Let's introduce the '<<*' down-casting operator!
//
// Usage: derived_ptr <<* base_ptr
// Returns null if downcast is not possible.
template<typename To, typename From>
static inline To* operator <<(To*& t, From& f) {
  t=is_a<To>(f)?down_cast<To*>(&f):0;
  return t;
}

// Short alias for protocol buffers base namespace
namespace pb = ::google::protobuf;

// protocol buffer helper
struct pbx {
  // Serialize a bounded protocol buffer
  static inline bool write(const pb::MessageLite& msg, pb::io::CodedOutputStream& stream) {
    stream.WriteVarint32(msg.ByteSize());
    return msg.SerializeToCodedStream(&stream);
  }

  // Deserialize a bounded protocol buffer
  static inline bool read(pb::MessageLite& msg, pb::io::ZeroCopyInputStream& stream) {
    uint32 size(0);
    do {
      pb::io::CodedInputStream tmp(&stream);
      if (!tmp.ReadVarint32(&size)) return false;
    } while (false);
    return msg.ParseFromBoundedZeroCopyStream(&stream, size);
  }
  
  // Custom log handler for libprotobuf, which
  // redirects messages from libprotobuf to loggingshim
  struct log_handler {
    pb::LogHandler* old_value;
    static void write(pb::LogLevel level, const char* filename, int line, const std::string& message) {
      LOG_MANUAL((level==pb::LOGLEVEL_ERROR)?ERROR:INFO, filename, line) << message;
    }
    log_handler(): old_value(pb::SetLogHandler(write)) { }
    ~log_handler() { pb::SetLogHandler(old_value); }
  };
};

// Matrix4 utility stuff
struct matrix4_extra {
  // is this matrix the identity?
  static inline bool is_identity(const Matrix4& m) {
    static const Matrix4 identity(Matrix4::identity());
    ::std::equal_to<Matrix4> e;
    return e(identity, m);
  }

  // Try to decompose a matrix into rotation, position and uniform scale.
  // Only works for rigid body transformations.
  static bool decompose(const Matrix4& m, Quat& rotation, Vector3& position, float& scale) {
    // Ensure last row is [0 0 0 1]
    Vector4 x(m.getRow(3) - Vector4::wAxis());
    if (dot(x,x) > SQUARE_EPSILON) return false;

    // Get rotation matrix
    Matrix3 upper(m.getUpper3x3());
    Matrix3 R;
    R[0] = normalize(upper[0]);
    R[1] = normalize(upper[1]-R[0]*dot(R[0], upper[1]));
    R[2] = normalize(upper[2]-R[0]*dot(R[0], upper[2])-R[1]*dot(R[1], upper[2]));
    if (determinant(R) < 0.f ) {
      for (size_t col(0); col < 3; ++col)
        R[col] = -R[col];
    }

    // Reject matrix if it as a shear component
    // Real shear is y*(1/s[0], 1/s[0], 1/s[1]), but we don't need the denominator
    // since we just test if it's zero
    Vector3 y(dot(R[0], upper[1]), dot(R[0], upper[2]), dot(R[1], upper[2]));
    if (dot(y,y) > SQUARE_EPSILON) return false;

    // Reject matrix if scaling is not uniform
    Vector3 s(dot(R[0], upper[0]), dot(R[1], upper[1]), dot(R[2], upper[2]));
    scale = s[0];
    if ((std::abs(scale-s[1]) > EPSILON) || (std::abs(scale-s[2]) > EPSILON)) return false;

    rotation = Quat(R);
    position = m[3].getXYZ();
    return true;
  }

  // Recompose a decomposed matrix
  static inline Matrix4 recompose(const Quat& rotation, const Vector3& position, const float& scale) {
    return Matrix4::translation(position)
         * Matrix4::rotation(rotation)
         * Matrix4::scale(Vector3(scale, scale, scale));
  }
};

// template class for mapping an index to an object and an object to its index
template<class T, class Hash=std::hash<T>, class Pred=std::equal_to<T> >
struct bidirectional_index {
  // key-to-value map type
  typedef std::vector<T> db_type;
  // value-to-key map type
  typedef std::tr1::unordered_map<T, size_t, Hash, Pred> reverse_db_type;
  db_type db;
  reverse_db_type reverse_db;
};

typedef bidirectional_index<std::string> string_db_t;
typedef bidirectional_index<Matrix4>     matrix_db_t;

} // namespace extra
} // namespace o3d

// We don't need those anymore
#undef EPSILON
#undef SQUARE_EPSILON

/* vim: set sw=2 ts=2 sts=2 expandtab ff=unix: */
