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


// This file contains the definition of the RendererGL class that
// implements the abstract Renderer API using OpenGL and the Cg
// Runtime.


#include "core/cross/gl/renderer_gl.h"

#include "core/cross/error.h"
#include "core/cross/gl/buffer_gl.h"
#include "core/cross/gl/draw_element_gl.h"
#include "core/cross/gl/effect_gl.h"
#include "core/cross/gl/param_cache_gl.h"
#include "core/cross/gl/primitive_gl.h"
#include "core/cross/gl/render_surface_gl.h"
#include "core/cross/gl/sampler_gl.h"
#include "core/cross/gl/stream_bank_gl.h"
#include "core/cross/gl/texture_gl.h"
#include "core/cross/gl/utils_gl-inl.h"
#include "core/cross/gl/utils_gl.h"
#include "core/cross/material.h"
#include "core/cross/semantic_manager.h"
#include "core/cross/features.h"
#include "core/cross/shape.h"
#include "core/cross/types.h"

namespace o3d {

	namespace {

		GLenum ConvertCmpFunc(State::Comparison cmp) {
			switch(cmp) {
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

		GLenum ConvertFillMode(State::Fill mode) {
			switch(mode) {
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

		GLenum ConvertBlendFunc(State::BlendingFunction blend_func) {
			switch(blend_func) {
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
			switch(blend_equation) {
			case State::BLEND_ADD:
				return GL_FUNC_ADD;
			case State::BLEND_SUBTRACT:
				return GL_FUNC_SUBTRACT;
			case State::BLEND_REVERSE_SUBTRACT:
				return GL_FUNC_REVERSE_SUBTRACT;
			case State::BLEND_MIN:
				return GL_MIN;
			case State::BLEND_MAX:
				return GL_MAX;
			default:
				break;
			}

			return GL_FUNC_ADD;
		}

		GLenum ConvertStencilOp(State::StencilOperation stencil_func) {
			switch(stencil_func) {
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

// Helper routine that will bind the surfaces stored in the RenderSurface and
// RenderDepthStencilSurface arguments to the current OpenGL context.
// Returns true upon success.
// Note:  This routine assumes that a frambuffer object is presently bound
// to the context.
		bool InstallFramebufferObjects(const RenderSurface* surface,
		                               const RenderDepthStencilSurface* surface_depth) {
#ifndef NDEBUG
			GLint bound_framebuffer;
			::glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &bound_framebuffer);
			O3D_ASSERT(bound_framebuffer != 0);
#endif
			// Reset the bound attachments to the current framebuffer object.
			::glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
			                               GL_COLOR_ATTACHMENT0_EXT,
			                               GL_RENDERBUFFER_EXT,
			                               0);
			::glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
			                               GL_DEPTH_ATTACHMENT_EXT,
			                               GL_RENDERBUFFER_EXT,
			                               0);
			::glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
			                               GL_STENCIL_ATTACHMENT_EXT,
			                               GL_RENDERBUFFER_EXT,
			                               0);

			if(surface) {
				const RenderSurfaceGL* gl_surface =
				    down_cast<const RenderSurfaceGL*>(surface);
				Texture* texture = gl_surface->texture();
				GLuint handle = static_cast<GLuint>(reinterpret_cast<intptr_t>(
				                                        texture->GetTextureHandle()));

				if(texture->IsA(Texture2D::GetApparentClass())) {
					::glFramebufferTexture2DEXT(
					    GL_FRAMEBUFFER_EXT,
					    GL_COLOR_ATTACHMENT0_EXT,
					    GL_TEXTURE_2D,
					    handle,
					    gl_surface->mip_level());
				}
				else if(texture->IsA(TextureCUBE::GetApparentClass())) {
					::glFramebufferTexture2DEXT(
					    GL_FRAMEBUFFER_EXT,
					    GL_COLOR_ATTACHMENT0_EXT,
					    gl_surface->cube_face(),
					    handle,
					    gl_surface->mip_level());
				}
			}

			if(surface_depth) {
				// Bind both the depth and stencil attachments.
				const RenderDepthStencilSurfaceGL* gl_surface =
				    down_cast<const RenderDepthStencilSurfaceGL*>(surface_depth);
				::glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
				                               GL_DEPTH_ATTACHMENT_EXT,
				                               GL_RENDERBUFFER_EXT,
				                               gl_surface->depth_buffer());
				::glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
				                               GL_STENCIL_ATTACHMENT_EXT,
				                               GL_RENDERBUFFER_EXT,
				                               gl_surface->stencil_buffer());
			}

			GLenum framebuffer_status = ::glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);

			if(GL_FRAMEBUFFER_COMPLETE_EXT != framebuffer_status) {
				return false;
			}

			CHECK_GL_ERROR();
			return true;
		}

// Helper routine that returns a pointer to the non-NULL entry in the renderer's
// stack of bound surfaces.
		const RenderSurfaceBase* GetValidRenderSurface(
		    const std::pair<RenderSurface*, RenderDepthStencilSurface*> &stack_entry) {
			if(stack_entry.first) {
				return stack_entry.first;
			}
			else {
				return stack_entry.second;
			}
		}

	}  // unnamed namespace

// This class wraps StateHandler to make it typesafe.
	template <typename T>
	class TypedStateHandler : public RendererGL::StateHandler {
	public:
		// Override this function to set a specific state.
		// Parameters:
		//   renderer: The platform specific renderer.
		//   param: A concrete param with state data.
		virtual void SetStateFromTypedParam(RendererGL* renderer, T* param) const = 0;

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
			RendererGL* renderer_gl = down_cast<RendererGL*>(renderer);
			// This is safe because State guarntees Params match by type.
			O3D_ASSERT(param->IsA(T::GetApparentClass()));
			SetStateFromTypedParam(renderer_gl, down_cast<T*>(param));
		}
	};

