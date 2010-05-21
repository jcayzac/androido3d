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


// This file contains the declaration of the StreamBankGLES2 class.

#ifndef O3D_CORE_CROSS_GLES2_STREAM_BANK_GLES2_H_
#define O3D_CORE_CROSS_GLES2_STREAM_BANK_GLES2_H_

#include <map>
#include "core/cross/stream_bank.h"
#include "core/cross/gles2/param_cache_gles2.h"

namespace o3d {

// StreamBankGLES2 is the OpenGLES2 implementation of the StreamBank.
class StreamBankGLES2 : public StreamBank {
 public:
  explicit StreamBankGLES2(ServiceLocator* service_locator);
  virtual ~StreamBankGLES2();

  // Sets the streams for rendering.
  // Parameter:
  //   varying_map: Map of streams.
  //   max_vertrices: pointer to variable to receive the maximum vertices
  //     the streams can render.
  // Returns:
  //   true if all streams were bound.
  bool BindStreamsForRendering(
      const ParamCacheGLES2::VaryingParameterMap& varying_map,
      GLuint gl_program,
      unsigned int* max_vertices);

  // Checks for all required streams before rendering.
  bool CheckForMissingVertexStreams(
      ParamCacheGLES2::VaryingParameterMap& varying_map,
      GLuint gl_program,
      String* missing_stream);

 private:
  int FindVertexStream(Stream::Semantic semantic, int index);
};
}  // o3d

#endif  // O3D_CORE_CROSS_GLES2_STREAM_BANK_GLES2_H_

