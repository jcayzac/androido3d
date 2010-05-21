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


// This file contains the definition of the RendererGLES2 class that provides
// low-level access for O3D to graphics hardware using the OpenGLES2 API.

#ifndef O3D_CORE_CROSS_GLES2_RENDERER_GLES2_H_
#define O3D_CORE_CROSS_GLES2_RENDERER_GLES2_H_

#include "core/cross/gles2/gles2_headers.h"
#include <build/build_config.h>
#include "core/cross/renderer.h"
#include "core/cross/renderer_platform.h"
#include "core/cross/types.h"
#include "core/cross/state.h"

namespace o3d {

class Material;
class Effect;
class DrawEffect;
class SemanticManager;

// Implements the genereric Renderer interface using OpenGLES2.
class RendererGLES2 : public Renderer {
 public:
  // Creates a default Renderer.
  static RendererGLES2* CreateDefault(ServiceLocator* service_locator);
  virtual ~RendererGLES2();

  // Initialises the renderer for use, claiming hardware resources.
  virtual InitStatus InitPlatformSpecific(const DisplayWindow& display,
                                          bool off_screen);

  // Released all hardware resources.
  virtual void Destroy();

  // Overridden from Renderer.
  virtual bool GoFullscreen(const DisplayWindow& display,
                            int mode_id);

  // Overridden from Renderer.
  virtual bool CancelFullscreen(const DisplayWindow& display,
                                int width, int height);

  // Tells whether we're currently displayed fullscreen or not.
  virtual bool fullscreen() const {
    return fullscreen_;
  }

  // Get a vector of the available fullscreen display modes.
  // Clears *modes on error.
  virtual void GetDisplayModes(std::vector<DisplayMode> *modes);

  // Get a single fullscreen display mode by id.
  // Returns true on success, false on error.
  virtual bool GetDisplayMode(int id, DisplayMode *mode);

  // Resizes the viewport in OpenGLES2.
  virtual void Resize(int width, int height);

  // Creates a StreamBank, returning a platform specific implementation class.
  virtual StreamBank::Ref CreateStreamBank();

  // Creates a Primitive, returning a platform specific implementation class.
  virtual Primitive::Ref CreatePrimitive();

  // Creates a DrawElement, returning a platform specific implementation
  // class.
  virtual DrawElement::Ref CreateDrawElement();

  // Creates and returns a GLES2 specific float buffer.
  virtual VertexBuffer::Ref CreateVertexBuffer();

  // Creates and returns a GLES2 specific integer buffer.
  virtual IndexBuffer::Ref CreateIndexBuffer();

  // Creates and returns a GLES2 specific Effect object.
  virtual Effect::Ref CreateEffect();

  // Creates and returns a GLES2 specific Sampler object.
  virtual Sampler::Ref CreateSampler();

  // Creates and returns a platform-specific RenderDepthStencilSurface object
  // for use as a depth-stencil render target.
  virtual RenderDepthStencilSurface::Ref CreateDepthStencilSurface(
      int width,
      int height);

  // Overridden from Renderer.
  virtual const int* GetRGBAUByteNSwizzleTable();

  // Makes this renderer active on the current thread if it is not active
  // already.
  void MakeCurrentLazy() {
    if (!IsCurrent())
      MakeCurrent();
  }

  // Returns whether or not this renderer is active on the current thread.
  // Don't worry, the "get" calls are el cheapo.
  bool IsCurrent() {
#if defined(OS_MACOSX)
    if ((mac_agl_context_ != NULL) &&
        (mac_agl_context_ == aglGetCurrentContext())) {
      return true;
    } else if ((mac_cgl_context_ != NULL) &&
               (mac_cgl_context_ == CGLGetCurrentContext())) {
      return true;
    }
#elif defined(OS_WIN)
    if ((gl_context_ != NULL) &&
        (gl_context_ == wglGetCurrentContext())) {
      return true;
    }
#elif defined(OS_LINUX)
#if defined(GLES2_BACKEND_DESKTOP_GL)
    if ((context_ != NULL) &&
        (context_ == glXGetCurrentContext())) {
      return true;
    }
#elif defined(GLES2_BACKEND_NATIVE_GLES2)
    if (egl_context_ && egl_context_ == eglGetCurrentContext()) {
      return true;
    }
#elif defined(GLES2_BACKEND_GLES2_COMMAND_BUFFERS)
#error RendererGLES2::IsCurrent() Not implemented.
#endif
#else
    Error: must port RendererGLES2::IsCurrent() to your platform.
#endif
    return false;
  }

  // Makes this renderer active on the current thread.
  bool MakeCurrent();

  // Called by EffectGLES2::PrepareForDraw before setting any parameters.
  void ResetTextureGroupSetCount() {
    ++texture_unit_group_set_count_;
    next_texture_unit_ = 0;
  }

  int GetTextureGroupSetCount() {
    return texture_unit_group_set_count_;
  }

  // Samplers call this if their texture group set count is out of date.
  GLenum GetNextTextureUnit() {
    return next_texture_unit_++;
  }

  // Programs the helper constants into the hardware.
  void UpdateDxClippingUniform(GLint location);

