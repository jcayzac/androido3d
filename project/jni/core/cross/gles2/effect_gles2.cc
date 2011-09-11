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


// This file contains the definition of EffectGLES2, the OpenGLES2
// implementation of the abstract O3D class Effect.

#include <cctype>
#include <sstream>
#include "core/cross/semantic_manager.h"
#include "core/cross/error.h"
#include "core/cross/standard_param.h"
#include "core/cross/gles2/effect_gles2.h"
#include "core/cross/gles2/renderer_gles2.h"
#include "core/cross/gles2/primitive_gles2.h"
#include "core/cross/gles2/draw_element_gles2.h"
#include "core/cross/gles2/texture_gles2.h"
#include "core/cross/gles2/utils_gles2.h"
#include "core/cross/gles2/utils_gles2-inl.h"

namespace o3d {

// Number of repeating events to log before giving up, e.g. setup frame,
// draw polygons, etc.
	const int kNumLoggedEvents = 5;

// Convert a GLunum data type into a Param type.
	static const ObjectBase::Class* GLTypeToParamType(GLenum gl_type) {
		switch(gl_type) {
		case GL_FLOAT:
			return ParamFloat::GetApparentClass();
		case GL_FLOAT_VEC2:
			return ParamFloat2::GetApparentClass();
		case GL_FLOAT_VEC3:
			return ParamFloat3::GetApparentClass();
		case GL_FLOAT_VEC4:
			return ParamFloat4::GetApparentClass();
		case GL_INT:
			return ParamInteger::GetApparentClass();
		case GL_INT_VEC2:
			return NULL;
		case GL_INT_VEC3:
			return NULL;
		case GL_INT_VEC4:
			return NULL;
		case GL_BOOL:
			return ParamBoolean::GetApparentClass();
		case GL_BOOL_VEC2:
			return NULL;
		case GL_BOOL_VEC3:
			return NULL;
		case GL_BOOL_VEC4:
			return NULL;
		case GL_FLOAT_MAT2:
			return NULL;
		case GL_FLOAT_MAT3:
			return NULL;
		case GL_FLOAT_MAT4:
			return ParamMatrix4::GetApparentClass();
		case GL_SAMPLER_2D:
			return ParamSampler::GetApparentClass();
		case GL_SAMPLER_CUBE:
			return ParamSampler::GetApparentClass();
		default : {
				O3D_LOG(ERROR) << "Cannot convert GLtype "
				               << gl_type
				               << " to a Param type.";
				return NULL;
			}
		}
	}

// -----------------------------------------------------------------------------

	EffectGLES2::EffectGLES2(ServiceLocator* service_locator)
		: Effect(service_locator),
		  semantic_manager_(service_locator->GetService<SemanticManager>()),
		  renderer_(static_cast<RendererGLES2*>(
		                service_locator->GetService<Renderer>())),
		  gl_program_(0),
		  compile_count_(-1) {
		O3D_LOG(INFO) << "EffectGLES2 Construct";
	}

// Destructor releases vertex and fragment shaders and their correspoding
// constants tables.
	EffectGLES2::~EffectGLES2() {
		O3D_LOG(INFO) << "EffectGLES2 Destruct \"" << name() << "\"";
		ClearProgram();
	}

	void EffectGLES2::ClearProgram() {
		if(gl_program_) {
			glDeleteProgram(gl_program_);
			gl_program_ = 0;
		}

		set_source("");
	}

	GLuint EffectGLES2::LoadShader(GLenum type, const char* shader_src) {
		GLuint shader = glCreateShader(type);

		if(shader == 0) {
			return 0;
		}

		// Load the shader source
		glShaderSource(shader, 1, &shader_src, NULL);
		// Compile the shader
		glCompileShader(shader);
		// Check the compile status
		GLint value;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &value);

