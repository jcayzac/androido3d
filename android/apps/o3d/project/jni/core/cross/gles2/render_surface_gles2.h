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


// This file contains the declarations for RenderSurfaceGLES2 and
// RenderDepthStencilSurfaceGLES2.

#ifndef O3D_CORE_CROSS_GLES2_RENDER_SURFACE_GLES2_H_
#define O3D_CORE_CROSS_GLES2_RENDER_SURFACE_GLES2_H_

#include "core/cross/gles2/gles2_headers.h"
#include "core/cross/render_surface.h"
#include "core/cross/texture.h"

namespace o3d {

class RenderSurfaceGLES2 : public RenderSurface {
 public:
  typedef SmartPointer<RenderSurfaceGLES2> Ref;

  // Constructs a RenderSurfaceGLES2 instance associated with the texture
  // argument.
  // Parameters:
  //  service_locator:  Service locator for the instance.
  //  width:  The width of the surface, in pixels.
  //  height:  The height of the surface, in pixels.
  //  cube_face:  The face of the cube texture to which the surface is to be
  //    associated.  NOTE: If the texture is a 2d texture, then the value of
  //    this argument is irrelevent.
  //  mip_level:  The mip-level of the texture to associate with the surface.
  //  texture:  The texture to associate with the surface.
  RenderSurfaceGLES2(ServiceLocator *service_locator,
                     int width,
                     int height,
                     GLenum cube_face,
                     int mip_level,
                     Texture *texture);
  virtual ~RenderSurfaceGLES2();

  GLenum cube_face() const {
    return cube_face_;
  }

  int mip_level() const {
    return mip_level_;
  }

 protected:
  // The platform specific part of GetBitmap.
  virtual Bitmap::Ref PlatformSpecificGetBitmap() const;

 private:
  GLenum cube_face_;
  int mip_level_;
  DISALLOW_COPY_AND_ASSIGN(RenderSurfaceGLES2);
};

class RenderDepthStencilSurfaceGLES2 : public RenderDepthStencilSurface {
 public:
  typedef SmartPointer<RenderDepthStencilSurfaceGLES2> Ref;

  RenderDepthStencilSurfaceGLES2(ServiceLocator *service_locator,
                                 int width,
                                 int height);
  virtual ~RenderDepthStencilSurfaceGLES2();

  GLuint depth_buffer() const {
    return render_buffers_[0];
  }

  GLuint stencil_buffer() const {
    return render_buffers_[1];
  }
 private:
  // Handles to the depth and stencil render-buffers, respectively.
  GLuint render_buffers_[2];
  DISALLOW_COPY_AND_ASSIGN(RenderDepthStencilSurfaceGLES2);
};

}  // namespace o3d

#endif  // O3D_CORE_CROSS_GLES2_RENDER_SURFACE_GLES2_H_

