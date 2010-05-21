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


// This file contains the definition of the RendererGLES2 class that
// implements the abstract Renderer API using OpenGLES2.

#include "core/cross/gles2/renderer_gles2.h"

#include "core/cross/error.h"
#include "core/cross/gles2/buffer_gles2.h"
#include "core/cross/gles2/draw_element_gles2.h"
#include "core/cross/gles2/effect_gles2.h"
#include "core/cross/gles2/param_cache_gles2.h"
#include "core/cross/gles2/primitive_gles2.h"
#include "core/cross/gles2/render_surface_gles2.h"
#include "core/cross/gles2/sampler_gles2.h"
#include "core/cross/gles2/stream_bank_gles2.h"
#include "core/cross/gles2/texture_gles2.h"
#include "core/cross/gles2/utils_gles2-inl.h"
#include "core/cross/gles2/utils_gles2.h"
#include "core/cross/material.h"
#include "core/cross/semantic_manager.h"
#include "core/cross/features.h"
#include "core/cross/shape.h"
#include "core/cross/types.h"

namespace o3d {

namespace {

GLenum ConvertCmpFunc(State::Comparison cmp) {
  switch (cmp) {
    case State::CMP_ALWAYS:
      return GL_ALWAYS;
    case State::CMP_NEVER:
      return GL_NEVER;
    case State::CMP_LESS:
      return GL_LESS;
    case State::CMP_GREATER:
      return GL_GREATER;
    case State::CMP_LEQUAL:
      return GL_LEQUAL;
    case State::CMP_GEQUAL:
      return GL_GEQUAL;
    case State::CMP_EQUAL:
      return GL_EQUAL;
    case State::CMP_NOTEQUAL:
      return GL_NOTEQUAL;
    default:
      break;
  }
  return GL_ALWAYS;
}

#if defined(GLES2_BACKEND_DESKTOP_GL)
GLenum ConvertFillMode(State::Fill mode) {
  switch (mode) {
    case State::POINT:
      return GL_POINT;
    case State::WIREFRAME:
      return GL_LINE;
    case State::SOLID:
      return GL_FILL;
    default:
      break;
  }
  return GL_FILL;
}
#endif

GLenum ConvertBlendFunc(State::BlendingFunction blend_func) {
  switch (blend_func) {
    case State::BLENDFUNC_ZERO:
      return GL_ZERO;
    case State::BLENDFUNC_ONE:
      return GL_ONE;
    case State::BLENDFUNC_SOURCE_COLOR:
      return GL_SRC_COLOR;
    case State::BLENDFUNC_INVERSE_SOURCE_COLOR:
      return GL_ONE_MINUS_SRC_COLOR;
    case State::BLENDFUNC_SOURCE_ALPHA:
      return GL_SRC_ALPHA;
    case State::BLENDFUNC_INVERSE_SOURCE_ALPHA:
      return GL_ONE_MINUS_SRC_ALPHA;
    case State::BLENDFUNC_DESTINATION_ALPHA:
      return GL_DST_ALPHA;
    case State::BLENDFUNC_INVERSE_DESTINATION_ALPHA:
      return GL_ONE_MINUS_DST_ALPHA;
    case State::BLENDFUNC_DESTINATION_COLOR:
      return GL_DST_COLOR;
    case State::BLENDFUNC_INVERSE_DESTINATION_COLOR:
      return GL_ONE_MINUS_DST_COLOR;
    case State::BLENDFUNC_SOURCE_ALPHA_SATUTRATE:
      return GL_SRC_ALPHA_SATURATE;
    default:
      break;
  }
  return GL_ONE;
}

GLenum ConvertBlendEquation(State::BlendingEquation blend_equation) {
  switch (blend_equation) {
    case State::BLEND_ADD:
      return GL_FUNC_ADD;
    case State::BLEND_SUBTRACT:
      return GL_FUNC_SUBTRACT;
    case State::BLEND_REVERSE_SUBTRACT:
      return GL_FUNC_REVERSE_SUBTRACT;
#if defined(GLES2_BACKEND_DESKTOP_GL)
    case State::BLEND_MIN:
      return GL_MIN;
    case State::BLEND_MAX:
      return GL_MAX;
#else
    case State::BLEND_MIN:
    case State::BLEND_MAX:
      NOTIMPLEMENTED() << "MIN/MAX blend equation";
      break;
#endif
    default:
      break;
  }
  return GL_FUNC_ADD;
}

GLenum ConvertStencilOp(State::StencilOperation stencil_func) {
  switch (stencil_func) {
    case State::STENCIL_KEEP:
      return GL_KEEP;
    case State::STENCIL_ZERO:
      return GL_ZERO;
    case State::STENCIL_REPLACE:
      return GL_REPLACE;
    case State::STENCIL_INCREMENT_SATURATE:
      return GL_INCR;
    case State::STENCIL_DECREMENT_SATURATE:
      return GL_DECR;
    case State::STENCIL_INVERT:
      return GL_INVERT;
    case State::STENCIL_INCREMENT:
      return GL_INCR_WRAP;
    case State::STENCIL_DECREMENT:
      return GL_DECR_WRAP;
    default:
      break;
  }
  return GL_KEEP;
}

#ifndef DISABLE_FBO
// Helper routine that will bind the surfaces stored in the RenderSurface and
// RenderDepthStencilSurface arguments to the current OpenGLES2 context.
// Returns true upon success.
// Note:  This routine assumes that a frambuffer object is presently bound
// to the context.
bool InstallFramebufferObjects(const RenderSurface* surface,
                               const RenderDepthStencilSurface* surface_depth) {
#ifdef _DEBUG
  GLint bound_framebuffer;
  ::glGetIntegerv(GL_FRAMEBUFFER_BINDING, &bound_framebuffer);
  DCHECK(bound_framebuffer != 0);
#endif

  // Reset the bound attachments to the current framebuffer object.
  ::glFramebufferRenderbufferEXT(GL_FRAMEBUFFER,
                                 GL_COLOR_ATTACHMENT0,
                                 GL_RENDERBUFFER,
                                 0);

  ::glFramebufferRenderbufferEXT(GL_FRAMEBUFFER,
                                 GL_DEPTH_ATTACHMENT,
                                 GL_RENDERBUFFER,
                                 0);

  ::glFramebufferRenderbufferEXT(GL_FRAMEBUFFER,
                                 GL_STENCIL_ATTACHMENT,
                                 GL_RENDERBUFFER,
                                 0);

  if (surface) {
    const RenderSurfaceGLES2 *gl_surface =
        down_cast<const RenderSurfaceGLES2*>(surface);
    Texture *texture = gl_surface->texture();
    GLuint handle = static_cast<GLuint>(reinterpret_cast<intptr_t>(
        texture->GetTextureHandle()));
    if (texture->IsA(Texture2D::GetApparentClass())) {
      ::glFramebufferTexture2DEXT(
          GL_FRAMEBUFFER,
          GL_COLOR_ATTACHMENT0,
          GL_TEXTURE_2D,
          handle,
          gl_surface->mip_level());
    } else if (texture->IsA(TextureCUBE::GetApparentClass())) {
      ::glFramebufferTexture2DEXT(
          GL_FRAMEBUFFER,
          GL_COLOR_ATTACHMENT0,
          gl_surface->cube_face(),
          handle,
          gl_surface->mip_level());
    }
  }

  if (surface_depth) {
    // Bind both the depth and stencil attachments.
    const RenderDepthStencilSurfaceGLES2* gl_surface =
        down_cast<const RenderDepthStencilSurfaceGLES2*>(surface_depth);
    ::glFramebufferRenderbufferEXT(GL_FRAMEBUFFER,
                                   GL_DEPTH_ATTACHMENT,
                                   GL_RENDERBUFFER,
                                   gl_surface->depth_buffer());
    ::glFramebufferRenderbufferEXT(GL_FRAMEBUFFER,
                                   GL_STENCIL_ATTACHMENT,
                                   GL_RENDERBUFFER,
                                   gl_surface->stencil_buffer());
  }
  GLenum framebuffer_status = ::glCheckFramebufferStatusEXT(GL_FRAMEBUFFER);
  if (GL_FRAMEBUFFER_COMPLETE != framebuffer_status) {
    return false;
  }

  CHECK_GL_ERROR();
  return true;
}
#endif

// Helper routine that returns a pointer to the non-NULL entry in the renderer's
// stack of bound surfaces.
const RenderSurfaceBase* GetValidRenderSurface(
    const std::pair<RenderSurface*, RenderDepthStencilSurface*> &stack_entry) {
  if (stack_entry.first) {
    return stack_entry.first;
  } else {
    return stack_entry.second;
  }
}

}  // unnamed namespace

// This class wraps StateHandler to make it typesafe.
template <typename T>
class TypedStateHandler : public RendererGLES2::StateHandler {
 public:
  // Override this function to set a specific state.
  // Parameters:
  //   renderer: The platform specific renderer.
  //   param: A concrete param with state data.
  virtual void SetStateFromTypedParam(
      RendererGLES2* renderer, T* param) const = 0;