		if(value == 0) {
			GLint error_length;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &error_length);
			::o3d::base::scoped_array<char> buffer(new char[error_length + 1]);
			GLsizei length;
			glGetShaderInfoLog(shader, error_length + 1, &length, buffer.get());
			O3D_ERROR(service_locator()) << "Effect Compile Error: " << buffer.get();
			O3D_LOG(ERROR) << "Error compiling shader:" << buffer.get();
			O3D_LOG(ERROR) << "shader: \n" << shader_src << "\n";
			glDeleteShader(shader);
			return 0;
		}

		return shader;
	}

	namespace {

		std::string::size_type GetEndOfIdentifier(const std::string& original,
		        std::string::size_type start) {
			if(start < original.size()) {
				// check that first character is alpha or '_'
				if(isalpha(original[start]) || original[start] == '_') {
					std::string::size_type end = original.size();
					std::string::size_type position = start;

					while(position < end) {
						char c = original[position];

						if(!isalnum(c) && c != '_') {
							break;
						}

						++position;
					}

					return position;
				}
			}

			return std::string::npos;
		}

		bool GetIdentifierAfterString(const std::string& original,
		                              const std::string& phrase,
		                              std::string* word) {
			std::string::size_type position = original.find(phrase);

			if(position == std::string::npos) {
				return false;
			}

			// Find end of identifier
			std::string::size_type start = position + phrase.size();
			std::string::size_type end = GetEndOfIdentifier(original, start);

			if(end != start && end != std::string::npos) {
				*word = std::string(original, start, end - start);
				return true;
			}

			return false;
		}

		bool PrepareFragmentShaderForPicking(std::string& source) {
			// Copy the source
			std::string tmp(source);

			// Replace all allowed whitespaces with the 'space' character:
			for(size_t i(0); i < source.length(); ++i)
				tmp[i] = (source[i] == '\t' || source[i] == '\r' || source[i] == '\n') ? ' ' : tmp[i];

			// Search for /void[[:space:]]+main/ and put the position of
			// 'main' in found_at
			std::stringstream ss;
#if !defined (STLPORT)
			// avoid an unnecessary copy
			ss.rdbuf()->pubsetbuf(&tmp[0], source.length());
#else
			// work around annoying STLPort bug, doing the effing copy
			ss.str(tmp);
#endif
			size_t found_at = 0;
			static const std::string _main0("main");
			static const std::string _main1("main(");
			static const std::string _main2("main()");
			static const std::string _main3("main(){");
			static const std::string _void("void");
			std::string token;
			std::string previous;

			while(ss >> token) {
				// previous was "void"…
				if(previous == _void) {
					// …and current is "main[([)[{]?]?]?"
					if(token == _main0 || token == _main1 || token == _main2 || token == _main3) {
						found_at = (size_t)ss.tellg() - token.length();
						break;
					}
				}

				token.swap(previous);
			}

			// Found it?
			if(!found_at) return false;

			// Replace original with the new source
			source.replace(found_at, _main0.length(), "o3d_render_main");
			ss.str("");
			ss  << "uniform mediump vec4 o3d_picking_color;\n"
			    << "uniform bool o3d_picking_mode;\n"
			    << source << "\n"
			    << "void main() {\n"
			    << "    if (o3d_picking_mode) {\n"
			    << "        gl_FragColor = o3d_picking_color;\n"
			    << "    }\n"
			    << "    else {\n"
			    << "        o3d_render_main();\n"
			    << "    }\n"
			    << "}\n";
			ss.str().swap(source);
			return true;
		}

#ifdef GLES2_BACKEND_DESKTOP_GL
		const char kVertexHeader[] = "";
		const char kFragmentHeader[] = "// ";
#else
		const char kVertexHeader[] = "precision highp float; precision highp int;\n";
		const char kFragmentHeader[] =
		    "precision mediump float; precision mediump int;\n// ";
#endif

	}  // anonymous namespace