 protected:
  // Keep the constructor protected so only factory methods can create
  // renderers.
  explicit RendererGLES2(ServiceLocator* service_locator);

  // Overridden from Renderer.
  virtual bool PlatformSpecificBeginDraw();

  // Overridden from Renderer.
  virtual void PlatformSpecificEndDraw();

  // Overridden from Renderer.
  virtual bool PlatformSpecificStartRendering();

  // Overridden from Renderer.
  virtual void PlatformSpecificFinishRendering();

  // Overridden from Renderer.
  virtual void PlatformSpecificPresent();

  // Overridden from Renderer.
  virtual void PlatformSpecificClear(const Float4 &color,
                                     bool color_flag,
                                     float depth,
                                     bool depth_flag,
                                     int stencil,
                                     bool stencil_flag);

  // Overridden from Renderer.
  virtual ParamCache* CreatePlatformSpecificParamCache();

  // Sets the viewport. This is the platform specific version.
  void SetViewportInPixels(int left,
                           int top,
                           int width,
                           int height,
                           float min_z,
                           float max_z);

  // Overridden from Renderer.
  virtual void SetBackBufferPlatformSpecific();

  // Overridden from Renderer.
  virtual void SetRenderSurfacesPlatformSpecific(
      const RenderSurface* surface,
      const RenderDepthStencilSurface* depth_surface);

  // Overridden from Renderer.
  virtual Texture2D::Ref CreatePlatformSpecificTexture2D(
      int width,
      int height,
      Texture::Format format,
      int levels,
      bool enable_render_surfaces);

  // Overridden from Renderer.
  virtual TextureCUBE::Ref CreatePlatformSpecificTextureCUBE(
      int edge_length,
      Texture::Format format,
      int levels,
      bool enable_render_surfaces);

  // Overridden from Renderer.
  virtual void ApplyDirtyStates();

 private:
  // Platform-independent GLES2 initialization
  InitStatus InitCommonGLES2();

  // Platform-independent GLES2 destruction
  void DestroyCommonGLES2();

  // Updates the helper constant used to remap D3D clip coordinates to GLES2
  // ones.
  void UpdateHelperConstant(float width, float height);

  ServiceDependency<SemanticManager> semantic_manager_;

  // Indicates we're rendering fullscreen rather than in the plugin region.
  bool fullscreen_;

#ifdef OS_WIN
  // Handle to the GLES2 device.
  HWND window_;
  HDC device_context_;
  HGLRC gl_context_;
#endif

#ifdef OS_MACOSX
  AGLContext    mac_agl_context_;
  CGLContextObj mac_cgl_context_;
#endif

#ifdef OS_LINUX
  Display *display_;
  Window window_;
#if defined(GLES2_BACKEND_DESKTOP_GL)
  GLXContext context_;
#elif defined(GLES2_BACKEND_NATIVE_GLES2)
  EGLDisplay egl_display_;
  EGLSurface egl_surface_;
  EGLContext egl_context_;
#endif
#endif

  // Handle to the framebuffer-object used while rendering to off-screen
  // targets.
  GLuint render_surface_framebuffer_;

  friend class AlphaReferenceHandler;
  bool alpha_function_ref_changed_;
  GLenum alpha_function_;
  GLclampf alpha_ref_;

  friend class BlendFunctionHandler;
  friend class BlendEquationHandler;
  bool alpha_blend_settings_changed_;
  bool separate_alpha_blend_enable_;
  enum {
    RGB,
    ALPHA,
  };
  enum {
    SRC,
    DST,
  };
  GLenum blend_function_[2][2];  // SRC/DST, RGB/ALPHA
  GLenum blend_equation_[2];  //  RGB/ALPHA

  bool stencil_settings_changed_;
  bool separate_stencil_settings_enable_;

  // States for Stencils
  friend class StencilOperationHandler;
  friend class StencilRefHandler;
  friend class StencilMaskHandler;
  struct StencilStates {
    GLenum func_;
    enum {
      FAIL_OP,
      ZFAIL_OP,
      PASS_OP,
    };
    int op_[3];
  };

  enum {
    FRONT,
    BACK,
  };
  StencilStates stencil_settings_[2];

  enum {
    READ_MASK,
    WRITE_MASK,
  };
  int stencil_mask_[2];
  int stencil_ref_;

  // States for PolygonOffset
  friend class PolygonOffset1Handler;
  friend class PolygonOffset2Handler;
  bool polygon_offset_changed_;
  float polygon_offset_factor_;
  float polygon_offset_bias_;

  // Sets the stencils states for either front, back or both facing polys.
  void SetStencilStates(GLenum face, const StencilStates& stencil_states);

  // Sampler test against this to see if their cached texture unit is valid.
  int texture_unit_group_set_count_;

  // The next texture unit to use. This is reset with ResetTextureUnit
  // and retrieved with GetNextTextureUnit.
  GLenum next_texture_unit_;

  // Transform matrix coefficients to match DX clipping rules.
  GLfloat dx_clipping_[4];
};

}  // namespace o3d

#endif  // O3D_CORE_CROSS_GLES2_RENDERER_GLES2_H_