  // Gets Class of State's Parameter
  virtual const ObjectBase::Class* GetClass() const {
    return T::GetApparentClass();
  }

 private:
  // Calls SetStateFromTypedParam if the Param type is the correct type.
  // Parameters:
  //   renderer: The platform specific renderer.
  //   param: A param with state data.
  virtual void SetState(Renderer* renderer, Param* param) const {
    RendererGLES2 *renderer_gl = down_cast<RendererGLES2 *>(renderer);
    // This is safe because State guarntees Params match by type.
    DCHECK(param->IsA(T::GetApparentClass()));
    SetStateFromTypedParam(renderer_gl, down_cast<T*>(param));
  }
};

template <class T>
class NoOpHandler : public TypedStateHandler<T> {
 public:
  virtual void SetStateFromTypedParam(RendererGLES2* renderer,
                                      T* param) const {
  }
};


// A template the generates a handler for enable/disable states.
// Parameters:
//   state_constant: GLenum of state we want to enable/disable
template <GLenum state_constant>
class StateEnableHandler : public TypedStateHandler<ParamBoolean> {
 public:
  virtual void SetStateFromTypedParam(RendererGLES2* renderer,
                                      ParamBoolean* param) const {
    if (param->value()) {
      ::glEnable(state_constant);
    } else {
      ::glDisable(state_constant);
    }
  }
};

class BoolHandler : public TypedStateHandler<ParamBoolean> {
 public:
  explicit BoolHandler(bool* var, bool* changed_var)
      : var_(*var),
        changed_var_(*changed_var) {
  }
  virtual void SetStateFromTypedParam(RendererGLES2 *renderer,
                                      ParamBoolean *param) const {
    var_ = param->value();
  }
 private:
  bool& var_;
  bool& changed_var_;
};

class ZWriteEnableHandler : public TypedStateHandler<ParamBoolean> {
 public:
  virtual void SetStateFromTypedParam(RendererGLES2 *renderer,
                                      ParamBoolean *param) const {
    ::glDepthMask(param->value());
  }
};

class AlphaReferenceHandler : public TypedStateHandler<ParamFloat> {
 public:
  virtual void SetStateFromTypedParam(RendererGLES2* renderer,
                                      ParamFloat* param) const {
    float refFloat = param->value();

    // cap the float to the required range
    if (refFloat < 0.0f) {
      refFloat = 0.0f;
    } else if (refFloat > 1.0f) {
      refFloat = 1.0f;
    }

    renderer->alpha_function_ref_changed_ = true;
    renderer->alpha_ref_ = refFloat;
  }
};

class CullModeHandler : public TypedStateHandler<ParamInteger> {
 public:
  virtual void SetStateFromTypedParam(RendererGLES2* renderer,
                                      ParamInteger* param) const {
    State::Cull cull = static_cast<State::Cull>(param->value());
    switch (cull) {
      case State::CULL_CW:
        ::glEnable(GL_CULL_FACE);
        ::glCullFace(GL_BACK);
        break;
      case State::CULL_CCW:
        ::glEnable(GL_CULL_FACE);
        ::glCullFace(GL_FRONT);
        break;
      default:
        ::glDisable(GL_CULL_FACE);
        break;
    }
  }
};

class PolygonOffset1Handler : public TypedStateHandler<ParamFloat> {
 public:
  virtual void SetStateFromTypedParam(RendererGLES2* renderer,
                                      ParamFloat* param) const {
    renderer->polygon_offset_factor_ = param->value();
    renderer->polygon_offset_changed_ = true;
  }
};

class PolygonOffset2Handler : public TypedStateHandler<ParamFloat> {
 public:
  virtual void SetStateFromTypedParam(RendererGLES2* renderer,
                                      ParamFloat* param) const {
    renderer->polygon_offset_bias_ = param->value();
    renderer->polygon_offset_changed_ = true;
  }
};

class FillModeHandler : public TypedStateHandler<ParamInteger> {
 public:
  virtual void SetStateFromTypedParam(RendererGLES2* renderer,
                                      ParamInteger* param) const {
#if defined(GLES2_BACKEND_DESKTOP_GL)
    ::glPolygonMode(GL_FRONT_AND_BACK,
                    ConvertFillMode(static_cast<State::Fill>(param->value())));
#else
    NOTIMPLEMENTED() << "Fill mode";
#endif
  }
};

class ZFunctionHandler : public TypedStateHandler<ParamInteger> {
 public:
  virtual void SetStateFromTypedParam(RendererGLES2* renderer,
                                      ParamInteger* param) const {
    ::glDepthFunc(
        ConvertCmpFunc(static_cast<State::Comparison>(param->value())));
  }
};

class BlendEquationHandler : public TypedStateHandler<ParamInteger> {
 public:
  explicit BlendEquationHandler(GLenum* var)
      : var_(*var) {
  }
  virtual void SetStateFromTypedParam(RendererGLES2* renderer,
                                      ParamInteger* param) const {
    renderer->alpha_blend_settings_changed_ = true;
    var_ = ConvertBlendEquation(
        static_cast<State::BlendingEquation>(param->value()));
  }
 private:
  GLenum& var_;
};

class BlendFunctionHandler : public TypedStateHandler<ParamInteger> {
 public:
  explicit BlendFunctionHandler(GLenum* var)
      : var_(*var) {
  }
  virtual void SetStateFromTypedParam(RendererGLES2* renderer,
                                      ParamInteger* param) const {
    renderer->alpha_blend_settings_changed_ = true;
    var_ = ConvertBlendFunc(
        static_cast<State::BlendingFunction>(param->value()));
  }
 private:
  GLenum& var_;
};


class StencilOperationHandler : public TypedStateHandler<ParamInteger> {
 public:
  StencilOperationHandler(int face, int condition)
      : face_(face) ,
        condition_(condition) {
  }
  virtual void SetStateFromTypedParam(RendererGLES2* renderer,
                                      ParamInteger* param) const  {
    renderer->stencil_settings_changed_ = true;
    renderer->stencil_settings_[face_].op_[condition_] = ConvertStencilOp(
        static_cast<State::StencilOperation>(param->value()));
  }
 private:
  int face_;
  int condition_;
};

class ComparisonFunctionHandler : public TypedStateHandler<ParamInteger> {
 public:
  ComparisonFunctionHandler(GLenum* var, bool* changed_var)
      : var_(*var),
        changed_var_(*changed_var) {
  }
  virtual void SetStateFromTypedParam(RendererGLES2* renderer,
                                      ParamInteger* param) const {
    changed_var_ = true;
    var_ = ConvertCmpFunc(static_cast<State::Comparison>(param->value()));
  }
 private:
  GLenum& var_;
  bool& changed_var_;
};

class StencilRefHandler : public TypedStateHandler<ParamInteger> {
 public:
  virtual void SetStateFromTypedParam(RendererGLES2* renderer,
                                      ParamInteger* param) const {
    renderer->stencil_settings_changed_ = true;
    renderer->stencil_ref_ = param->value();
  }
};

class StencilMaskHandler : public TypedStateHandler<ParamInteger> {
 public:
  explicit StencilMaskHandler(int mask_index)
      : mask_index_(mask_index) {
  }
  virtual void SetStateFromTypedParam(RendererGLES2* renderer,
                                      ParamInteger* param) const {
    renderer->stencil_settings_changed_ = true;
    renderer->stencil_mask_[mask_index_] = param->value();
  }
 private:
  int mask_index_;
};

class ColorWriteEnableHandler : public TypedStateHandler<ParamInteger> {
 public:
  virtual void SetStateFromTypedParam(RendererGLES2* renderer,
                                      ParamInteger* param) const {
    int mask = param->value();
    ::glColorMask((mask & 0x1) != 0,
                  (mask & 0x2) != 0,
                  (mask & 0x4) != 0,
                  (mask & 0x8) != 0);
    renderer->SetWriteMask(mask);
  }
};

class PointSpriteEnableHandler : public TypedStateHandler<ParamBoolean> {
 public:
  virtual void SetStateFromTypedParam(RendererGLES2* renderer,
                                      ParamBoolean* param) const {
#if defined(GLES2_BACKEND_DESKTOP_GL)
    if (param->value()) {
      ::glEnable(GL_POINT_SPRITE);
      // TODO(o3d): It's not clear from D3D docs that point sprites affect
      // TEXCOORD0, but that's my guess. Check that.
      ::glActiveTextureARB(GL_TEXTURE0);
      ::glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
    } else {
      ::glActiveTextureARB(GL_TEXTURE0);
      ::glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_FALSE);
      ::glDisable(GL_POINT_SPRITE);
    }
#else
    NOTIMPLEMENTED() << "Point Sprites";
#endif
  }
};