// Initializes the Effect object using the shaders found in an FX formatted
// string.
	bool EffectGLES2::LoadFromFXString(const std::string& effect) {
		O3D_LOG(INFO) << "EffectGLES2 LoadFromFXString";
		renderer_->MakeCurrentLazy();
		++compile_count_;
		ClearProgram();
		std::string matrix_load_order_str;

		if(!GetIdentifierAfterString(effect,
		                             kMatrixLoadOrderPrefix,
		                             &matrix_load_order_str)) {
			O3D_ERROR(service_locator()) << "Failed to find \""
			                             << kMatrixLoadOrderPrefix
			                             << "\" in Effect";
			return false;
		}

		bool column_major = matrix_load_order_str == "ColumnMajor";
		MatrixLoadOrder matrix_load_order = column_major ? COLUMN_MAJOR : ROW_MAJOR;
		// Split the effect
		const char* kSplitMarker = "// #o3d SplitMarker";
		std::string::size_type split_pos = effect.find(kSplitMarker);

		if(split_pos == std::string::npos) {
			O3D_ERROR(service_locator()) << "Missing '" << kSplitMarker
			                             << "' in shader: " << effect;
			return false;
		}

		std::string vertex_shader(kVertexHeader + effect.substr(0, split_pos));
		std::string fragment_shader(kFragmentHeader + effect.substr(split_pos));

		// TODO(jcayzac): it would be better to maintain two shader programs
		// and select the right one depending on renderer.picking(), but I
		// don't know how to do that as of yet.
		if(!PrepareFragmentShaderForPicking(fragment_shader)) {
			O3D_ERROR(service_locator()) << "Failed to enable picking support in shader: " << effect;
			return false;
		}

		set_matrix_load_order(matrix_load_order);
		GLuint gl_vertex_shader =
		    LoadShader(GL_VERTEX_SHADER, vertex_shader.c_str());

		if(!gl_vertex_shader) {
			return false;
		}

		GLuint gl_fragment_shader =
		    LoadShader(GL_FRAGMENT_SHADER, fragment_shader.c_str());

		if(!gl_fragment_shader) {
			glDeleteShader(gl_vertex_shader);
			return false;
		}

		gl_program_ = glCreateProgram();

		if(!gl_program_) {
			glDeleteShader(gl_fragment_shader);
			glDeleteShader(gl_vertex_shader);
			return false;
		}

		glAttachShader(gl_program_, gl_vertex_shader);
		glAttachShader(gl_program_, gl_fragment_shader);
		glLinkProgram(gl_program_);
		glDeleteShader(gl_vertex_shader);
		glDeleteShader(gl_fragment_shader);
		// Check the compile status
		GLint value = 0;
		glGetProgramiv(gl_program_, GL_LINK_STATUS, &value);
		CHECK_GL_ERROR();

		if(value == 0) {
			GLint error_length = 0;
			glGetProgramiv(gl_program_, GL_INFO_LOG_LENGTH, &error_length);
			::o3d::base::scoped_array<char> buffer(new char[error_length + 1]);
			GLsizei length;
			glGetProgramInfoLog(gl_program_, error_length + 1, &length, buffer.get());
			O3D_ERROR(service_locator()) << "Effect Link Error: " << buffer.get();
			O3D_LOG(ERROR) << "Error linking programr:" << buffer.get();
			glDeleteProgram(gl_program_);
			return false;
		}

		CacheGLParamMapping();
		CHECK_GL_ERROR();
		set_source(effect);
		return true;
	}

	bool EffectGLES2::OnContextRestored() {
		if(gl_program_ && !source().empty()) {
			// The program is not valid. It belonged to a different context.
			gl_program_ = 0;
			// we have to make a copy of the string because LoadFromFXString
			// will clear the internal string.
			std::string shader(source());

			if(!LoadFromFXString(shader)) {
				O3D_LOG(ERROR) << "couldn't restore" << name() << ":\n" << source();
				return false;
			}
		}

		return true;
	}

	void EffectGLES2::CacheGLParamMapping() {
		GLint num_uniforms = 0;
		GLint max_len = 0;
		glGetProgramiv(gl_program_, GL_ACTIVE_UNIFORMS, &num_uniforms);
		glGetProgramiv(gl_program_, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_len);
		::o3d::base::scoped_array<char> name_buffer(new char[max_len + 1]);

		for(GLint ii = 0; ii < num_uniforms; ++ii) {
			GLsizei length;
			GLsizei size;
			GLenum gl_type;
			glGetActiveUniform(
			    gl_program_, ii,
			    max_len + 1, &length, &size, &gl_type, name_buffer.get());
			// TODO(gman): Should we check for error?
			GLint location = glGetUniformLocation(gl_program_, name_buffer.get());
			std::string name(name_buffer.get(), length);
			GLProgramParam::Ref param(new GLProgramParam(location, gl_type, size));
			shader_param_info_map_[name] = param;
		}
	}

	void EffectGLES2::GetShaderParamInfo(
	    GLuint program,
	    std::map<std::string, EffectParameterInfo>* info_map) {
		O3D_ASSERT(info_map);

		if(!program) {
			return;
		}

		GLint num_uniforms = 0;
		GLint max_len = 0;
		glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &num_uniforms);
		glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_len);
		// TODO(gman): Should we check for error?
		::o3d::base::scoped_array<char> name_buffer(new char[max_len + 1]);

		// Loop over all parameters.
		for(GLint ii = 0; ii < num_uniforms; ++ii) {
			GLsizei length;
			GLsizei size;
			GLenum type;
			glGetActiveUniform(
			    program, ii, max_len + 1, &length, &size, &type, name_buffer.get());
			std::string name(name_buffer.get());
			// TODO(gman): Should we check for error?
			// TODO(gman): Should we skip uniforms that start with "gl_"?
			int num_elements = 0;

			if(size > 1) {
				// It's an array.
				num_elements = size;
			}

			const ObjectBase::Class* param_class = GLTypeToParamType(type);

			if(!param_class)
				continue;

			const ObjectBase::Class* sem_class = NULL;
			// Since there is no SAS for GLSL let's just use reserved names.
			sem_class = semantic_manager_->LookupSemantic(name);
			(*info_map)[name] = EffectParameterInfo(
			                        name,
			                        param_class,
			                        num_elements,
			                        sem_class != NULL ? name : "",
			                        sem_class);
		}
	}

	namespace {
		struct selectParamInfo {
			EffectParameterInfo& operator()(std::pair<std::string, EffectParameterInfo>& x) const { return x.second; }
			const EffectParameterInfo& operator()(const std::pair<std::string, EffectParameterInfo>& x) const { return x.second; }
		};
	}

	void EffectGLES2::GetParameterInfo(EffectParameterInfoArray* info_array) {
		O3D_ASSERT(info_array);
		std::map<std::string, EffectParameterInfo> info_map;
		renderer_->MakeCurrentLazy();

		if(gl_program_) {
			GetShaderParamInfo(gl_program_, &info_map);
		}

		info_array->clear();
		info_array->reserve(info_map.size());
		std::transform(
		    info_map.begin(),
		    info_map.end(),
		    std::back_inserter(*info_array),
		    selectParamInfo());
	}

	void EffectGLES2::GetVaryingVertexShaderParamInfo(
	    GLuint program,
	    std::vector<EffectStreamInfo>* info_array) {
		GLint num_attribs = 0;
		GLint max_len = 0;
		glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &num_attribs);
		glGetProgramiv(program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &max_len);
		// TODO(gman): Should we check for error?
		::o3d::base::scoped_array<char> name_buffer(new char[max_len + 1]);

		for(GLint ii = 0; ii < num_attribs; ++ii) {
			GLsizei length;
			GLsizei size;
			GLenum type;
			glGetActiveAttrib(
			    program, ii, max_len + 1, &length, &size, &type, name_buffer.get());
			// TODO(gman): Should we check for error?
			std::string name(name_buffer.get());
			// Since GLSL has no semantics just go by name.
			Stream::Semantic semantic;
			int semantic_index;

			if(!SemanticNameToSemantic(name, &semantic, &semantic_index)) {
				continue;
			}

			info_array->push_back(EffectStreamInfo(semantic, semantic_index));
		}
	}

	void EffectGLES2::GetStreamInfo(
	    EffectStreamInfoArray* info_array) {
		O3D_ASSERT(info_array);
		renderer_->MakeCurrentLazy();
		info_array->clear();
		GetVaryingVertexShaderParamInfo(gl_program_, info_array);
	}