// A template the generates a handler for enable/disable states.
// Parameters:
//   state_constant: GLenum of state we want to enable/disable
	template <GLenum state_constant>
	class StateEnableHandler : public TypedStateHandler<ParamBoolean> {
	public:
		virtual void SetStateFromTypedParam(RendererGL* renderer,
		                                    ParamBoolean* param) const {
			if(param->value()) {
				::glEnable(state_constant);
			}
			else {
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
		virtual void SetStateFromTypedParam(RendererGL* renderer,
		                                    ParamBoolean* param) const {
			var_ = param->value();
		}
	private:
		bool& var_;
		bool& changed_var_;
	};

	class ZWriteEnableHandler : public TypedStateHandler<ParamBoolean> {
	public:
		virtual void SetStateFromTypedParam(RendererGL* renderer,
		                                    ParamBoolean* param) const {
			::glDepthMask(param->value());
		}
	};

	class AlphaReferenceHandler : public TypedStateHandler<ParamFloat> {
	public:
		virtual void SetStateFromTypedParam(RendererGL* renderer,
		                                    ParamFloat* param) const {
			float refFloat = param->value();

			// cap the float to the required range
			if(refFloat < 0.0f) {
				refFloat = 0.0f;
			}
			else if(refFloat > 1.0f) {
				refFloat = 1.0f;
			}

			renderer->alpha_function_ref_changed_ = true;
			renderer->alpha_ref_ = refFloat;
		}
	};

	class CullModeHandler : public TypedStateHandler<ParamInteger> {
	public:
		virtual void SetStateFromTypedParam(RendererGL* renderer,
		                                    ParamInteger* param) const {
			State::Cull cull = static_cast<State::Cull>(param->value());

			switch(cull) {
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
		virtual void SetStateFromTypedParam(RendererGL* renderer,
		                                    ParamFloat* param) const {
			renderer->polygon_offset_factor_ = param->value();
			renderer->polygon_offset_changed_ = true;
		}
	};

	class PolygonOffset2Handler : public TypedStateHandler<ParamFloat> {
	public:
		virtual void SetStateFromTypedParam(RendererGL* renderer,
		                                    ParamFloat* param) const {
			renderer->polygon_offset_bias_ = param->value();
			renderer->polygon_offset_changed_ = true;
		}
	};

	class FillModeHandler : public TypedStateHandler<ParamInteger> {
	public:
		virtual void SetStateFromTypedParam(RendererGL* renderer,
		                                    ParamInteger* param) const {
			::glPolygonMode(GL_FRONT_AND_BACK,
			                ConvertFillMode(static_cast<State::Fill>(param->value())));
		}
	};

	class ZFunctionHandler : public TypedStateHandler<ParamInteger> {
	public:
		virtual void SetStateFromTypedParam(RendererGL* renderer,
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
		virtual void SetStateFromTypedParam(RendererGL* renderer,
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
		virtual void SetStateFromTypedParam(RendererGL* renderer,
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
		virtual void SetStateFromTypedParam(RendererGL* renderer,
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
		virtual void SetStateFromTypedParam(RendererGL* renderer,
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
		virtual void SetStateFromTypedParam(RendererGL* renderer,
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
		virtual void SetStateFromTypedParam(RendererGL* renderer,
		                                    ParamInteger* param) const {
			renderer->stencil_settings_changed_ = true;
			renderer->stencil_mask_[mask_index_] = param->value();
		}
	private:
		int mask_index_;
	};

	class ColorWriteEnableHandler : public TypedStateHandler<ParamInteger> {
	public:
		virtual void SetStateFromTypedParam(RendererGL* renderer,
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
		virtual void SetStateFromTypedParam(RendererGL* renderer,
		                                    ParamBoolean* param) const {
			if(param->value()) {
				::glEnable(GL_POINT_SPRITE);
				// TODO: It's not clear from D3D docs that point sprites affect
				// TEXCOORD0, but that's my guess. Check that.
				::glActiveTextureARB(GL_TEXTURE0);
				::glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
			}
			else {
				::glActiveTextureARB(GL_TEXTURE0);
				::glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_FALSE);
				::glDisable(GL_POINT_SPRITE);
			}
		}
	};

	class PointSizeHandler : public TypedStateHandler<ParamFloat> {
	public:
		virtual void SetStateFromTypedParam(RendererGL* renderer,
		                                    ParamFloat* param) const {
			::glPointSize(param->value());
		}
	};

	RendererGL* RendererGL::CreateDefault(ServiceLocator* service_locator) {
		return new RendererGL(service_locator);
	}

	RendererGL::RendererGL(ServiceLocator* service_locator)
		: Renderer(service_locator),
		  semantic_manager_(service_locator),
		  fullscreen_(0),
#ifdef OS_LINUX
		  display_(NULL),
		  window_(0),
		  context_(0),
#endif
#ifdef OS_MACOSX
		  mac_agl_context_(0),
		  mac_cgl_context_(0),
#endif
		  render_surface_framebuffer_(0),
		  cg_context_(NULL),
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
		O3D_LOG(INFO) << "RendererGL Construct";

		// Setup default state values.
		for(int ii = 0; ii < 2; ++ii) {
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
		AddStateHandler(State::kAlphaTestEnableParamName,
		                new StateEnableHandler<GL_ALPHA_TEST>);
		AddStateHandler(State::kAlphaReferenceParamName,
		                new AlphaReferenceHandler);
		AddStateHandler(State::kAlphaComparisonFunctionParamName,
		                new ComparisonFunctionHandler(&alpha_function_,
		                        &alpha_function_ref_changed_));
		AddStateHandler(State::kCullModeParamName,
		                new CullModeHandler);
		AddStateHandler(State::kDitherEnableParamName,
		                new StateEnableHandler<GL_DITHER>);
		AddStateHandler(State::kLineSmoothEnableParamName,
		                new StateEnableHandler<GL_LINE_SMOOTH>);
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

	RendererGL::~RendererGL() {
		Destroy();
	}

// platform neutral initialization code
//
	Renderer::InitStatus RendererGL::InitCommonGL() {
		GLenum glew_error = glewInit();

		if(glew_error != GLEW_OK) {
			O3D_LOG(ERROR) << "Unable to initialise GLEW : "
			               << ::glewGetErrorString(glew_error);
			return INITIALIZATION_ERROR;
		}

		// Check to see that we can use the OpenGL vertex attribute APIs
		// TODO:  We should return false if this check fails, but because some
		// Intel hardware does not support OpenGL 2.0, yet does support all of the
		// extensions we require, we only log an error.  A future CL should change
		// this check to ensure that all of the extension strings we require are
		// present.
		if(!GLEW_VERSION_2_0) {
			O3D_LOG(ERROR) << "GL drivers do not have OpenGL 2.0 functionality.";
		}

		if(!GLEW_ARB_vertex_buffer_object) {
			// NOTE: Linux NVidia drivers claim to support OpenGL 2.0 when using
			// indirect rendering (e.g. remote X), but it is actually lying. The
			// ARB_vertex_buffer_object functions silently no-op (!) when using
			// indirect rendering, leading to crashes. Fortunately, in that case, the
			// driver claims to not support ARB_vertex_buffer_object, so fail in that
			// case.
			O3D_LOG(ERROR) << "GL drivers do not support vertex buffer objects.";
			return GPU_NOT_UP_TO_SPEC;
		}

		if(!GLEW_EXT_framebuffer_object) {
			O3D_LOG(ERROR) << "GL drivers do not support framebuffer objects.";
			return GPU_NOT_UP_TO_SPEC;
		}

		SetSupportsNPOT(GLEW_ARB_texture_non_power_of_two != 0);
#ifdef OS_MACOSX

		// The Radeon X1600 says it supports NPOT, but in most situations it doesn't.
		if(supports_npot() &&
		        !strcmp("ATI Radeon X1600 OpenGL Engine",
		                reinterpret_cast<const char*>(::glGetString(GL_RENDERER))))
			SetSupportsNPOT(false);

#endif

		// Check for necessary extensions
		if(!GLEW_VERSION_2_0 && !GLEW_EXT_stencil_two_side) {
			O3D_LOG(ERROR) << "Two sided stencil extension missing.";
		}

		if(!GLEW_VERSION_1_4 && !GLEW_EXT_blend_func_separate) {
			O3D_LOG(ERROR) << "Separate blend func extension missing.";
		}

		if(!GLEW_VERSION_2_0 && !GLEW_EXT_blend_equation_separate) {
			O3D_LOG(ERROR) << "Separate blend function extension missing.";
		}

		// create a Cg Runtime.
		cg_context_ = cgCreateContext();
		DLOG_CG_ERROR("Creating Cg context");
		// NOTE: the first CGerror number after the recreation of a
		// CGcontext (the second time through) seems to be trashed. Please
		// ignore any "CG ERROR: Invalid context handle." message on this
		// function - Invalid context handle isn't one of therror states of
		// cgCreateContext().
		O3D_LOG(INFO) << "OpenGL Vendor: " << ::glGetString(GL_VENDOR);
		O3D_LOG(INFO) << "OpenGL Renderer: " << ::glGetString(GL_RENDERER);
		O3D_LOG(INFO) << "OpenGL Version: " << ::glGetString(GL_VERSION);
		O3D_LOG(INFO) << "Cg Version: " << cgGetString(CG_VERSION);
		cg_vertex_profile_ = cgGLGetLatestProfile(CG_GL_VERTEX);
		cgGLSetOptimalOptions(cg_vertex_profile_);
		O3D_LOG(INFO) << "Best Cg vertex profile = "
		              << cgGetProfileString(cg_vertex_profile_);
		cg_fragment_profile_ = cgGLGetLatestProfile(CG_GL_FRAGMENT);
		cgGLSetOptimalOptions(cg_fragment_profile_);
		O3D_LOG(INFO) << "Best Cg fragment profile = "
		              << cgGetProfileString(cg_fragment_profile_);
		// Set up all Cg State Assignments for OpenGL.
		cgGLRegisterStates(cg_context_);
		DLOG_CG_ERROR("Registering GL StateAssignments");
		cgGLSetDebugMode(CG_FALSE);
		// Enable the profiles we use.
		cgGLEnableProfile(CG_PROFILE_ARBVP1);
		cgGLEnableProfile(CG_PROFILE_ARBFP1);
		// get some limits for this profile.
		GLint max_vertex_attribs = 0;
		::glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_vertex_attribs);
		O3D_LOG(INFO) << "Max Vertex Attribs = " << max_vertex_attribs;
		// Initialize global GL settings.
		// Tell GL that texture buffers can be single-byte aligned.
		::glPixelStorei(GL_PACK_ALIGNMENT, 1);
		::glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		::glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		::glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
		CHECK_GL_ERROR();
		GLint viewport[4];
		::glGetIntegerv(GL_VIEWPORT, &viewport[0]);
		SetClientSize(viewport[2], viewport[3]);
		CHECK_GL_ERROR();
		::glGenFramebuffersEXT(1, &render_surface_framebuffer_);
		CHECK_GL_ERROR();
		return SUCCESS;
	}

// platform neutral destruction code
	void RendererGL::DestroyCommonGL() {
		MakeCurrentLazy();

		if(render_surface_framebuffer_) {
			::glDeleteFramebuffersEXT(1, &render_surface_framebuffer_);
		}

		if(cg_context_) {
			cgDestroyContext(cg_context_);
			cg_context_ = NULL;
		}
	}

#ifdef OS_MACOSX

	Renderer::InitStatus  RendererGL::InitPlatformSpecific(
	    const DisplayWindow& display,
	    bool /*off_screen*/) {
		const DisplayWindowMac& display_platform =
		    static_cast<const DisplayWindowMac&>(display);
		// TODO: Add support for off screen rendering on the Mac.
		mac_agl_context_ = display_platform.agl_context();
		mac_cgl_context_ = display_platform.cgl_context();
		return InitCommonGL();
	}

	void RendererGL::Destroy() {
		DestroyCommonGL();

		// We only have to destroy agl contexts,
		// cgl contexts are not owned by us.
		if(mac_agl_context_) {
			::aglDestroyContext(mac_agl_context_);
			mac_agl_context_ = NULL;
		}
	}

#endif  // OS_MACOSX

#ifdef OS_LINUX
	Renderer::InitStatus  RendererGL::InitPlatformSpecific(
	    const DisplayWindow& display_window,
	    bool off_screen) {
		const DisplayWindowLinux& display_platform =
		    static_cast<const DisplayWindowLinux&>(display_window);
		Display* display = display_platform.display();
		Window window = display_platform.window();
		XWindowAttributes attributes;
		::XGetWindowAttributes(display, window, &attributes);
		XVisualInfo visual_info_template;
		visual_info_template.visualid = ::XVisualIDFromVisual(attributes.visual);
		int visual_info_count = 0;
		XVisualInfo* visual_info_list = ::XGetVisualInfo(display, VisualIDMask,
		                                &visual_info_template,
		                                &visual_info_count);
		O3D_ASSERT(visual_info_list);
		O3D_ASSERT(visual_info_count > 0);
		context_ = 0;

		for(int i = 0; i < visual_info_count; ++i) {
			context_ = ::glXCreateContext(display, visual_info_list + i, 0,
			                              True);

			if(context_) break;
		}

		::XFree(visual_info_list);

		if(!context_) {
			O3D_LOG(ERROR) << "Couldn't create GL context.";
			return INITIALIZATION_ERROR;
		}

		display_ = display;
		window_ = window;

		if(!MakeCurrent()) {
			::glXDestroyContext(display, context_);
			context_ = 0;
			display_ = NULL;
			window_ = 0;
			O3D_LOG(ERROR) << "Couldn't create GL context.";
			return INITIALIZATION_ERROR;
		}

		InitStatus init_status = InitCommonGL();

		if(init_status != SUCCESS) {
			::glXDestroyContext(display, context_);
			context_ = 0;
			display_ = NULL;
			window_ = 0;
		}

		return init_status;
	}

	void RendererGL::Destroy() {
		DestroyCommonGL();

		if(display_) {
			::glXMakeCurrent(display_, 0, 0);

			if(context_) {
				::glXDestroyContext(display_, context_);
				context_ = 0;
			}

			display_ = NULL;
			window_ = 0;
		}
	}

#endif

	bool RendererGL::MakeCurrent() {
#ifdef OS_MACOSX

		if(mac_cgl_context_ != NULL) {
			::CGLSetCurrentContext(mac_cgl_context_);
			return true;
		}
		else if(mac_agl_context_ != NULL) {
			::aglSetCurrentContext(mac_agl_context_);
			return true;
		}
		else {
			return false;
		}

#endif
#ifdef OS_LINUX

		if(context_ != NULL) {
			bool result = ::glXMakeCurrent(display_, window_, context_) == True;
			return result;
		}
		else {
			return false;
		}

#endif
	}

	void RendererGL::PlatformSpecificClear(const Float4& color,
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

// Updates the helper constant used for the D3D -> GL remapping.
// See effect_gl.cc for details.
	void RendererGL::UpdateHelperConstant(float width, float height) {
		MakeCurrentLazy();

		// If render-targets are active, pass -1 to invert the Y axis.  OpenGL uses
		// a different viewport orientation than DX.  Without the inversion, the
		// output of render-target rendering will be upside down.
		if(RenderSurfaceActive()) {
			::glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB,
			                             0,
			                             1.0f / width,
			                             -1.0f / height,
			                             2.0f,
			                             -1.0f);
		}
		else {
			// Only apply the origin offset when rendering to the client area.
			::glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB,
			                             0,
			                             (1.0f + (2.0f * -dest_x_offset())) / width,
			                             (-1.0f + (2.0f * dest_y_offset())) / height,
			                             2.0f,
			                             1.0f);
		}

		CHECK_GL_ERROR();
	}

	void RendererGL::SetViewportInPixels(int left,
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
		if(left == 0 &&
		        top == 0 &&
		        width == display_width() &&
		        height == display_height()) {
			::glDisable(GL_SCISSOR_TEST);
		}
		else {
			::glScissor(left, vieport_top, width, height);
			::glEnable(GL_SCISSOR_TEST);
		}

		::glDepthRange(min_z, max_z);
	}

// Resizes the viewport.
	void RendererGL::Resize(int width, int height) {
		MakeCurrentLazy();
		SetClientSize(width, height);
		CHECK_GL_ERROR();
	}

	bool RendererGL::GoFullscreen(const DisplayWindow& display,
	                              int mode_id) {
#ifdef OS_LINUX
		// This actually just switches the GLX context to the new window. The real
		// work is in main_linux.cc.
		const DisplayWindowLinux& display_platform =
		    static_cast<const DisplayWindowLinux&>(display);
		display_ = display_platform.display();
		window_ = display_platform.window();

		if(!MakeCurrent()) {
			return false;
		}

#endif
		fullscreen_ = true;
		return true;
	}

	bool RendererGL::CancelFullscreen(const DisplayWindow& display,
	                                  int width, int height) {
#ifdef OS_LINUX
		// This actually just switches the GLX context to the old window. The real
		// work is in main_linux.cc.
		const DisplayWindowLinux& display_platform =
		    static_cast<const DisplayWindowLinux&>(display);
		display_ = display_platform.display();
		window_ = display_platform.window();

		if(!MakeCurrent()) {
			return false;
		}

#endif
		fullscreen_ = false;
		return true;
	}

	void RendererGL::GetDisplayModes(std::vector<DisplayMode> *modes) {
#ifdef OS_MACOSX
		// Mac is supposed to call a different function in plugin_mac.mm instead.
		O3D_LOG(FATAL) << "Not supposed to be called";
#endif
		// On all other platforms this is unimplemented. Linux only supports
		// DISPLAY_MODE_DEFAULT for now.
		modes->clear();
	}

	bool RendererGL::GetDisplayMode(int id, DisplayMode* mode) {
#ifdef OS_MACOSX
		// Mac is supposed to call a different function in plugin_mac.mm instead.
		O3D_LOG(FATAL) << "Not supposed to be called";
		return false;
#elif defined(OS_LINUX)

		if(id == DISPLAY_MODE_DEFAULT) {
			// Don't need to know any of this on Linux.
			mode->Set(0, 0, 0, id);
			return true;
		}
		else {
			// There are no other valid ids until we implement GetDisplayModes() and
			// mode switching.
			return false;
		}

#else
		return false;
#endif
	}

	bool RendererGL::PlatformSpecificStartRendering() {
		O3D_LOG_FIRST_N(INFO, 10) << "RendererGL StartRendering";
		MakeCurrentLazy();
		// Currently always returns true.
		// Should be modified if current behavior changes.
		CHECK_GL_ERROR();
		return true;
	}

// Clears the color, depth and stncil buffers and prepares GL for rendering
// the frame.
// Returns true on success.
	bool RendererGL::PlatformSpecificBeginDraw() {
		O3D_LOG_FIRST_N(INFO, 10) << "RendererGL BeginDraw";
		MakeCurrentLazy();
		// Currently always returns true.
		// Should be modified if current behavior changes.
		CHECK_GL_ERROR();
		return true;
	}

// Assign the surface arguments to the renderer, and update the stack
// of pushed surfaces.
	void RendererGL::SetRenderSurfacesPlatformSpecific(
	    const RenderSurface* surface,
	    const RenderDepthStencilSurface* surface_depth) {
		// TODO:  This routine re-uses a single global framebuffer object for
		// all RenderSurface rendering.  Because of the validation checks performed
		// at attachment-change time, it may be more performant to create a pool
		// of framebuffer objects with different attachment characterists and
		// switch between them here.
		MakeCurrentLazy();
		::glBindFramebufferEXT(GL_FRAMEBUFFER, render_surface_framebuffer_);

		if(!InstallFramebufferObjects(surface, surface_depth)) {
			O3D_ERROR(service_locator())
			        << "Failed to bind OpenGL render target objects:"
			        << (surface ? surface->name() : "(no surface)") << ", "
			        << (surface_depth ? surface_depth->name() : "(no depth surface)");
		}

		// RenderSurface rendering is performed with an inverted Y, so the front
		// face winding must be changed to clock-wise.  See comments for
		// UpdateHelperConstant.
		glFrontFace(GL_CW);
	}

	void RendererGL::SetBackBufferPlatformSpecific() {
		MakeCurrentLazy();
		// Bind the default context, and restore the default front-face winding.
		::glBindFramebufferEXT(GL_FRAMEBUFFER, 0);
		glFrontFace(GL_CCW);
	}

// Executes a post rendering step
	void RendererGL::PlatformSpecificEndDraw() {
		O3D_LOG_FIRST_N(INFO, 10) << "RendererGL EndDraw";
		O3D_ASSERT(IsCurrent());
	}

// Swaps the buffers.
	void RendererGL::PlatformSpecificFinishRendering() {
		O3D_LOG_FIRST_N(INFO, 10) << "RendererGL FinishRendering";
		O3D_ASSERT(IsCurrent());
		::glFlush();
		CHECK_GL_ERROR();
	}

	void RendererGL::PlatformSpecificPresent() {
		O3D_LOG_FIRST_N(INFO, 10) << "RendererGL Present";
		O3D_ASSERT(IsCurrent());
#ifdef OS_MACOSX
#ifdef O3D_USE_AGL_DOUBLE_BUFFER

		if(mac_agl_context_) {
			::aglSwapBuffers(mac_agl_context_);
		}

#endif
#endif
#ifdef OS_LINUX
		::glXSwapBuffers(display_, window_);
#endif
	}

	StreamBank::Ref RendererGL::CreateStreamBank() {
		return StreamBank::Ref(new StreamBankGL(service_locator()));
	}

	Primitive::Ref RendererGL::CreatePrimitive() {
		return Primitive::Ref(new PrimitiveGL(service_locator()));
	}

	DrawElement::Ref RendererGL::CreateDrawElement() {
		return DrawElement::Ref(new DrawElementGL(service_locator()));
	}

	void RendererGL::SetStencilStates(GLenum face,
	                                  const StencilStates& stencil_state) {
		O3D_ASSERT(IsCurrent());

		if(face == GL_FRONT_AND_BACK) {
			::glStencilFunc(stencil_state.func_,
			                stencil_ref_,
			                stencil_mask_[READ_MASK]);
			::glStencilOp(stencil_state.op_[StencilStates::FAIL_OP],
			              stencil_state.op_[StencilStates::ZFAIL_OP],
			              stencil_state.op_[StencilStates::PASS_OP]);
			::glStencilMask(stencil_mask_[WRITE_MASK]);
		}
		else if(GLEW_VERSION_2_0) {
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
		else if(GLEW_EXT_stencil_two_side) {
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

		CHECK_GL_ERROR();
	}

	void RendererGL::ApplyDirtyStates() {
		MakeCurrentLazy();
		O3D_ASSERT(IsCurrent());

		// Set blend settings.
		if(alpha_blend_settings_changed_) {
			if(separate_alpha_blend_enable_) {
				if(GLEW_VERSION_1_4) {
					::glBlendFuncSeparate(blend_function_[SRC][RGB],
					                      blend_function_[DST][RGB],
					                      blend_function_[SRC][ALPHA],
					                      blend_function_[DST][ALPHA]);
				}
				else if(GLEW_EXT_blend_func_separate) {
					::glBlendFuncSeparateEXT(blend_function_[SRC][RGB],
					                         blend_function_[DST][RGB],
					                         blend_function_[SRC][ALPHA],
					                         blend_function_[DST][ALPHA]);
				}

				if(GLEW_VERSION_2_0) {
					::glBlendEquationSeparate(blend_equation_[RGB],
					                          blend_equation_[ALPHA]);
				}
				else if(GLEW_EXT_blend_equation_separate) {
					::glBlendEquationSeparateEXT(blend_equation_[RGB],
					                             blend_equation_[ALPHA]);
				}
			}
			else {
				::glBlendFunc(blend_function_[SRC][RGB],
				              blend_function_[DST][RGB]);

				if(::glBlendEquation != NULL)
					::glBlendEquation(blend_equation_[RGB]);
			}

			alpha_blend_settings_changed_ = false;
		}

		// Set alpha settings.
		if(alpha_function_ref_changed_) {
			::glAlphaFunc(alpha_function_, alpha_ref_);
			alpha_function_ref_changed_ = false;
		}

		// Set stencil settings.
		if(stencil_settings_changed_) {
			if(separate_stencil_settings_enable_) {
				SetStencilStates(GL_FRONT, stencil_settings_[FRONT]);
				SetStencilStates(GL_BACK, stencil_settings_[BACK]);
			}
			else {
				SetStencilStates(GL_FRONT_AND_BACK, stencil_settings_[FRONT]);
			}

			stencil_settings_changed_ = false;
		}

		if(polygon_offset_changed_) {
			bool enable = (polygon_offset_factor_ != 0.f) ||
			              (polygon_offset_bias_ != 0.f);

			if(enable) {
				::glEnable(GL_POLYGON_OFFSET_POINT);
				::glEnable(GL_POLYGON_OFFSET_LINE);
				::glEnable(GL_POLYGON_OFFSET_FILL);
				::glPolygonOffset(polygon_offset_factor_, polygon_offset_bias_);
			}
			else {
				::glDisable(GL_POLYGON_OFFSET_POINT);
				::glDisable(GL_POLYGON_OFFSET_LINE);
				::glDisable(GL_POLYGON_OFFSET_FILL);
			}

			polygon_offset_changed_ = false;
		}

		CHECK_GL_ERROR();
	}

	VertexBuffer::Ref RendererGL::CreateVertexBuffer() {
		O3D_LOG(INFO) << "RendererGL CreateVertexBuffer";
		MakeCurrentLazy();
		return VertexBuffer::Ref(new VertexBufferGL(service_locator()));
	}

	IndexBuffer::Ref RendererGL::CreateIndexBuffer() {
		O3D_LOG(INFO) << "RendererGL CreateIndexBuffer";
		MakeCurrentLazy();
		return IndexBuffer::Ref(new IndexBufferGL(service_locator()));
	}

	Effect::Ref RendererGL::CreateEffect() {
		O3D_LOG(INFO) << "RendererGL CreateEffect";
		MakeCurrentLazy();
		return Effect::Ref(new EffectGL(service_locator(), cg_context_));
	}

	Sampler::Ref RendererGL::CreateSampler() {
		return Sampler::Ref(new SamplerGL(service_locator()));
	}

	ParamCache* RendererGL::CreatePlatformSpecificParamCache() {
		return new ParamCacheGL(semantic_manager_.Get(), this);
	}


	Texture2D::Ref RendererGL::CreatePlatformSpecificTexture2D(
	    int width,
	    int height,
	    Texture::Format format,
	    int levels,
	    bool enable_render_surfaces) {
		O3D_LOG(INFO) << "RendererGL CreateTexture2D";
		MakeCurrentLazy();
		return Texture2D::Ref(Texture2DGL::Create(service_locator(),
		                      format,
		                      levels,
		                      width,
		                      height,
		                      enable_render_surfaces));
	}

	TextureCUBE::Ref RendererGL::CreatePlatformSpecificTextureCUBE(
	    int edge_length,
	    Texture::Format format,
	    int levels,
	    bool enable_render_surfaces) {
		O3D_LOG(INFO) << "RendererGL CreateTextureCUBE";
		MakeCurrentLazy();
		return TextureCUBE::Ref(TextureCUBEGL::Create(service_locator(),
		                        format,
		                        levels,
		                        edge_length,
		                        enable_render_surfaces));
	}

	RenderDepthStencilSurface::Ref RendererGL::CreateDepthStencilSurface(
	    int width,
	    int height) {
		return RenderDepthStencilSurface::Ref(
		           new RenderDepthStencilSurfaceGL(service_locator(),
		                   width,
		                   height));
	}

	const int* RendererGL::GetRGBAUByteNSwizzleTable() {
		static int swizzle_table[] = { 0, 1, 2, 3, };
		return swizzle_table;
	}

// This is a factory function for creating Renderer objects.  Since
// we're implementing GL, we only ever return a GL renderer.
	Renderer* Renderer::CreateDefaultRenderer(ServiceLocator* service_locator) {
		return RendererGL::CreateDefault(service_locator);
	}

}  // namespace o3d