class PointSizeHandler : public TypedStateHandler<ParamFloat> {
 public:
  virtual void SetStateFromTypedParam(RendererGLES2* renderer,
                                      ParamFloat* param) const {
#if defined(GLES2_BACKEND_DESKTOP_GL)
    ::glPointSize(param->value());
#else
    NOTIMPLEMENTED() << "Point Size";
#endif
  }
};

RendererGLES2* RendererGLES2::CreateDefault(ServiceLocator* service_locator) {
  return new RendererGLES2(service_locator);
}

RendererGLES2::RendererGLES2(ServiceLocator* service_locator)
    : Renderer(service_locator),
      semantic_manager_(service_locator),
#ifdef OS_WIN
      gl_context_(NULL),
#endif
      fullscreen_(0),
#ifdef OS_LINUX
      display_(NULL),
      window_(0),
#if defined(GLES2_BACKEND_DESKTOP_GL)
      context_(0),
#elif defined(GLES2_BACKEND_NATIVE_GLES2)
      egl_display_(NULL),
      egl_surface_(NULL),
      egl_context_(NULL),
#endif  // GLES_BACKEND_xxx
#endif  // OS_LINUX
#ifdef OS_MACOSX
      mac_agl_context_(0),
      mac_cgl_context_(0),
#endif
      render_surface_framebuffer_(0),
      alpha_function_ref_changed_(true),
      alpha_function_(GL_ALWAYS),
      alpha_ref_(0.f),
      alpha_blend_settings_changed_(true),
      separate_alpha_blend_enable_(false),
      stencil_settings_changed_(true),
      separate_stencil_settings_enable_(false),
      stencil_ref_(0),
      polygon_offset_changed_(true),
      polygon_offset_factor_(0.f),
      polygon_offset_bias_(0.f) {
  DLOG(INFO) << "RendererGLES2 Construct";

  // Setup default state values.
  for (int ii = 0; ii < 2; ++ii) {
    stencil_settings_[ii].func_ = GL_ALWAYS;
    stencil_settings_[ii].op_[StencilStates::FAIL_OP] = GL_KEEP;
    stencil_settings_[ii].op_[StencilStates::ZFAIL_OP] = GL_KEEP;
    stencil_settings_[ii].op_[StencilStates::PASS_OP] = GL_KEEP;
    stencil_mask_[ii] = -1;
    blend_function_[ii][FRONT] = GL_ONE;
    blend_function_[ii][BACK] = GL_ZERO;
    blend_equation_[ii] = GL_FUNC_ADD;
  }

  // Setup state handlers
#if defined(GLES2_BACKEND_DESKTOP_GL)
  AddStateHandler(State::kAlphaTestEnableParamName,
                  new StateEnableHandler<GL_ALPHA_TEST>);
#else
  // TODO(piman): Alpha test isn't supported in GLES2, because it can be done
  // in a shader. Solution here would be to add instructions to the shader...
  AddStateHandler(State::kAlphaTestEnableParamName,
                  new NoOpHandler<ParamBoolean>);
#endif
  AddStateHandler(State::kAlphaReferenceParamName,
                  new AlphaReferenceHandler);
  AddStateHandler(State::kAlphaComparisonFunctionParamName,
                  new ComparisonFunctionHandler(&alpha_function_,
                                                &alpha_function_ref_changed_));
  AddStateHandler(State::kCullModeParamName,
                  new CullModeHandler);
  AddStateHandler(State::kDitherEnableParamName,
                  new StateEnableHandler<GL_DITHER>);
#if defined(GLES2_BACKEND_DESKTOP_GL)
  AddStateHandler(State::kLineSmoothEnableParamName,
                  new StateEnableHandler<GL_LINE_SMOOTH>);
#else
  AddStateHandler(State::kLineSmoothEnableParamName,
                  new NoOpHandler<ParamBoolean>);
#endif
  AddStateHandler(State::kPointSpriteEnableParamName,
                  new PointSpriteEnableHandler);
  AddStateHandler(State::kPointSizeParamName,
                  new PointSizeHandler);
  AddStateHandler(State::kPolygonOffset1ParamName,
                  new PolygonOffset1Handler);
  AddStateHandler(State::kPolygonOffset2ParamName,
                  new PolygonOffset2Handler);
  AddStateHandler(State::kFillModeParamName,
                  new FillModeHandler);
  AddStateHandler(State::kZEnableParamName,
                  new StateEnableHandler<GL_DEPTH_TEST>);
  AddStateHandler(State::kZWriteEnableParamName,
                  new ZWriteEnableHandler);
  AddStateHandler(State::kZComparisonFunctionParamName,
                  new ZFunctionHandler);
  AddStateHandler(State::kAlphaBlendEnableParamName,
                  new StateEnableHandler<GL_BLEND>);
  AddStateHandler(State::kSourceBlendFunctionParamName,
                  new BlendFunctionHandler(&blend_function_[SRC][RGB]));
  AddStateHandler(State::kDestinationBlendFunctionParamName,
                  new BlendFunctionHandler(&blend_function_[DST][RGB]));
  AddStateHandler(State::kStencilEnableParamName,
                  new StateEnableHandler<GL_STENCIL_TEST>);
  AddStateHandler(State::kStencilFailOperationParamName,
                  new StencilOperationHandler(FRONT, StencilStates::FAIL_OP));
  AddStateHandler(State::kStencilZFailOperationParamName,
                  new StencilOperationHandler(FRONT, StencilStates::ZFAIL_OP));
  AddStateHandler(State::kStencilPassOperationParamName,
                  new StencilOperationHandler(FRONT, StencilStates::PASS_OP));
  AddStateHandler(State::kStencilComparisonFunctionParamName,
                  new ComparisonFunctionHandler(
                      &stencil_settings_[FRONT].func_,
                      &stencil_settings_changed_));
  AddStateHandler(State::kStencilReferenceParamName,
                  new StencilRefHandler);
  AddStateHandler(State::kStencilMaskParamName,
                  new StencilMaskHandler(READ_MASK));
  AddStateHandler(State::kStencilWriteMaskParamName,
                  new StencilMaskHandler(WRITE_MASK));
  AddStateHandler(State::kColorWriteEnableParamName,
                  new ColorWriteEnableHandler);
  AddStateHandler(State::kBlendEquationParamName,
                  new BlendEquationHandler(&blend_equation_[RGB]));
  AddStateHandler(State::kTwoSidedStencilEnableParamName,
                  new BoolHandler(&separate_stencil_settings_enable_,
                                  &stencil_settings_changed_));
  AddStateHandler(State::kCCWStencilFailOperationParamName,
                  new StencilOperationHandler(BACK, StencilStates::FAIL_OP));
  AddStateHandler(State::kCCWStencilZFailOperationParamName,
                  new StencilOperationHandler(BACK, StencilStates::ZFAIL_OP));
  AddStateHandler(State::kCCWStencilPassOperationParamName,
                  new StencilOperationHandler(BACK, StencilStates::PASS_OP));
  AddStateHandler(State::kCCWStencilComparisonFunctionParamName,
                  new ComparisonFunctionHandler(
                      &stencil_settings_[BACK].func_,
                      &stencil_settings_changed_));
  AddStateHandler(State::kSeparateAlphaBlendEnableParamName,
                  new BoolHandler(&separate_alpha_blend_enable_,
                                  &alpha_blend_settings_changed_));
  AddStateHandler(State::kSourceBlendAlphaFunctionParamName,
                  new BlendFunctionHandler(&blend_function_[SRC][ALPHA]));
  AddStateHandler(State::kDestinationBlendAlphaFunctionParamName,
                  new BlendFunctionHandler(&blend_function_[DST][ALPHA]));
  AddStateHandler(State::kBlendAlphaEquationParamName,
                  new BlendEquationHandler(&blend_equation_[ALPHA]));
}