// private functions -----------------------------------------------------------

// Loop through all the uniform parameters on the effect and set their values
// from their corresponding Params on the various ParamObject (as stored in the
// ParamCacheGLES2).
	void EffectGLES2::UpdateShaderUniformsFromEffect(
	    ParamCacheGLES2* param_cache_gl) {
		O3D_LOG_FIRST_N(INFO, kNumLoggedEvents)
		        << "EffectGLES2 UpdateShaderUniformsFromEffect";
		renderer_->ResetTextureGroupSetCount();
		ParamCacheGLES2::UniformParameterMap& map = param_cache_gl->uniform_map();
		ParamCacheGLES2::UniformParameterMap::iterator i;

		for(i = map.begin(); i != map.end(); ++i) {
			GLES2Parameter gl_param = i->first;
			i->second->SetEffectParam(renderer_, gl_param);
		}

		renderer_->UpdateDxClippingUniform(
		    glGetUniformLocation(gl_program_, "dx_clipping"));
		const bool picking_mode_enabled(renderer_->picking());
		glUniform1i(glGetUniformLocation(gl_program_, "o3d_picking_mode"), (GLint)picking_mode_enabled);

		if(picking_mode_enabled)
			renderer_->UpdatePickingColorUniform(glGetUniformLocation(gl_program_, "o3d_picking_color"));

		CHECK_GL_ERROR();
	}

// Loop through all the uniform parameters on the effect and reset their values.
// For now, this unbinds textures contained in sampler parameters.
	void EffectGLES2::ResetShaderUniforms(ParamCacheGLES2* param_cache_gles2) {
		O3D_LOG_FIRST_N(INFO, kNumLoggedEvents) << "EffectGLES2 ResetShaderUniforms";
		ParamCacheGLES2::UniformParameterMap& map = param_cache_gles2->uniform_map();
		ParamCacheGLES2::UniformParameterMap::iterator i;

		for(i = map.begin(); i != map.end(); ++i) {
			GLES2Parameter gl_param = i->first;
			i->second->ResetEffectParam(renderer_, gl_param);
		}

		CHECK_GL_ERROR();
	}

// Updates the values of the vertex and fragment shader parameters using the
// current values in the param/glparam caches.
	void EffectGLES2::PrepareForDraw(ParamCacheGLES2* param_cache_gles2) {
		O3D_LOG_FIRST_N(INFO, kNumLoggedEvents) << "EffectGLES2 PrepareForDraw \""
		                                        << name()
		                                        << "\"";
		O3D_ASSERT(renderer_->IsCurrent());

		if(gl_program_) {
			// Initialise the render states for this pass, this includes the shaders.
			glUseProgram(gl_program_);
			UpdateShaderUniformsFromEffect(param_cache_gles2);
		}
		else {
			O3D_LOG_FIRST_N(ERROR, kNumLoggedEvents)
			        << "No valid GLES2effect found "
			        << "in Effect \"" << name() << "\"";
		}

		CHECK_GL_ERROR();
	}

// Resets the render states back to their default value.
	void EffectGLES2::PostDraw(ParamCacheGLES2* param_cache_gles2) {
		O3D_LOG_FIRST_N(INFO, kNumLoggedEvents)
		        << "EffectGLES2 PostDraw \"" << name() << "\"";
		O3D_ASSERT(renderer_->IsCurrent());
		ResetShaderUniforms(param_cache_gles2);
		CHECK_GL_ERROR();
	}

}  // namespace o3d