RendererGLES2::~RendererGLES2() {
  Destroy();
}

// platform neutral initialization code
//
Renderer::InitStatus RendererGLES2::InitCommonGLES2() {
#if defined(GLES2_BACKEND_DESKTOP_GL)
  GLenum glew_error = glewInit();
  if (glew_error != GLEW_OK) {
    DLOG(ERROR) << "Unable to initialise GLEW : "
                << ::glewGetErrorString(glew_error);
    return INITIALIZATION_ERROR;
  }

  // Check to see that we can use the OpenGLES2 vertex attribute APIs
  // TODO(o3d):  We should return false if this check fails, but because some
  // Intel hardware does not support OpenGLES2 2.0, yet does support all of the
  // extensions we require, we only log an error.  A future CL should change
  // this check to ensure that all of the extension strings we require are
  // present.
  if (!GLEW_VERSION_2_0) {
    DLOG(ERROR) << "GLES2 drivers do not have OpenGLES2 2.0 functionality.";
  }

  if (!GLEW_ARB_vertex_buffer_object) {
    // NOTE: Linux NVidia drivers claim to support OpenGLES2 2.0 when using
    // indirect rendering (e.g. remote X), but it is actually lying. The
    // ARB_vertex_buffer_object functions silently no-op (!) when using
    // indirect rendering, leading to crashes. Fortunately, in that case, the
    // driver claims to not support ARB_vertex_buffer_object, so fail in that
    // case.
    DLOG(ERROR) << "GLES2 drivers do not support vertex buffer objects.";
    return GPU_NOT_UP_TO_SPEC;
  }

#ifndef DISABLE_FBO
  if (!GLEW_EXT_framebuffer_object) {
    DLOG(ERROR) << "GLES2 drivers do not support framebuffer objects.";
    return GPU_NOT_UP_TO_SPEC;
  }
#endif

  SetSupportsNPOT(GLEW_ARB_texture_non_power_of_two != 0);

#ifdef OS_MACOSX
  // The Radeon X1600 says it supports NPOT, but in most situations it doesn't.
  if (supports_npot() &&
      !strcmp("ATI Radeon X1600 OpenGLES2 Engine",
              reinterpret_cast<const char*>(::glGetString(GL_RENDERER))))
    SetSupportsNPOT(false);
#endif

  // Check for necessary extensions
  if (!GLEW_VERSION_2_0 && !GLEW_EXT_stencil_two_side) {
    DLOG(ERROR) << "Two sided stencil extension missing.";
  }
  if (!GLEW_VERSION_1_4 && !GLEW_EXT_blend_func_separate) {
    DLOG(ERROR) << "Separate blend func extension missing.";
  }
  if (!GLEW_VERSION_2_0 && !GLEW_EXT_blend_equation_separate) {
    DLOG(ERROR) << "Separate blend function extension missing.";
  }
#elif defined(GLES2_BACKEND_NATIVE_GLES2)
  // GLES specific initialization ?
#endif  // GLES2_BACKEND
  DLOG(INFO) << "OpenGLES2 Vendor: " << ::glGetString(GL_VENDOR);
  DLOG(INFO) << "OpenGLES2 Renderer: " << ::glGetString(GL_RENDERER);
  DLOG(INFO) << "OpenGLES2 Version: " << ::glGetString(GL_VERSION);
  DLOG(INFO) << "OpenGLES2 Extensions: " << ::glGetString(GL_EXTENSIONS);
  // get some limits for this profile.
  GLint max_vertex_attribs = 0;
  ::glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_vertex_attribs);
  DLOG(INFO) << "Max Vertex Attribs = " << max_vertex_attribs;
  // Initialize global GLES2 settings.
  // Tell GLES2 that texture buffers can be single-byte aligned.
  ::glPixelStorei(GL_PACK_ALIGNMENT, 1);
  ::glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
#if defined(GLES2_BACKEND_DESKTOP_GL)
  ::glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
  ::glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
#endif
  CHECK_GL_ERROR();
  // TODO(piman): by default, GLES2 has some minimal support for NPOT. Is it
  // enough for what we use ?
  SetSupportsNPOT(true);

  GLint viewport[4];
  ::glGetIntegerv(GL_VIEWPORT, &viewport[0]);
  SetClientSize(viewport[2], viewport[3]);
  CHECK_GL_ERROR();

#ifndef DISABLE_FBO
  ::glGenFramebuffersEXT(1, &render_surface_framebuffer_);
  CHECK_GL_ERROR();
#endif

  return SUCCESS;
}

// platform neutral destruction code
void RendererGLES2::DestroyCommonGLES2() {
  MakeCurrentLazy();
#ifndef DISABLE_FBO
  if (render_surface_framebuffer_) {
    ::glDeleteFramebuffersEXT(1, &render_surface_framebuffer_);
  }
#endif
}

#ifdef OS_WIN

namespace {

PIXELFORMATDESCRIPTOR kPixelFormatDescriptor = {
  sizeof(kPixelFormatDescriptor),    // Size of structure.
  1,                       // Default version.
  PFD_DRAW_TO_WINDOW |     // Window drawing support.
  PFD_SUPPORT_OPENGL |     // OpenGL support.
  PFD_DOUBLEBUFFER,        // Double buffering support (not stereo).
  PFD_TYPE_RGBA,           // RGBA color mode (not indexed).
  24,                      // 24 bit color mode.
  0, 0, 0, 0, 0, 0,        // Don't set RGB bits & shifts.
  8, 0,                    // 8 bit alpha
  0,                       // No accumulation buffer.
  0, 0, 0, 0,              // Ignore accumulation bits.
  24,                      // 24 bit z-buffer size.
  8,                       // 8-bit stencil buffer.
  0,                       // No aux buffer.
  PFD_MAIN_PLANE,          // Main drawing plane (not overlay).
  0,                       // Reserved.
  0, 0, 0,                 // Layer masks ignored.
};

LRESULT CALLBACK IntermediateWindowProc(HWND window,
                                        UINT message,
                                        WPARAM w_param,
                                        LPARAM l_param) {
  return ::DefWindowProc(window, message, w_param, l_param);
}

// Helper routine that returns the highest quality pixel format supported on
// the current platform.  Returns true upon success.
Renderer::InitStatus GetWindowsPixelFormat(HWND window,
                                           Features* features,
                                           int* pixel_format) {
  // We must initialize a GLES2 context before we can determine the multi-sampling
  // supported on the current hardware, so we create an intermediate window
  // and context here.
  HINSTANCE module_handle;
  if (!::GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT |
                           GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                           reinterpret_cast<wchar_t*>(IntermediateWindowProc),
                           &module_handle)) {
    return Renderer::INITIALIZATION_ERROR;
  }

  WNDCLASS intermediate_class;
  intermediate_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  intermediate_class.lpfnWndProc = IntermediateWindowProc;
  intermediate_class.cbClsExtra = 0;
  intermediate_class.cbWndExtra = 0;
  intermediate_class.hInstance = module_handle;
  intermediate_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  intermediate_class.hCursor = LoadCursor(NULL, IDC_ARROW);
  intermediate_class.hbrBackground = NULL;
  intermediate_class.lpszMenuName = NULL;
  intermediate_class.lpszClassName = L"Intermediate GLES2 Window";

  ATOM class_registration = ::RegisterClass(&intermediate_class);
  if (!class_registration) {
    return Renderer::INITIALIZATION_ERROR;
  }

  HWND intermediate_window = ::CreateWindow(
      reinterpret_cast<wchar_t*>(class_registration),
      L"",
      WS_OVERLAPPEDWINDOW,
      0, 0,
      CW_USEDEFAULT, CW_USEDEFAULT,
      NULL,
      NULL,
      NULL,
      NULL);

  if (!intermediate_window) {
    ::UnregisterClass(reinterpret_cast<wchar_t*>(class_registration),
                      module_handle);
    return Renderer::INITIALIZATION_ERROR;
  }

  HDC intermediate_dc = ::GetDC(intermediate_window);
  int format_index = ::ChoosePixelFormat(intermediate_dc,
                                         &kPixelFormatDescriptor);
  if (format_index == 0) {
    DLOG(ERROR) << "Unable to get the pixel format for GLES2 context.";
    ::ReleaseDC(intermediate_window, intermediate_dc);
    ::DestroyWindow(intermediate_window);
    ::UnregisterClass(reinterpret_cast<wchar_t*>(class_registration),
                      module_handle);
    return Renderer::INITIALIZATION_ERROR;
  }
  if (!::SetPixelFormat(intermediate_dc, format_index,
                        &kPixelFormatDescriptor)) {
    DLOG(ERROR) << "Unable to set the pixel format for GLES2 context.";
    ::ReleaseDC(intermediate_window, intermediate_dc);
    ::DestroyWindow(intermediate_window);
    ::UnregisterClass(reinterpret_cast<wchar_t*>(class_registration),
                      module_handle);
    return Renderer::INITIALIZATION_ERROR;
  }

  // Store the pixel format without multisampling.
  *pixel_format = format_index;
  HGLRC gl_context = ::wglCreateContext(intermediate_dc);
  if (::wglMakeCurrent(intermediate_dc, gl_context)) {
    // GLES2 context was successfully created and applied to the window's DC.
    // Startup GLEW, the GLES2 extensions wrangler.
    GLenum glew_error = ::glewInit();
    if (glew_error == GLEW_OK) {
      DLOG(INFO) << "Initialized GLEW " << ::glewGetString(GLEW_VERSION);
    } else {
      DLOG(ERROR) << "Unable to initialise GLEW : "
                  << ::glewGetErrorString(glew_error);
      ::wglMakeCurrent(intermediate_dc, NULL);
      ::wglDeleteContext(gl_context);
      ::ReleaseDC(intermediate_window, intermediate_dc);
      ::DestroyWindow(intermediate_window);
      ::UnregisterClass(reinterpret_cast<wchar_t*>(class_registration),
                        module_handle);
      return Renderer::INITIALIZATION_ERROR;
    }

    // If the multi-sample extensions are present, query the api to determine
    // the pixel format.
    if (!features->not_anti_aliased() &&
        WGLEW_ARB_pixel_format && WGLEW_ARB_multisample) {
      int pixel_attributes[] = {
        WGL_SAMPLES_ARB, 4,
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
        WGL_COLOR_BITS_ARB, 24,
        WGL_ALPHA_BITS_ARB, 8,
        WGL_DEPTH_BITS_ARB, 24,
        WGL_STENCIL_BITS_ARB, 8,
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
        WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
        0, 0};

      float pixel_attributes_f[] = {0, 0};
      int msaa_pixel_format;
      unsigned int num_formats;

      // Query for the highest sampling rate supported, starting at 4x.
      static const int kSampleCount[] = {4, 2};
      static const int kNumSamples = 2;
      for (int sample = 0; sample < kNumSamples; ++sample) {
        pixel_attributes[1] = kSampleCount[sample];
        if (GL_TRUE == ::wglChoosePixelFormatARB(intermediate_dc,
                                                 pixel_attributes,
                                                 pixel_attributes_f,
                                                 1,
                                                 &msaa_pixel_format,
                                                 &num_formats)) {
          *pixel_format = msaa_pixel_format;
          break;
        }
      }
    }
  }

  ::wglMakeCurrent(intermediate_dc, NULL);
  ::wglDeleteContext(gl_context);
  ::ReleaseDC(intermediate_window, intermediate_dc);
  ::DestroyWindow(intermediate_window);
  ::UnregisterClass(reinterpret_cast<wchar_t*>(class_registration),
                    module_handle);
  return Renderer::SUCCESS;
}

}  // unnamed namespace

Renderer::InitStatus RendererGLES2::InitPlatformSpecific(
    const DisplayWindow& display,
    bool off_screen) {
  const DisplayWindowWindows &display_platform =
      static_cast<const DisplayWindowWindows&>(display);

  DLOG(INFO) << "RendererGLES2 Init";

  // TODO(o3d): Add support for off-screen rendering using OpenGLES2.
  if (off_screen) {
    return INITIALIZATION_ERROR;
  }

  int pixel_format;
  InitStatus init_status;

  init_status = GetWindowsPixelFormat(display_platform.hwnd(),
                                      features(),
                                      &pixel_format);
  if (init_status != SUCCESS) {
      return init_status;
  }

  window_ = display_platform.hwnd();
  device_context_ = ::GetDC(window_);
  if (!::SetPixelFormat(device_context_, pixel_format,
                        &kPixelFormatDescriptor)) {
    DLOG(ERROR) << "Unable to set the pixel format for GLES2 context.";
    return INITIALIZATION_ERROR;
  }

  gl_context_ = ::wglCreateContext(device_context_);
  if (MakeCurrent()) {
    // Ensure that glew has been initialized for the created rendering context.
    init_status = InitCommonGLES2();
    if (init_status != SUCCESS) {
      DLOG(ERROR) << "Failed to initialize GLES2 rendering context.";
      return init_status;
    }
    if (WGLEW_ARB_multisample) {
      ::glEnable(GL_MULTISAMPLE_ARB);
    }
  } else {
    DLOG(ERROR) << "Failed to create the GLES2 Context.";
    return INITIALIZATION_ERROR;
  }
  CHECK_GL_ERROR();
  return SUCCESS;
}

// Deletes the GLES2 device.
void RendererGLES2::Destroy() {
  DLOG(INFO) << "Destroy RendererGLES2";
  DestroyCommonGLES2();
  if (device_context_) {
    CHECK_GL_ERROR();
    // Release the OpenGLES2 rendering context.
    ::wglMakeCurrent(device_context_, NULL);
    if (gl_context_) {
      ::wglDeleteContext(gl_context_);
      gl_context_ = NULL;
    }
    // release the hDC obtained through GetDC().
    ::ReleaseDC(window_, device_context_);
    device_context_ = NULL;
    window_ = NULL;
  }
  DLOG(INFO) << "Renderer destroyed.";
}

#endif  // OS_WIN

#ifdef OS_MACOSX

Renderer::InitStatus  RendererGLES2::InitPlatformSpecific(
    const DisplayWindow& display,
    bool /*off_screen*/) {
  const DisplayWindowMac &display_platform =
      static_cast<const DisplayWindowMac&>(display);
  // TODO(o3d): Add support for off screen rendering on the Mac.
  mac_agl_context_ = display_platform.agl_context();
  mac_cgl_context_ = display_platform.cgl_context();

  return InitCommonGLES2();
}

void RendererGLES2::Destroy() {
  DestroyCommonGLES2();
  // We only have to destroy agl contexts,
  // cgl contexts are not owned by us.
  if (mac_agl_context_) {
    ::aglDestroyContext(mac_agl_context_);
    mac_agl_context_ = NULL;
  }
}

#endif  // OS_MACOSX

#ifdef OS_LINUX
Renderer::InitStatus  RendererGLES2::InitPlatformSpecific(
    const DisplayWindow& display_window,
    bool off_screen) {
  const DisplayWindowLinux &display_platform =
      static_cast<const DisplayWindowLinux&>(display_window);
  Display *display = display_platform.display();
  Window window = display_platform.window();
#if defined(GLES2_BACKEND_DESKTOP_GL)
  XWindowAttributes attributes;
  ::XGetWindowAttributes(display, window, &attributes);
  XVisualInfo visual_info_template;
  visual_info_template.visualid = ::XVisualIDFromVisual(attributes.visual);
  int visual_info_count = 0;
  XVisualInfo *visual_info_list = ::XGetVisualInfo(display, VisualIDMask,
                                                   &visual_info_template,
                                                   &visual_info_count);
  DCHECK(visual_info_list);
  DCHECK_GT(visual_info_count, 0);
  context_ = 0;
  for (int i = 0; i < visual_info_count; ++i) {
    context_ = ::glXCreateContext(display, visual_info_list + i, 0,
                                  True);
    if (context_) break;
  }
  ::XFree(visual_info_list);
  if (!context_) {
    DLOG(ERROR) << "Couldn't create GLES2 context.";
    return INITIALIZATION_ERROR;
  }
  display_ = display;
  window_ = window;
  if (!MakeCurrent()) {
    ::glXDestroyContext(display, context_);
    context_ = 0;
    display_ = NULL;
    window_ = 0;
    DLOG(ERROR) << "Couldn't create GLES2 context.";
    return INITIALIZATION_ERROR;
  }

  InitStatus init_status = InitCommonGLES2();
  if (init_status != SUCCESS) {
    ::glXDestroyContext(display, context_);
    context_ = 0;
    display_ = NULL;
    window_ = 0;
  }
  return init_status;
#elif defined(GLES2_BACKEND_NATIVE_GLES2)
  EGLDisplay egl_display = eglGetDisplay(display);
  if (eglGetError() != EGL_SUCCESS) {
    DLOG(ERROR) << "eglGetDisplay failed.";
    return INITIALIZATION_ERROR;
  }

  EGLint major;
  EGLint minor;
  // TODO(piman): is it ok to do this several times ?
  if (!eglInitialize(egl_display, &major, &minor)) {
    DLOG(ERROR) << "eglInitialize failed.";
    return INITIALIZATION_ERROR;
  }
  DLOG(INFO) << "EGL vendor:" << eglQueryString(egl_display, EGL_VENDOR);
  DLOG(INFO) << "EGL version:" << eglQueryString(egl_display, EGL_VERSION);
  DLOG(INFO) << "EGL extensions:"
             << eglQueryString(egl_display, EGL_EXTENSIONS);
  DLOG(INFO) << "EGL client apis:"
             << eglQueryString(egl_display, EGL_CLIENT_APIS);

  EGLint attribs[] = {
#if 0
    // On some platforms X is started in 16-bit mode, and making a 32-bit
    // framebuffer causes tons of problems.
    // TODO(piman): fix this.
    EGL_RED_SIZE,       8,
    EGL_GREEN_SIZE,     8,
    EGL_BLUE_SIZE,      8,
    EGL_ALPHA_SIZE,     8,
    EGL_DEPTH_SIZE,     24,
    EGL_STENCIL_SIZE,   8,
#else
    EGL_RED_SIZE,       5,
    EGL_GREEN_SIZE,     6,
    EGL_BLUE_SIZE,      5,
    EGL_DEPTH_SIZE,     16,
    EGL_STENCIL_SIZE,   0,
#endif
    EGL_SURFACE_TYPE,   EGL_WINDOW_BIT,
    EGL_NONE
  };

  EGLint num_configs = -1;
  if (!eglGetConfigs(egl_display, NULL, 0, &num_configs)) {
    DLOG(ERROR) << "eglGetConfigs failed.";
    return INITIALIZATION_ERROR;
  }

  EGLConfig config;
  if (!eglChooseConfig(egl_display, attribs, &config, 1, &num_configs)) {
    DLOG(ERROR) << "eglChooseConfig failed.";
    return INITIALIZATION_ERROR;
  }

  EGLint red_size, green_size, blue_size, alpha_size, depth_size, stencil_size;
  eglGetConfigAttrib(egl_display, config, EGL_RED_SIZE, &red_size);
  eglGetConfigAttrib(egl_display, config, EGL_GREEN_SIZE, &green_size);
  eglGetConfigAttrib(egl_display, config, EGL_BLUE_SIZE, &blue_size);
  eglGetConfigAttrib(egl_display, config, EGL_ALPHA_SIZE, &alpha_size);
  eglGetConfigAttrib(egl_display, config, EGL_DEPTH_SIZE, &depth_size);
  eglGetConfigAttrib(egl_display, config, EGL_STENCIL_SIZE, &stencil_size);
  DLOG(INFO) << "R,G,B,A: " << red_size << "," << green_size
             << "," << blue_size << "," << alpha_size << " bits";
  DLOG(INFO) << "Depth: " << depth_size << " bits, Stencil:" << stencil_size
             << "bits";

  EGLSurface egl_surface = eglCreateWindowSurface(egl_display, config,
                                                  window, NULL);
  if (!egl_surface) {
    DLOG(ERROR) << "eglCreateWindowSurface failed.";
    return INITIALIZATION_ERROR;
  }

  EGLContext egl_context = eglCreateContext(egl_display, config, NULL, NULL);
  if (!egl_context) {
    DLOG(ERROR) << "eglCreateContext failed.";
    eglDestroySurface(egl_display, egl_surface);
    return INITIALIZATION_ERROR;
  }

  display_ = display;
  window_ = window;
  egl_display_ = egl_display;
  egl_surface_ = egl_surface;
  egl_context_ = egl_context;
  if (!MakeCurrent()) {
    eglDestroyContext(egl_display, egl_context);
    eglDestroySurface(egl_display, egl_surface);
    display_ = NULL;
    window_ = 0;
    egl_display_ = NULL;
    egl_surface_ = NULL;
    egl_context_ = NULL;
    return INITIALIZATION_ERROR;
  }

  EGLint width;
  EGLint height;
  eglQuerySurface(egl_display, egl_surface, EGL_WIDTH, &width);
  eglQuerySurface(egl_display, egl_surface, EGL_HEIGHT, &height);
  glViewport(0, 0, width, height);

  InitStatus init_status = InitCommonGLES2();
  if (init_status != SUCCESS) {
    Destroy();
    return INITIALIZATION_ERROR;
  }
  return init_status;
#else
#error Not Implemented
#endif
}

void RendererGLES2::Destroy() {
  DestroyCommonGLES2();
  if (display_) {
#if defined(GLES2_BACKEND_DESKTOP_GL)
    ::glXMakeCurrent(display_, 0, 0);
    if (context_) {
      ::glXDestroyContext(display_, context_);
      context_ = 0;
    }
#elif defined(GLES2_BACKEND_NATIVE_GLES2)
    if (egl_display_) {
      eglMakeCurrent(egl_display_, EGL_NO_SURFACE, EGL_NO_SURFACE,
                     EGL_NO_CONTEXT);
      if (egl_context_) {
        eglDestroyContext(egl_display_, egl_context_);
        egl_context_ = NULL;
      }
      if (egl_surface_) {
        eglDestroySurface(egl_display_, egl_surface_);
        egl_surface_ = NULL;
      }
      // TODO(piman): is it ok to do this if we have multiple clients ?
      eglTerminate(egl_display_);
      egl_display_ = NULL;
    }
#else
#error Not Implemented
#endif
    display_ = NULL;
    window_ = 0;
  }
}

#endif

bool RendererGLES2::MakeCurrent() {
#ifdef OS_WIN
  if (!device_context_ || !gl_context_) return false;
  bool result = ::wglMakeCurrent(device_context_, gl_context_) != 0;
  return result;
#endif
#ifdef OS_MACOSX
  if (mac_cgl_context_ != NULL) {
    ::CGLSetCurrentContext(mac_cgl_context_);
    return true;
  } else if (mac_agl_context_ != NULL) {
    ::aglSetCurrentContext(mac_agl_context_);
    return true;
  } else {
    return false;
  }
#endif
#ifdef OS_LINUX
#if defined(GLES2_BACKEND_DESKTOP_GL)
  if (context_ != NULL) {
    bool result = ::glXMakeCurrent(display_, window_, context_) == True;
    return result;
  } else {
    return false;
  }
#elif defined(GLES2_BACKEND_NATIVE_GLES2)
  if (egl_context_ != NULL) {
    EGLBoolean result = eglMakeCurrent(egl_display_, egl_surface_,
                                       egl_surface_, egl_context_);
    return result == EGL_TRUE;
  } else {
    return false;
  }
#else
#error Not Implemented
#endif
#endif
}

void RendererGLES2::PlatformSpecificClear(const Float4 &color,
                                          bool color_flag,
                                          float depth,
                                          bool depth_flag,
                                          int stencil,
                                          bool stencil_flag) {
  MakeCurrentLazy();
  ::glClearColor(color[0], color[1], color[2], color[3]);
  ::glClearDepth(depth);
  ::glClearStencil(stencil);

  ::glClear((color_flag   ? GL_COLOR_BUFFER_BIT   : 0) |
            (depth_flag   ? GL_DEPTH_BUFFER_BIT   : 0) |
            (stencil_flag ? GL_STENCIL_BUFFER_BIT : 0));
  CHECK_GL_ERROR();
}

// Updates the helper constant used for the D3D -> GLES2 remapping.
// See effect_gles2.cc for details.
void RendererGLES2::UpdateHelperConstant(float width, float height) {
  MakeCurrentLazy();
  // If render-targets are active, pass -1 to invert the Y axis.  OpenGLES2 uses
  // a different viewport orientation than DX.  Without the inversion, the
  // output of render-target rendering will be upside down.
  CHECK_GL_ERROR();

  if (RenderSurfaceActive()) {
    dx_clipping_[0] = 1.f / width;
    dx_clipping_[1] = -1.f / height;
    dx_clipping_[2] = 2.f;
    dx_clipping_[3] = -1.f;
  } else {
    dx_clipping_[0] = (1.0f + 2.0f * (-dest_x_offset())) / width;
    dx_clipping_[1] = (-1.0f + 2.0f * dest_y_offset()) / height;
    dx_clipping_[2] = 2.0f;
    dx_clipping_[3] = 1.0f;
  }
}

// Programs the helper constants into the hardware.
void RendererGLES2::UpdateDxClippingUniform(GLint location) {
  // For some reason, if location is -1 an error is signalled, despite the spec
  // saying it is OK.
  if (location != -1)
    glUniform4fv(location, 1, dx_clipping_);
  CHECK_GL_ERROR();
}

void RendererGLES2::SetViewportInPixels(int left,
                                        int top,
                                        int width,
                                        int height,
                                        float min_z,
                                        float max_z) {
  MakeCurrentLazy();
  int vieport_top =
      RenderSurfaceActive() ? top : display_height() - top - height;
  ::glViewport(left, vieport_top, width, height);
  UpdateHelperConstant(static_cast<float>(width), static_cast<float>(height));

  // If it's the full client area turn off scissor test for speed.
  if (left == 0 &&
      top == 0 &&
      width == display_width() &&
      height == display_height()) {
    ::glDisable(GL_SCISSOR_TEST);
  } else {
    ::glScissor(left, vieport_top, width, height);
    ::glEnable(GL_SCISSOR_TEST);
  }
  ::glDepthRange(min_z, max_z);
}

// Resizes the viewport.
void RendererGLES2::Resize(int width, int height) {
  MakeCurrentLazy();
  SetClientSize(width, height);
  CHECK_GL_ERROR();
}

bool RendererGLES2::GoFullscreen(const DisplayWindow& display,
                                 int mode_id) {
#ifdef OS_LINUX
  // This actually just switches the GLX context to the new window. The real
  // work is in main_linux.cc.
  const DisplayWindowLinux &display_platform =
      static_cast<const DisplayWindowLinux&>(display);
  display_ = display_platform.display();
  window_ = display_platform.window();
  if (!MakeCurrent()) {
    return false;
  }
#endif
  fullscreen_ = true;
  return true;
}

bool RendererGLES2::CancelFullscreen(const DisplayWindow& display,
                                     int width, int height) {
#ifdef OS_LINUX
  // This actually just switches the GLX context to the old window. The real
  // work is in main_linux.cc.
  const DisplayWindowLinux &display_platform =
      static_cast<const DisplayWindowLinux&>(display);
  display_ = display_platform.display();
  window_ = display_platform.window();
  if (!MakeCurrent()) {
    return false;
  }
#endif
  fullscreen_ = false;
  return true;
}

void RendererGLES2::GetDisplayModes(std::vector<DisplayMode> *modes) {
#ifdef OS_MACOSX
  // Mac is supposed to call a different function in plugin_mac.mm instead.
  DLOG(FATAL) << "Not supposed to be called";
#endif
  // On all other platforms this is unimplemented. Linux only supports
  // DISPLAY_MODE_DEFAULT for now.
  modes->clear();
}

bool RendererGLES2::GetDisplayMode(int id, DisplayMode *mode) {
#ifdef OS_MACOSX
  // Mac is supposed to call a different function in plugin_mac.mm instead.
  DLOG(FATAL) << "Not supposed to be called";
  return false;
#elif defined(OS_LINUX)
  if (id == DISPLAY_MODE_DEFAULT) {
    // Don't need to know any of this on Linux.
    mode->Set(0, 0, 0, id);
    return true;
  } else {
    // There are no other valid ids until we implement GetDisplayModes() and
    // mode switching.
    return false;
  }
#else
  return false;
#endif
}

bool RendererGLES2::PlatformSpecificStartRendering() {
  DLOG_FIRST_N(INFO, 10) << "RendererGLES2 StartRendering";
  MakeCurrentLazy();

  // Currently always returns true.
  // Should be modified if current behavior changes.
  CHECK_GL_ERROR();
  return true;
}

// Clears the color, depth and stncil buffers and prepares GLES2 for rendering
// the frame.
// Returns true on success.
bool RendererGLES2::PlatformSpecificBeginDraw() {
  DLOG_FIRST_N(INFO, 10) << "RendererGLES2 BeginDraw";

  MakeCurrentLazy();

  // Currently always returns true.
  // Should be modified if current behavior changes.
  CHECK_GL_ERROR();
  return true;
}

// Assign the surface arguments to the renderer, and update the stack
// of pushed surfaces.
void RendererGLES2::SetRenderSurfacesPlatformSpecific(
    const RenderSurface* surface,
    const RenderDepthStencilSurface* surface_depth) {
  // TODO(o3d):  This routine re-uses a single global framebuffer object for
  // all RenderSurface rendering.  Because of the validation checks performed
  // at attachment-change time, it may be more performant to create a pool
  // of framebuffer objects with different attachment characterists and
  // switch between them here.
  MakeCurrentLazy();
#ifndef DISABLE_FBO
  ::glBindFramebufferEXT(GL_FRAMEBUFFER, render_surface_framebuffer_);
  if (!InstallFramebufferObjects(surface, surface_depth)) {
    O3D_ERROR(service_locator())
        << "Failed to bind OpenGLES2 render target objects:"
        << surface->name() <<", "<< surface_depth->name();
  }
#endif
  // RenderSurface rendering is performed with an inverted Y, so the front
  // face winding must be changed to clock-wise.  See comments for
  // UpdateHelperConstant.
  glFrontFace(GL_CW);
}

void RendererGLES2::SetBackBufferPlatformSpecific() {
  MakeCurrentLazy();
#ifndef DISABLE_FBO
  // Bind the default context, and restore the default front-face winding.
  ::glBindFramebufferEXT(GL_FRAMEBUFFER, 0);
#endif
  glFrontFace(GL_CCW);
}

// Executes a post rendering step
void RendererGLES2::PlatformSpecificEndDraw() {
  DLOG_FIRST_N(INFO, 10) << "RendererGLES2 EndDraw";
  DCHECK(IsCurrent());
}

// Swaps the buffers.
void RendererGLES2::PlatformSpecificFinishRendering() {
  DLOG_FIRST_N(INFO, 10) << "RendererGLES2 FinishRendering";
  DCHECK(IsCurrent());
  ::glFlush();
  CHECK_GL_ERROR();
}

void RendererGLES2::PlatformSpecificPresent() {
  DLOG_FIRST_N(INFO, 10) << "RendererGLES2 Present";
  DCHECK(IsCurrent());
#ifdef OS_WIN
  ::SwapBuffers(device_context_);
#endif
#ifdef OS_MACOSX
#ifdef USE_AGL_DOUBLE_BUFFER
  if (mac_agl_context_) {
    ::aglSwapBuffers(mac_agl_context_);
  }
#endif
#endif
#ifdef OS_LINUX
#if defined(GLES2_BACKEND_DESKTOP_GL)
  ::glXSwapBuffers(display_, window_);
#elif defined(GLES2_BACKEND_NATIVE_GLES2)
  eglSwapBuffers(egl_display_, egl_surface_);
#else
#error Not Implemented
#endif  // GLES2_BACKEND_xxx
#endif
}

StreamBank::Ref RendererGLES2::CreateStreamBank() {
  return StreamBank::Ref(new StreamBankGLES2(service_locator()));
}

Primitive::Ref RendererGLES2::CreatePrimitive() {
  return Primitive::Ref(new PrimitiveGLES2(service_locator()));
}

DrawElement::Ref RendererGLES2::CreateDrawElement() {
  return DrawElement::Ref(new DrawElementGLES2(service_locator()));
}

void RendererGLES2::SetStencilStates(GLenum face,
                                     const StencilStates& stencil_state) {
  DCHECK(IsCurrent());
  if (face == GL_FRONT_AND_BACK) {
    ::glStencilFunc(stencil_state.func_,
                    stencil_ref_,
                    stencil_mask_[READ_MASK]);
    ::glStencilOp(stencil_state.op_[StencilStates::FAIL_OP],
                  stencil_state.op_[StencilStates::ZFAIL_OP],
                  stencil_state.op_[StencilStates::PASS_OP]);
    ::glStencilMask(stencil_mask_[WRITE_MASK]);
  } else if (GLEW_VERSION_2_0) {
    ::glStencilFuncSeparate(face,
                            stencil_state.func_,
                            stencil_ref_,
                            stencil_mask_[READ_MASK]);
    ::glStencilOpSeparate(face,
                          stencil_state.op_[StencilStates::FAIL_OP],
                          stencil_state.op_[StencilStates::ZFAIL_OP],
                          stencil_state.op_[StencilStates::PASS_OP]);
    ::glStencilMaskSeparate(face,
                            stencil_mask_[WRITE_MASK]);
  }
#if defined(GLES2_BACKEND_DESKTOP_GL)
  else if (GLEW_EXT_stencil_two_side) {
    ::glEnable(GL_STENCIL_TEST_TWO_SIDE_EXT);
    ::glActiveStencilFaceEXT(face);
    ::glStencilFunc(stencil_state.func_,
                    stencil_ref_,
                    stencil_mask_[READ_MASK]);
    ::glStencilOp(stencil_state.op_[StencilStates::FAIL_OP],
                  stencil_state.op_[StencilStates::ZFAIL_OP],
                  stencil_state.op_[StencilStates::PASS_OP]);
    ::glStencilMask(stencil_mask_[WRITE_MASK]);
    ::glDisable(GL_STENCIL_TEST_TWO_SIDE_EXT);
  }
#endif
  CHECK_GL_ERROR();
}

void RendererGLES2::ApplyDirtyStates() {
  MakeCurrentLazy();
  DCHECK(IsCurrent());
  // Set blend settings.
  if (alpha_blend_settings_changed_) {
    if (separate_alpha_blend_enable_) {
      if (GLEW_VERSION_1_4) {
        ::glBlendFuncSeparate(blend_function_[SRC][RGB],
                              blend_function_[DST][RGB],
                              blend_function_[SRC][ALPHA],
                              blend_function_[DST][ALPHA]);
      }
#if defined(GLES2_BACKEND_DESKTOP_GL)
      else if (GLEW_EXT_blend_func_separate) {
        ::glBlendFuncSeparateEXT(blend_function_[SRC][RGB],
                                 blend_function_[DST][RGB],
                                 blend_function_[SRC][ALPHA],
                                 blend_function_[DST][ALPHA]);
      }
#endif
      if (GLEW_VERSION_2_0) {
        ::glBlendEquationSeparate(blend_equation_[RGB],
                                  blend_equation_[ALPHA]);
      }
#if defined(GLES2_BACKEND_DESKTOP_GL)
      else if (GLEW_EXT_blend_equation_separate) {
        ::glBlendEquationSeparateEXT(blend_equation_[RGB],
                                     blend_equation_[ALPHA]);
      }
#endif
    } else {
      ::glBlendFunc(blend_function_[SRC][RGB],
                    blend_function_[DST][RGB]);
#if defined(GLES2_BACKEND_DESKTOP_GL)
      if (::glBlendEquation != NULL)
#endif
        ::glBlendEquation(blend_equation_[RGB]);
    }
    alpha_blend_settings_changed_ = false;
  }

#if defined(GLES2_BACKEND_DESKTOP_GL)
  // Set alpha settings.
  if (alpha_function_ref_changed_) {
    ::glAlphaFunc(alpha_function_, alpha_ref_);
    alpha_function_ref_changed_ = false;
  }
#endif

  // Set stencil settings.
  if (stencil_settings_changed_) {
    if (separate_stencil_settings_enable_) {
      SetStencilStates(GL_FRONT, stencil_settings_[FRONT]);
      SetStencilStates(GL_BACK, stencil_settings_[BACK]);
    } else {
      SetStencilStates(GL_FRONT_AND_BACK, stencil_settings_[FRONT]);
    }
    stencil_settings_changed_ = false;
  }

  if (polygon_offset_changed_) {
    bool enable = (polygon_offset_factor_ != 0.f) ||
                  (polygon_offset_bias_ != 0.f);
    if (enable) {
#if defined(GLES2_BACKEND_DESKTOP_GL)
      ::glEnable(GL_POLYGON_OFFSET_POINT);
      ::glEnable(GL_POLYGON_OFFSET_LINE);
#endif
      ::glEnable(GL_POLYGON_OFFSET_FILL);
      ::glPolygonOffset(polygon_offset_factor_, polygon_offset_bias_);
    } else {
#if defined(GLES2_BACKEND_DESKTOP_GL)
      ::glDisable(GL_POLYGON_OFFSET_POINT);
      ::glDisable(GL_POLYGON_OFFSET_LINE);
#endif
      ::glDisable(GL_POLYGON_OFFSET_FILL);
    }
    polygon_offset_changed_ = false;
  }
  CHECK_GL_ERROR();
}

VertexBuffer::Ref RendererGLES2::CreateVertexBuffer() {
  DLOG(INFO) << "RendererGLES2 CreateVertexBuffer";
  MakeCurrentLazy();
  return VertexBuffer::Ref(new VertexBufferGLES2(service_locator()));
}

IndexBuffer::Ref RendererGLES2::CreateIndexBuffer() {
  DLOG(INFO) << "RendererGLES2 CreateIndexBuffer";
  MakeCurrentLazy();
  return IndexBuffer::Ref(new IndexBufferGLES2(service_locator()));
}

Effect::Ref RendererGLES2::CreateEffect() {
  DLOG(INFO) << "RendererGLES2 CreateEffect";
  MakeCurrentLazy();
  return Effect::Ref(new EffectGLES2(service_locator()));
}

Sampler::Ref RendererGLES2::CreateSampler() {
  return Sampler::Ref(new SamplerGLES2(service_locator()));
}

ParamCache* RendererGLES2::CreatePlatformSpecificParamCache() {
  return new ParamCacheGLES2(semantic_manager_.Get(), this);
}


Texture2D::Ref RendererGLES2::CreatePlatformSpecificTexture2D(
    int width,
    int height,
    Texture::Format format,
    int levels,
    bool enable_render_surfaces) {
  DLOG(INFO) << "RendererGLES2 CreateTexture2D";
  MakeCurrentLazy();
  return Texture2D::Ref(Texture2DGLES2::Create(service_locator(),
                                               format,
                                               levels,
                                               width,
                                               height,
                                               enable_render_surfaces));
}

TextureCUBE::Ref RendererGLES2::CreatePlatformSpecificTextureCUBE(
    int edge_length,
    Texture::Format format,
    int levels,
    bool enable_render_surfaces) {
  DLOG(INFO) << "RendererGLES2 CreateTextureCUBE";
  MakeCurrentLazy();
  return TextureCUBE::Ref(TextureCUBEGLES2::Create(service_locator(),
                                                   format,
                                                   levels,
                                                   edge_length,
                                                   enable_render_surfaces));
}

RenderDepthStencilSurface::Ref RendererGLES2::CreateDepthStencilSurface(
    int width,
    int height) {
  return RenderDepthStencilSurface::Ref(
      new RenderDepthStencilSurfaceGLES2(service_locator(),
                                         width,
                                         height));
}

const int* RendererGLES2::GetRGBAUByteNSwizzleTable() {
  static int swizzle_table[] = { 0, 1, 2, 3, };
  return swizzle_table;
}

// This is a factory function for creating Renderer objects.  Since
// we're implementing GLES2, we only ever return a GLES2 renderer.
Renderer* Renderer::CreateDefaultRenderer(ServiceLocator* service_locator) {
  return RendererGLES2::CreateDefault(service_locator);
}

}  // namespace o3d
