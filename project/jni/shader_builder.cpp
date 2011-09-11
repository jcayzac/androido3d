// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// TODO(gman): This code is shit right now. It was converted directly from
//     JavaScript with little thought on how it should be re-written for C++.
// Small change to test hg

#include <map>
#include <string>
#include <vector>
#include <ctype.h>
#include <stdlib.h>
#include <strings.h>
#include "shader_builder.h"
#include "core/cross/effect.h"
#include "core/cross/material.h"
#include "core/cross/pack.h"
#include "core/cross/param.h"
#include "core/cross/param_array.h"
#include "core/cross/sampler.h"
#include "core/cross/texture.h"
#include "debug.h"

namespace o3d_utils {
	using namespace o3d;

	class IntStringThing {
	public:
		IntStringThing()
			: value_(0) {
		}

		void Set(int value) {
			value_ = value;
		}

		void Increment() {
			++value_;
		}

		std::string Inc() {
			int v = value_;
			value_++;
			char buf[20];
			sprintf(buf, "%d", v);
			return &buf[0];
		}

		std::string ToString() {
			char buf[20];
			sprintf(buf, "%d", value_);
			return &buf[0];
		}

	private:
		int value_;
	};

	/**
	 * Check if lighting type is a collada lighting type.
	 * @param {string} lightingType Lighting type to check.
	 * @return {boolean} true if it"s a collada lighting type.
	 */
	bool ShaderBuilder::isColladaLightingType(const std::string& lightingType) {
		static const char* COLLADA_LIGHTING_TYPES[] = {
			"phong",
			"lambert",
			"blinn",
			"constant",
		};

		for(size_t ii = 0; ii < o3d_arraysize(COLLADA_LIGHTING_TYPES); ++ii) {
			if(strcasecmp(lightingType.c_str(), COLLADA_LIGHTING_TYPES[ii]) == 0) {
				return true;
			}
		}

		return false;
	};

	/**
	 * Returns the collada lighting type of a collada standard material.
	 * @param {!o3d.Material} material Material to get lighting type from.
	 * @return {string} The lighting type or "" if it"s not a collada standard
	 *     material.
	 */
	std::string ShaderBuilder::getColladaLightingType(Material* material) {
		/**
		 * The name of the parameter on a material if it"s a collada standard
		 * material.
		 *
		 * NOTE: This parameter is just a string attached to a material. It has no
		 *     meaning to the plugin, it is passed from the conditioner to the
		 *     javascript libraries so that they can build collada like effects.
		 *
		 * @type {string}
		 */
		static const char* COLLADA_LIGHTING_TYPE_PARAM_NAME =
		    "collada.lightingType";
		ParamString* lightingTypeParam =
		    material->GetParam<ParamString>(COLLADA_LIGHTING_TYPE_PARAM_NAME);

		if(lightingTypeParam) {
			std::string lightingType(lightingTypeParam->value());
			std::transform(lightingType.begin(), lightingType.end(),
			               lightingType.begin(), tolower);

			if(isColladaLightingType(lightingType)) {
				return lightingType;
			}
		}

		return "";
	};

	/**
	 * Get the number of TEXCOORD streams needed by this material.
	 * @param {!o3d.Material} material The material MUST be a standard
	 *     collada material.
	 * @return {number} The number oc TEXCOORD streams needed.
	 */
	int ShaderBuilder::getNumTexCoordStreamsNeeded(Material* material) {
		/**
		 * The FCollada standard materials sampler parameter name prefixes.
		 * @type {!Array.<string>}
		 */
		static const char* COLLADA_SAMPLER_PARAMETER_PREFIXES[] = {
			"emissive",
			"ambient",
			"diffuse",
			"specular",
			"bump",
		};
		std::string lightingType = getColladaLightingType(material);

		if(!isColladaLightingType(lightingType)) {
			O3D_NEVER_REACHED() << "not a collada standard material";
		}

		int numTexCoordStreamsNeeded = 0;

		for(size_t cc = 0;
		        cc < o3d_arraysize(COLLADA_SAMPLER_PARAMETER_PREFIXES);
		        ++cc) {
			std::string samplerPrefix(COLLADA_SAMPLER_PARAMETER_PREFIXES[cc]);
			ParamSampler* samplerParam = material->GetParam<ParamSampler>(
			                                 samplerPrefix + "Sampler");

			if(samplerParam) {
				++numTexCoordStreamsNeeded;
			}
		}

		return numTexCoordStreamsNeeded;
	};

	class GLSLShaderBuilder : public ShaderBuilder {
	private:
		/**
		 * Caches the varying parameter declarations to be repeated in the case that
		 * we"re in glsl and need to declare the varying parameters in both shaders.
		 * @type {string}
		 */
		std::string varying_decls_;

		/**
		 * An integer value which keeps track of the next available interpolant.
		 * @type {number}
		 * @private
		 */
		IntStringThing interpolant_;

		std::map<std::string, std::string> name_to_semantic_map_;

#define TWO_COLOR_CHECKER_EFFECT_NAME "o3djs.effect.twoColorCheckerEffect"

#define FLOAT2 "vec2"
#define FLOAT3 "vec3"
#define FLOAT4 "vec4"
#define MATRIX4 "mat4"
#define MATRIX3 "mat3"
#define MOD "mod"
#define ATTRIBUTE "attribute "
#define ATTRIBUTE_PREFIX ""
#define VARYING "varying "
#define VARYING_DECLARATION_PREFIX "v_"
#define VERTEX_VARYING_PREFIX "v_"
#define PIXEL_VARYING_PREFIX "v_"
#define TEXTURE "texture"
#define BEGIN_IN_STRUCT ""
#define BEGIN_OUT_STRUCT ""
#define END_STRUCT ""

		std::string getSemanticMapName(const std::string& semantic) {
			static const char* s_map[][2] = {
				{ "POSITION"  , "position", },
				{ "NORMAL"    , "normal", },
				{ "TANGENT"   , "tangent", },
				{ "BINORMAL"  , "binormal", },
				{ "COLOR"     , "color", },
				{ "TEXCOORD0" , "texCoord0", },
				{ "TEXCOORD1" , "texCoord1", },
				{ "TEXCOORD2" , "texCoord2", },
				{ "TEXCOORD3" , "texCoord3", },
				{ "TEXCOORD4" , "texCoord4", },
				{ "TEXCOORD5" , "texCoord5", },
				{ "TEXCOORD6" , "texCoord6", },
				{ "TEXCOORD7" , "texCoord7", },
			};

			for(size_t ii = 0; ii < sizeof(s_map) / sizeof(s_map[0]); ++ii) {
				if(semantic.compare(s_map[ii][0]) == 0) {
					return s_map[ii][1];
				}
			}

			return "";
		};

		/**
		 * The string that goes between the stream name and the semicolon to indicate
		 * the semantic.
		 * @param {string} name Name of the semantic.
		 * @return {string}
		 */
		std::string semanticSuffix(const std::string& name) {
			return "";
		};


		/**
		 * Attribute variables in GLSL need to be named by their semantic in
		 * order for the implementation to hook them up correctly.
		 * @private
		 */
		std::string getAttributeName(const std::string& name,
		                             const std::string& semantic) {
			return getSemanticMapName(semantic);
		};

		/**
		 * Generates code to multiply two things.
		 * @param {string} a One multiplicand.
		 * @param {string} b The other multiplicand.
		 * @return {string}
		 */
		std::string mul(const std::string& a, const std::string& b) {
			return std::string("(") + b + " * " + a + ")";
		};

		/**
		 * Generates code for some utility functions
		 * (functions defined in cg but not glsl).
		 * @return {string} The code for the utility functions.
		 */
		std::string utilityFunctions() {
			return std::string(
			           "vec4 lit(float l ,float h, float m) {\n"
			           "  return vec4(1.0,\n"
			           "              max(l, 0.0),\n"
			           "              (l > 0.0) ? pow(max(0.0, h), m) : 0.0,\n"
			           "              1.0);\n"
			           "}\n");
		};


		/**
		 * The string that starts the vertex shader main function.
		 * @return {string} The effect code for the start of the main.
		 */
		std::string beginVertexShaderMain() {
			return std::string("void main() {\n");
		};

		/**
		 * The string that ends the vertex main function.
		 * @return {string}
		 */
		std::string endVertexShaderMain() {
			return std::string("  gl_Position = "
			                   VERTEX_VARYING_PREFIX
			                   "position;\n}\n");
		};

		/**
		 * The string that goes infront of the pixel shader main.
		 * @param {!o3d.Material} material The material to inspect.
		 * @param {boolean} diffuse Whether to include stuff for diffuse calculations.
		 * @param {boolean} specular Whether to include stuff for diffuse
		 *     calculations.
		 * @param {boolean} bumpSampler Whether there is a bump sampler.
		 * @return {string} The header.
		 */
		std::string pixelShaderHeader(
		    Material* /* material */,
		    bool /* diffuse */,
		    bool /* specular */,
		    ParamSampler* /* bumpSampler */) {
			return "\n// #o3d SplitMarker\n";
		};

		/**
		 * Repeats the declarations for the varying parameters if necessary.
		 * @param {string} opt_decls The declarations if you know them already.
		 * @return {string} Code for the parameter declarations.
		 */
		// TODO(gman): Remove this. Just save the return from buildVaryingDecls
		//   locally and use it.
		std::string repeatVaryingDecls(const std::string* opt_decls) {
			return (opt_decls ? *opt_decls : varying_decls_) + "\n";
		};

		/**
		 * The string that goes infront of the pixel shader main.
		 * @return {string} The effect code for the start of the main.
		 */
		std::string beginPixelShaderMain() {
			return std::string(
			           "void main() {\n"
			       );
		};

		/**
		 * The string that goes at the end of the pixel shader main.
		 * @param {string} color The code for the color to return.
		 * @return {string} The effect code for the end of the main.
		 */
		std::string endPixelShaderMain(const std::string& color) {
			return std::string("  gl_FragColor = ") + color + ";\n}\n";
		};

		/**
		 * The vertex and fragment shader entry points.  In glsl, this is unnecessary.
		 * @return {string}
		 */
		std::string entryPoints() {
			return "";
		};

		std::string matrixLoadOrder() {
			return "// #o3d MatrixLoadOrder RowMajor\n";
		};

		/**
		 * Builds the vertex attribute declarations for a given material.
		 * @param {!o3d.Material} material The material to inspect.
		 * @param {boolean} diffuse Whether to include stuff for diffuse calculations.
		 * @param {boolean} specular Whether to include stuff for diffuse
		 *     calculations.
		 * @param {boolean} bumpSampler Whether there is a bump sampler.
		 * @return {string} The code for the declarations.
		 */
		std::string buildAttributeDecls(
		    Material* material,
		    bool diffuse,
		    bool specular,
		    bool bumpSampler) {
			std::string str = std::string(BEGIN_IN_STRUCT) +
			                  ATTRIBUTE + FLOAT4 + " " + "position" +
			                  semanticSuffix("POSITION") + ";\n";

			if(diffuse || specular) {
				str = str + ATTRIBUTE + FLOAT3 + " " + "normal" +
				      semanticSuffix("NORMAL") + ";\n";
			}

			str += buildTexCoords(material, false);
			str += buildBumpInputCoords(bumpSampler);
			str += END_STRUCT;
			return str;
		};


		/**
		 * Builds the varying parameter declarations for a given material.
		 * @param {!o3d.Material} material The material to inspect.
		 * @param {boolean} diffuse Whether to include stuff for diffuse calculations.
		 * @param {boolean} specular Whether to include stuff for diffuse
		 *     calculations.
		 * @param {boolean} bumpSampler Whether there is a bump sampler.
		 * @return {string} The code for the declarations.
		 */
		std::string buildVaryingDecls(
		    Material* material,
		    bool diffuse,
		    bool specular,
		    bool bumpSampler) {
			std::string str = std::string(BEGIN_OUT_STRUCT) +
			                  VARYING + FLOAT4 + " " +
			                  VARYING_DECLARATION_PREFIX + "position" +
			                  semanticSuffix("POSITION") + ";\n";
			str += buildTexCoords(material, true);
			str += buildBumpOutputCoords(bumpSampler);

			if(diffuse || specular) {
				str = str + VARYING + FLOAT3 + " " +
				      VARYING_DECLARATION_PREFIX + "normal" +
				      semanticSuffix("TEXCOORD" +
				                     interpolant_.Inc() + "") + ";\n";
				str = str + VARYING + FLOAT3 + " " +
				      VARYING_DECLARATION_PREFIX + "surfaceToLight" +
				      semanticSuffix(
				          "TEXCOORD" + interpolant_.Inc() + "") + ";\n";
			}

			if(specular) {
				str = str + VARYING + FLOAT3 + " " +
				      VARYING_DECLARATION_PREFIX + "surfaceToView" +
				      semanticSuffix(
				          "TEXCOORD" + interpolant_.Inc() + "") + ";\n";
			}

			str = str + END_STRUCT;
			varying_decls_ = str;
			return str;
		};

		/**
		 * Builds the texture coordinate declaration for a given color input
		 * (usually emissive, ambient, diffuse or specular).  If the color
		 * input does not have a sampler, no TEXCOORD declaration is built.
		 * @param {!o3d.Material} material The material to inspect.
		 * @param {boolean} varying Whether these vertex declarations should
		 *     be written as varying values.
		 * @param {string} name The name of the color input.
		 * @return {string} The code for the texture coordinate declaration.
		 */
		std::string buildTexCoord(
		    Material* material, bool varying, const std::string& name) {
			// In the GLSL version we need to name the incoming attributes by
			// the semantic name in order for them to get hooked up correctly.
			if(material->GetUntypedParam(name + "Sampler")) {
				if(varying) {
					return std::string("  ") + VARYING + FLOAT2 + " " +
					       VARYING_DECLARATION_PREFIX + name + "UV" +
					       semanticSuffix(
					           "TEXCOORD" + interpolant_.Inc() + "") + ";\n";
				}
				else {
					std::string desiredName(name + "UV");
					std::string semantic(std::string("TEXCOORD") + interpolant_.Inc());
					std::string outputName(getAttributeName(desiredName, semantic));
					name_to_semantic_map_[desiredName] = semantic;
					return std::string("  ") + ATTRIBUTE + FLOAT2 + " " + outputName +
					       semanticSuffix(semantic) + ";\n";
				}
			}
			else {
				return "";
			}
		};

		/**
		 * Builds all the texture coordinate declarations for a vertex attribute
		 * declaration.
		 * @param {!o3d.Material} material The material to inspect.
		 * @param {boolean} varying Whether these vertex declarations should
		 *     be written as varying values.
		 * @return {string} The code for the texture coordinate declarations.
		 */
		std::string buildTexCoords(Material* material, bool varying) {
			interpolant_.Set(0);

			if(!varying) {
				name_to_semantic_map_.clear();
			}

			std::string str;
			str += buildTexCoord(material, varying, "emissive");
			str += buildTexCoord(material, varying, "ambient");
			str += buildTexCoord(material, varying, "diffuse");
			str += buildTexCoord(material, varying, "specular");
			return str;
		};


		/**
		 * Builds the texture coordinate passthrough statement for a given
		 * color input (usually emissive, ambient, diffuse or specular).  These
		 * assigments are used in the vertex shader to pass the texcoords to be
		 * interpolated to the rasterizer.  If the color input does not have
		 * a sampler, no code is generated.
		 * @param {!o3d.Material} material The material to inspect.
		 * @param {string} name The name of the color input.
		 * @return {string} The code for the texture coordinate passthrough statement.
		 */
		std::string buildUVPassthrough(
		    Material* material, const std::string& name) {
			if(material->GetUntypedParam(name + "Sampler")) {
				std::string sourceName(name + "UV");
				std::string destName(sourceName);
				std::string semantic = name_to_semantic_map_[sourceName];

				if(!semantic.empty()) {
					sourceName = getAttributeName(sourceName, semantic);
				}

				return std::string("  ") + VERTEX_VARYING_PREFIX + destName + " = " +
				       ATTRIBUTE_PREFIX + sourceName + ";\n";
			}
			else {
				return "";
			}
		};


		/**
		 * Builds all the texture coordinate passthrough statements for the
		 * vertex shader.
		 * @param {!o3d.Material} material The material to inspect.
		 * @return {string} The code for the texture coordinate passthrough
		 *                  statements.
		 */
		std::string buildUVPassthroughs(Material* material) {
			// TODO(petersont): in the GLSL implementation we need to generate
			// the code for these attributes before we can pass their values
			// through, because in this implementation their names must be their
			// semantics (i.e., "texCoord4") rather than these chosen names.
			// Currently bumpUV is the only one which does not obey this rule.
			std::string str;
			str += buildUVPassthrough(material, "emissive");
			str += buildUVPassthrough(material, "ambient");
			str += buildUVPassthrough(material, "diffuse");
			str += buildUVPassthrough(material, "specular");
			str += buildUVPassthrough(material, "bump");
			return str;
		};


		/**
		 * Builds bump input coords if needed.
		 * @param {boolean} bumpSampler Whether there is a bump sampler.
		 * @return {string} The code for bump input coords.
		 */
		std::string buildBumpInputCoords(bool bumpSampler) {
			std::string str;

			if(bumpSampler) {
				str += std::string("  ") + ATTRIBUTE + FLOAT3 + " tangent" +
				       semanticSuffix("TANGENT") + ";\n";
				str += std::string("  ") + ATTRIBUTE + FLOAT3 + " binormal" +
				       semanticSuffix("BINORMAL") + ";\n";
				str += std::string("  ") + ATTRIBUTE + FLOAT2 + " bumpUV" +
				       semanticSuffix(
				           std::string("TEXCOORD") + interpolant_.Inc()) + ";\n";
			}

			return str;
		};


		/**
		 * Builds bump output coords if needed.
		 * @param {boolean} bumpSampler Whether there is a bump sampler.
		 * @return {string} The code for bump input coords.
		 */
		std::string buildBumpOutputCoords(bool bumpSampler) {
			std::string str;

			if(bumpSampler) {
				str += std::string("  ") +
				       VARYING + FLOAT3 + " " + VARYING_DECLARATION_PREFIX + "tangent" +
				       semanticSuffix(std::string("TEXCOORD") + interpolant_.Inc()) + ";\n";
				str += std::string("  ") +
				       VARYING + FLOAT3 + " " + VARYING_DECLARATION_PREFIX + "binormal" +
				       semanticSuffix(std::string("TEXCOORD") + interpolant_.Inc()) + ";\n";
				str += std::string("  ") +
				       VARYING + FLOAT2 + " " + VARYING_DECLARATION_PREFIX + "bumpUV" +
				       semanticSuffix(std::string("TEXCOORD") + interpolant_.Inc()) + ";\n";
			}

			return str;
		};


		/**
		 * Builds vertex and fragment shader string for a 2-color checker effect.
		 * @return {string} The effect code for the shader, ready to be parsed.
		 */
		std::string buildCheckerShaderString() {
			std::string varyingDecls;
			varyingDecls += std::string(BEGIN_OUT_STRUCT) +
			                VARYING + FLOAT4 + " " +
			                VERTEX_VARYING_PREFIX + "position" +
			                semanticSuffix("POSITION") + ";\n";
			varyingDecls += std::string("") + VARYING + FLOAT2 + " " +
			                VERTEX_VARYING_PREFIX + "texCoord" +
			                semanticSuffix("TEXCOORD0") + ";\n";
			varyingDecls += std::string("") + VARYING + FLOAT3 + " " +
			                VERTEX_VARYING_PREFIX + "normal" +
			                semanticSuffix("TEXCOORD1") + ";\n";
			varyingDecls += std::string("") + VARYING + FLOAT3 + " " +
			                VERTEX_VARYING_PREFIX + "worldPosition" +
			                semanticSuffix("TEXCOORD2") + ";\n" +
			                END_STRUCT;
			std::string str;
			str += std::string("uniform ") + MATRIX4 + " worldViewProjection" +
			       semanticSuffix("WORLDVIEWPROJECTION") + ";\n";
			str += std::string("uniform ") + MATRIX4 + " worldInverseTranspose" +
			       semanticSuffix("WORLDINVERSETRANSPOSE") + ";\n";
			str += std::string("uniform ") + MATRIX4 + " world" +
			       semanticSuffix("WORLD") + ";\n";
			str += "\n";
			str += BEGIN_IN_STRUCT;
			str += std::string("") + ATTRIBUTE + FLOAT4 + " position" +
			       semanticSuffix("POSITION") + ";\n";
			str += std::string("") + ATTRIBUTE + FLOAT3 + " normal" +
			       semanticSuffix("NORMAL") + ";\n";
			str += std::string("") + ATTRIBUTE + FLOAT2 + " texCoord0" +
			       semanticSuffix("TEXCOORD0") + ";\n";
			str += END_STRUCT;
			str += "\n";
			str += varyingDecls + "\n";
			str += beginVertexShaderMain();
			str += std::string("  ") + VERTEX_VARYING_PREFIX + "position = " +
			       mul(std::string(ATTRIBUTE_PREFIX) + "position",
			           "worldViewProjection") + ";\n";
			str += std::string("  ") + VERTEX_VARYING_PREFIX + "normal = " +
			       mul(std::string(FLOAT4) + "(" +
			           ATTRIBUTE_PREFIX + "normal, 0.0)",
			           "worldInverseTranspose") + ".xyz;\n";
			str += std::string("  ") + VERTEX_VARYING_PREFIX + "worldPosition = " +
			       mul(std::string(ATTRIBUTE_PREFIX) + "position", "world") + ".xyz;\n";
			str += std::string("  ") + VERTEX_VARYING_PREFIX + "texCoord = " +
			       ATTRIBUTE_PREFIX + "texCoord0;\n";
			str += endVertexShaderMain();
			str += "\n";
			str += pixelShaderHeader(NULL, false, false, NULL) +
			       "uniform " + FLOAT4 + " color1;\n" +
			       "uniform " + FLOAT4 + " color2;\n" +
			       "uniform float checkSize;\n" +
			       "uniform " + FLOAT3 + " lightWorldPos;\n" +
			       "uniform " + FLOAT3 + " lightColor;\n" +
			       "\n";
			str += repeatVaryingDecls(&varyingDecls) +
			       FLOAT4 + " checker(" + FLOAT2 + " uv) {\n" +
			       "  float fmodResult = " + MOD + "(" +
			       "    floor(checkSize * uv.x) + \n" +
			       "    floor(checkSize * uv.y), 2.0);\n" +
			       "  return (fmodResult < 1.0) ? color1 : color2;\n" +
			       "}\n\n";
			str += beginPixelShaderMain() +
			       "  " + FLOAT3 + " surfaceToLight = \n" +
			       "      normalize(lightWorldPos - " +
			       PIXEL_VARYING_PREFIX + "worldPosition);\n" +
			       "  " + FLOAT3 + " worldNormal = normalize(" +
			       PIXEL_VARYING_PREFIX + "normal);\n" +
			       "  " + FLOAT4 + " check = checker(" +
			       PIXEL_VARYING_PREFIX + "texCoord);\n" +
			       "  float directionalIntensity = \n" +
			       "      clamp(dot(worldNormal, surfaceToLight), 0.0, 1.0);\n" +
			       "  " + FLOAT4 +
			       " outColor = directionalIntensity * check;\n";
			str += endPixelShaderMain(
			           std::string(FLOAT4) + "(outColor.rgb, check.a)");
			str += "\n";
			str + entryPoints();
			str += matrixLoadOrder();
			return str;
		};

		/**
		 * Extracts the texture type from a texture param.
		 * @param {!o3d.ParamTexture} textureParam The texture parameter to
		 *     inspect.
		 * @return {string} The texture type (1D, 2D, 3D or CUBE).
		 */
		std::string getTextureType(ParamTexture* textureParam) {
			Texture* texture = textureParam->value();

			if(!texture) {
				return "2D";  // No texture value, have to make a guess.
			}

			if(texture->IsA(Texture2D::GetApparentClass())) {
				return "2D";
			}

			if(texture->IsA(TextureCUBE::GetApparentClass())) {
				return "CUBE";
			}

			return "2D";
		}

		/**
		 * Extracts the sampler type from a sampler param.  It does it by inspecting
		 * the texture associated with the sampler.
		 * @param {!o3d.ParamTexture} samplerParam The texture parameter to
		 *     inspect.
		 * @return {string} The texture type (1D, 2D, 3D or CUBE).
		 */
		std::string getSamplerType(ParamSampler* samplerParam) {
			Sampler* sampler = samplerParam->value();

			if(!sampler) {
				return "2D";
			}

			ParamTexture* textureParam =
			    sampler->GetParam<ParamTexture>(Sampler::kTextureParamName);

			if(textureParam) {
				return getTextureType(textureParam);
			}
			else {
				return "2D";
			}
		};


		/**
		 * Builds uniform variables common to all standard lighting types.
		 * @return {string} The effect code for the common shader uniforms.
		 */
		std::string buildCommonVertexUniforms() {
			return std::string("uniform ") + MATRIX4 + " worldViewProjection" +
			       semanticSuffix("WORLDVIEWPROJECTION") + ";\n" +
			       "uniform " + FLOAT3 + " lightWorldPos;\n";
		};

		/**
		 * Builds uniform variables common to all standard lighting types.
		 * @return {string} The effect code for the common shader uniforms.
		 */
		std::string buildCommonPixelUniforms() {
			return std::string("uniform ") + FLOAT4 + " lightColor;\n";
		};

		/**
		 * Builds uniform variables common to lambert, phong and blinn lighting types.
		 * @return {string} The effect code for the common shader uniforms.
		 */
		std::string buildLightingUniforms() {
			std::string str;
			str += std::string("uniform ") + MATRIX4 + " world" +
			       semanticSuffix("WORLD") + ";\n";
			str += std::string("uniform ") + MATRIX4 +
			       " viewInverse" + semanticSuffix("VIEWINVERSE") + ";\n";
			str += std::string("uniform ") + MATRIX4 + " worldInverseTranspose" +
			       semanticSuffix("WORLDINVERSETRANSPOSE") + ";\n";
			return str;
		};

		/**
		 * Builds uniform parameters for a given color input.  If the material
		 * has a sampler parameter, a sampler uniform is created, otherwise a
		 * float4 color value is created.
		 * @param {!o3d.Material} material The material to inspect.
		 * @param {!Array.<string>} descriptions Array to add descriptions too.
		 * @param {string} name The name of the parameter to look for.  Usually
		 *     emissive, ambient, diffuse or specular.
		 * @param {boolean} opt_addColorParam Whether to add a color param if no
		 *     sampler exists. Default = true.
		 * @return {string} The effect code for the uniform parameter.
		 */
		std::string buildColorParam(
		    Material* material,
		    std::vector<std::string>* descriptions,
		    const std::string& name,
		    bool addColorParam) {
			ParamSampler* samplerParam =
			    material->GetParam<ParamSampler>(name + "Sampler");

			if(samplerParam) {
				std::string type = getSamplerType(samplerParam);
				descriptions->push_back(name + type + "Texture");
				return std::string("uniform sampler") + type + " " + name + "Sampler;\n";
			}
			else if(addColorParam) {
				descriptions->push_back(name + "Color");
				return std::string("uniform ") + FLOAT4 + " " + name + ";\n";
			}
			else {
				return "";
			}
		};

		/**
		 * Builds the effect code to retrieve a given color input.  If the material
		 * has a sampler parameter of that name, a texture lookup is done.  Otherwise
		 * it"s a no-op, since the value is retrieved directly from the color uniform
		 * of that name.
		 * @param {!o3d.Material} material The material to inspect.
		 * @param {string} name The name of the parameter to look for.  Usually
		 *                      emissive, ambient, diffuse or specular.
		 * @return {string} The effect code for the uniform parameter retrieval.
		 */
		std::string getColorParam(Material* material, const std::string& name) {
			ParamSampler* samplerParam =
			    material->GetParam<ParamSampler>(name + "Sampler");

			if(samplerParam) {
				std::string type = getSamplerType(samplerParam);
				return std::string("  ") + FLOAT4 + " " + name + " = " + TEXTURE + type +
				       "(" + name + "Sampler, " +
				       PIXEL_VARYING_PREFIX + name + "UV);\n";
			}
			else {
				return "";
			}
		};

		/**
		 * Builds vertex and fragment shader string for the Constant lighting type.
		 * @param {!o3d.Material} material The material for which to build
		 *     shaders.
		 * @param {!Array.<string>} descriptions Array to add descriptions too.
		 * @return {string} The effect code for the shader, ready to be parsed.
		 */
		std::string buildConstantShaderString(
		    Material* material,
		    ParamSampler* bumpSampler,
		    std::vector<std::string>* descriptions) {
			descriptions->push_back("constant");
			std::string str;
			str += buildCommonVertexUniforms();
			str += buildVertexDecls(material, false, false, bumpSampler);
			str += beginVertexShaderMain();
			str += positionVertexShaderCode();
			str += buildUVPassthroughs(material);
			str += endVertexShaderMain();
			str += pixelShaderHeader(material, false, false, bumpSampler);
			str += buildCommonPixelUniforms();
			str += repeatVaryingDecls(NULL);
			str += buildColorParam(material, descriptions, "emissive", true);
			str += beginPixelShaderMain();
			str += getColorParam(material, "emissive");
			str += endPixelShaderMain("emissive");
			str += entryPoints();
			str += matrixLoadOrder();
			return str;
		};

		/**
		 * Builds vertex and fragment shader string for the Lambert lighting type.
		 * @param {!o3d.Material} material The material for which to build
		 *     shaders.
		 * @param {!Array.<string>} descriptions Array to add descriptions too.
		 * @return {string} The effect code for the shader, ready to be parsed.
		 */
		std::string buildLambertShaderString(
		    Material* material,
		    ParamSampler* bumpSampler,
		    std::vector<std::string>* descriptions) {
			descriptions->push_back("lambert");
			std::string str;
			str += buildCommonVertexUniforms();
			str += buildLightingUniforms();
			str += buildVertexDecls(material, true, false, bumpSampler);
			str += beginVertexShaderMain();
			str += buildUVPassthroughs(material);
			str += positionVertexShaderCode();
			str += normalVertexShaderCode();
			str += surfaceToLightVertexShaderCode();
			str += bumpVertexShaderCode(bumpSampler);
			str += endVertexShaderMain();
			str += pixelShaderHeader(material, true, false, bumpSampler);
			str += buildCommonPixelUniforms();
			str += repeatVaryingDecls(NULL);
			str += buildColorParam(material, descriptions, "emissive", true);
			str += buildColorParam(material, descriptions, "ambient", true);
			str += buildColorParam(material, descriptions, "diffuse", true);
			str += buildColorParam(material, descriptions, "bump", false);
			str += utilityFunctions();
			str += beginPixelShaderMain();
			str += getColorParam(material, "emissive");
			str += getColorParam(material, "ambient");
			str += getColorParam(material, "diffuse");
			str += getNormalShaderCode(bumpSampler) +
			       "  " + FLOAT3 + " surfaceToLight = normalize(" +
			       PIXEL_VARYING_PREFIX + "surfaceToLight);\n" +
			       "  " + FLOAT4 +
			       " litR = lit(dot(normal, surfaceToLight), 0.0, 0.0);\n";
			str += endPixelShaderMain(std::string(FLOAT4) +
			                          "((emissive +\n" +
			                          "      lightColor *" +
			                          " (ambient * diffuse + diffuse * litR.y)).rgb,\n" +
			                          "          diffuse.a)");
			str += entryPoints();
			str += matrixLoadOrder();
			return str;
		};

		/**
		 * Builds vertex and fragment shader string for the Blinn lighting type.
		 * @param {!o3d.Material} material The material for which to build
		 *     shaders.
		 * @param {!Array.<string>} descriptions Array to add descriptions too.
		 * @return {string} The effect code for the shader, ready to be parsed.
		 * TODO: This is actually just a copy of the Phong code.
		 *     Change to Blinn.
		 */
		std::string buildBlinnShaderString(
		    Material* material,
		    ParamSampler* bumpSampler,
		    std::vector<std::string>* descriptions) {
			descriptions->push_back("phong");
			std::string str;
			str += buildCommonVertexUniforms();
			str += buildLightingUniforms();
			str += buildVertexDecls(material, true, true, bumpSampler);
			str += beginVertexShaderMain();
			str += buildUVPassthroughs(material);
			str += positionVertexShaderCode();
			str += normalVertexShaderCode();
			str += surfaceToLightVertexShaderCode();
			str += surfaceToViewVertexShaderCode();
			str += bumpVertexShaderCode(bumpSampler);
			str += endVertexShaderMain();
			str += pixelShaderHeader(material, true, true, bumpSampler);
			str += buildCommonPixelUniforms();
			str += repeatVaryingDecls(NULL);
			str += buildColorParam(material, descriptions, "emissive", true);
			str += buildColorParam(material, descriptions, "ambient", true);
			str += buildColorParam(material, descriptions, "diffuse", true);
			str += buildColorParam(material, descriptions, "specular", true);
			str += buildColorParam(material, descriptions, "bump", false);
			str += "uniform float shininess;\n";
			str += "uniform float specularFactor;\n";
			str += utilityFunctions();
			str += beginPixelShaderMain();
			str += getColorParam(material, "emissive");
			str += getColorParam(material, "ambient");
			str += getColorParam(material, "diffuse");
			str += getColorParam(material, "specular");
			str += getNormalShaderCode(bumpSampler) +
			       "  " + FLOAT3 + " surfaceToLight = normalize(" +
			       PIXEL_VARYING_PREFIX + "surfaceToLight);\n" +
			       "  " + FLOAT3 + " surfaceToView = normalize(" +
			       PIXEL_VARYING_PREFIX + "surfaceToView);\n" +
			       "  " + FLOAT3 +
			       " halfVector = normalize(surfaceToLight + " +
			       PIXEL_VARYING_PREFIX + "surfaceToView);\n" +
			       "  " + FLOAT4 +
			       " litR = lit(dot(normal, surfaceToLight), \n" +
			       "                    dot(normal, halfVector), shininess);\n";
			str += endPixelShaderMain(std::string(FLOAT4) +
			                          "((emissive +\n" +
			                          "  lightColor *" +
			                          " (ambient * diffuse + diffuse * litR.y +\n" +
			                          "                        + specular * litR.z *" +
			                          " specularFactor)).rgb,\n" +
			                          "      diffuse.a)");
			str += entryPoints();
			str += matrixLoadOrder();
			return str;
		};

		/**
		 * Builds vertex and fragment shader string for the Phong lighting type.
		 * @param {!o3d.Material} material The material for which to build
		 *     shaders.
		 * @param {!Array.<string>} descriptions Array to add descriptions too.
		 * @return {string} The effect code for the shader, ready to be parsed.
		 */
		std::string buildPhongShaderString(
		    Material* material,
		    ParamSampler* bumpSampler,
		    std::vector<std::string>* descriptions) {
			descriptions->push_back("phong");
			std::string str;
			str += buildCommonVertexUniforms();
			str += buildLightingUniforms();
			str += buildVertexDecls(material, true, true, bumpSampler);
			str += beginVertexShaderMain();
			str += buildUVPassthroughs(material);
			str += positionVertexShaderCode();
			str += normalVertexShaderCode();
			str += surfaceToLightVertexShaderCode();
			str += surfaceToViewVertexShaderCode();
			str += bumpVertexShaderCode(bumpSampler);
			str += endVertexShaderMain();
			str += pixelShaderHeader(material, true, true, bumpSampler);
			str += buildCommonPixelUniforms();
			str += repeatVaryingDecls(NULL);
			str += buildColorParam(material, descriptions, "emissive", true);
			str += buildColorParam(material, descriptions, "ambient", true);
			str += buildColorParam(material, descriptions, "diffuse", true);
			str += buildColorParam(material, descriptions, "specular", true);
			str += buildColorParam(material, descriptions, "bump", false);
			str += "uniform float shininess;\n";
			str += "uniform float specularFactor;\n";
			str += utilityFunctions();
			str += beginPixelShaderMain();
			str += getColorParam(material, "emissive");
			str += getColorParam(material, "ambient");
			str += getColorParam(material, "diffuse");
			str += getColorParam(material, "specular");
			str += getNormalShaderCode(bumpSampler) +
			       "  " + FLOAT3 + " surfaceToLight = normalize(" +
			       PIXEL_VARYING_PREFIX + "surfaceToLight);\n" +
			       "  " + FLOAT3 + " surfaceToView = normalize(" +
			       PIXEL_VARYING_PREFIX + "surfaceToView);\n" +
			       "  " + FLOAT3 +
			       " halfVector = normalize(surfaceToLight + surfaceToView);\n" +
			       "  " + FLOAT4 +
			       " litR = lit(dot(normal, surfaceToLight), \n" +
			       "                    dot(normal, halfVector), shininess);\n";
			str += endPixelShaderMain(std::string(FLOAT4) +
			                          "((emissive +\n" +
			                          "  lightColor * (ambient * diffuse + diffuse * litR.y +\n" +
			                          "                        + specular * litR.z *" +
			                          " specularFactor)).rgb,\n" +
			                          "      diffuse.a)");
			str += entryPoints();
			str += matrixLoadOrder();
			return str;
		};

		/**
		 * Builds the position code for the vertex shader.
		 * @return {string} The code for the vertex shader.
		 */
		std::string positionVertexShaderCode() {
			return std::string("  ") + VERTEX_VARYING_PREFIX + "position = " +
			       mul(std::string(ATTRIBUTE_PREFIX) +
			           "position", "worldViewProjection") + ";\n";
		};

		/**
		 * Builds the normal code for the vertex shader.
		 * @return {string} The code for the vertex shader.
		 */
		std::string normalVertexShaderCode() {
			return std::string("  ") + VERTEX_VARYING_PREFIX + "normal = " +
			       mul(std::string(FLOAT4) + "(" +
			           ATTRIBUTE_PREFIX +
			           "normal, 0)", "worldInverseTranspose") + ".xyz;\n";
		};

		/**
		 * Builds the surface to light code for the vertex shader.
		 * @return {string} The code for the vertex shader.
		 */
		std::string surfaceToLightVertexShaderCode() {
			return std::string("  ") + VERTEX_VARYING_PREFIX +
			       "surfaceToLight = lightWorldPos - \n" +
			       "                          " +
			       mul(std::string(ATTRIBUTE_PREFIX) + "position",
			           "world") + ".xyz;\n";
		};

		/**
		 * Builds the surface to view code for the vertex shader.
		 * @return {string} The code for the vertex shader.
		 */
		std::string surfaceToViewVertexShaderCode() {
			return std::string("  ") + VERTEX_VARYING_PREFIX +
			       "surfaceToView = (viewInverse[3] - " +
			       mul(std::string(ATTRIBUTE_PREFIX) + "position", "world") + ").xyz;\n";
		};

		/**
		 * Builds the normal map part of the vertex shader.
		 * @param {boolean} opt_bumpSampler Whether there is a bump
		 *     sampler. Default = false.
		 * @return {string} The code for normal mapping in the vertex shader.
		 */
		std::string bumpVertexShaderCode(ParamSampler* opt_bumpSampler) {
			std::string str("");

			if(opt_bumpSampler) {
				str += std::string("  ") + VERTEX_VARYING_PREFIX + "binormal = " +
				       mul(std::string(FLOAT4) + "(" +
				           ATTRIBUTE_PREFIX + "binormal, 0)",
				           "worldInverseTranspose") + ".xyz;\n";
				str += std::string("  ") + VERTEX_VARYING_PREFIX + "tangent = " +
				       mul(std::string(FLOAT4) +
				           "(" + ATTRIBUTE_PREFIX + "tangent, 0)",
				           "worldInverseTranspose") + ".xyz;\n";
			}

			return str;
		};

		/**
		 * Builds the normal calculation of the pixel shader.
		 * @return {string} The code for normal computation in the pixel shader.
		 */
		std::string getNormalShaderCode(ParamSampler* bumpSampler) {
			return bumpSampler ?
			       (std::string(MATRIX3) + " tangentToWorld = " + MATRIX3 +
			        "(" + PIXEL_VARYING_PREFIX + "tangent,\n" +
			        "                                   " +
			        PIXEL_VARYING_PREFIX + "binormal,\n" +
			        "                                   " +
			        PIXEL_VARYING_PREFIX + "normal);\n" +
			        FLOAT3 + " tangentNormal = " + TEXTURE + "2D(bumpSampler, " +
			        PIXEL_VARYING_PREFIX + "bumpUV.xy).xyz -\n" +
			        "                       " + FLOAT3 +
			        "(0.5, 0.5, 0.5);\n" + FLOAT3 + " normal = " +
			        mul("tangentNormal", "tangentToWorld") + ";\n" +
			        "normal = normalize(normal);\n") :
			       std::string("  ") + FLOAT3 + " normal = normalize(" +
			       PIXEL_VARYING_PREFIX + "normal);\n";
		};

		/**
		 * Builds the vertex declarations for a given material.
		 * @param {!o3d.Material} material The material to inspect.
		 * @param {boolean} diffuse Whether to include stuff for diffuse
		 *     calculations.
		 * @param {boolean} specular Whether to include stuff for diffuse
		 *     calculations.
		 * @return {string} The code for the vertex declarations.
		 */
		std::string buildVertexDecls(
		    Material* material,
		    bool diffuse,  bool specular,
		    ParamSampler* bumpSampler) {
			std::string str;
			str += buildAttributeDecls(material, diffuse, specular, bumpSampler);
			str += buildVaryingDecls(material, diffuse, specular, bumpSampler);
			return str;
		};


		/**
		 * Builds a shader string for a given standard COLLADA material type.
		 *
		 * @param {!o3d.Material} material Material for which to build the shader.
		 * @param {string} effectType Type of effect to create ("phong", "lambert",
		 *     "constant").
		 * @return {{description: string, shader: string}} A description and the shader
		 *     string.
		 */
		std::string buildStandardShaderString(
		    Material* material,
		    const std::string& effectType,
		    std::string* description) {
			ParamSampler* bumpSampler =
			    material->GetParam<ParamSampler>("bumpSampler");
			// Create a shader string of the appropriate type, based on the
			// effectType.
			std::string str;
			std::vector<std::string> descriptions;

			if(effectType == "phong") {
				str = buildPhongShaderString(material, bumpSampler, &descriptions);
			}
			else if(effectType == "lambert") {
				str = buildLambertShaderString(material, bumpSampler, &descriptions);
			}
			else if(effectType == "blinn") {
				str = buildBlinnShaderString(material, bumpSampler, &descriptions);
			}
			else if(effectType == "constant") {
				str = buildConstantShaderString(material, bumpSampler, &descriptions);
			}
			else {
				O3D_NEVER_REACHED() << "unknown effect type "" + effectType + """;
			}

			description->clear();

			for(size_t ii = 0; ii < descriptions.size(); ++ii) {
				*description += descriptions[ii];
			}

			return str;
		};

		/**
		 * Gets or builds a shader for given standard COLLADA material type.
		 *
		 * Looks at the material passed in and assigns it an Effect that matches its
		 * Params. If a suitable Effect already exists in pack it will use that Effect.
		 *
		 * @param {!o3d.Pack} pack Pack in which to create the new Effect.
		 * @param {!o3d.Material} material Material for which to build the shader.
		 * @param {string} effectType Type of effect to create ("phong", "lambert",
		 *     "constant").
		 * @return {o3d.Effect} The created effect.
		 */
		Effect* getStandardShader(
		    Pack* pack,
		    Material* material,
		    const std::string& effectType) {
			std::string description;
			std::string shader = buildStandardShaderString(
			                         material, effectType, &description);
			std::vector<Effect*> effects = pack->GetByClass<Effect>();
			Effect* effect = NULL;

			for(size_t ii = 0; ii < effects.size(); ++ii) {
				effect = effects[ii];

				if(effect->name().compare(description) == 0 &&
				        effect->source().compare(shader) == 0) {
					return effect;
				}
			}

			effect = pack->Create<Effect>();

			if(effect) {
				effect->set_name(description);
				O3D_LOG(INFO) << "-----------------------------------SHADER--";
				DumpMultiLineString(shader);
				O3D_LOG(INFO) << "\n--------------END--";

				if(effect->LoadFromFXString(shader)) {
					return effect;
				}

				O3D_LOG(INFO) << "LoadFromFXString failed";
				pack->RemoveObject(effect);
			}

			return NULL;
		};

		/**
		 * Attaches a shader for a given standard COLLADA material type to the
		 * material.
		 *
		 * Looks at the material passed in and assigns it an Effect that matches its
		 * Params. If a suitable Effect already exists in pack it will use that Effect.
		 *
		 * @param {!o3d.Pack} pack Pack in which to create the new Effect.
		 * @param {!o3d.Material} material Material for which to build the shader.
		 * @param {!o3djs.math.Vector3} lightPos Position of the default light.
		 * @param {string} effectType Type of effect to create ("phong", "lambert",
		 *     "constant").
		 * @return {boolean} True on success.
		 */
		bool attachStandardShader(
		    Pack* pack,
		    Material* material,
		    const Vector3& lightPos,
		    const std::string& effectType) {
			Effect* effect = getStandardShader(pack, material, effectType);

			if(effect) {
				material->set_effect(effect);
				effect->CreateUniformParameters(material);
				// Set a couple of the default parameters in the hopes that this will
				// help the user get something on the screen. We check to make sure they
				// are not connected to something otherwise we"ll get an error.
				ParamFloat3* light_param =
				    material->GetParam<ParamFloat3>("lightWorldPos");

				if(light_param && !light_param->input_connection()) {
					light_param->set_value(
					    Float3(lightPos.getX(), lightPos.getY(), lightPos.getZ()));
				}

				ParamFloat4* color_param =
				    material->GetParam<ParamFloat4>("lightColor");

				if(color_param && !color_param->input_connection()) {
					color_param->set_value(Float4(1.0f, 1.0f, 1.0f, 1.0f));
				}

				return true;
			}
			else {
				return false;
			}
		};

		/**
		 * Creates an effect that draws a 2 color procedural checker pattern.
		 * @param {!o3d.Pack} pack The pack to create the effect in. If the pack
		 *     already has an effect with the same name that effect will be returned.
		 * @return {!o3d.Effect} The effect.
		 */
		Effect* createCheckerEffect(Pack* pack) {
			ObjectBaseArray effects =
			    pack->GetObjects(Effect::GetApparentClass()->name(),
			                     TWO_COLOR_CHECKER_EFFECT_NAME);

			if(!effects.empty()) {
				return static_cast<Effect*>(effects[0]);
			}

			Effect* effect = pack->Create<Effect>();
			effect->LoadFromFXString(buildCheckerShaderString());
			effect->set_name(TWO_COLOR_CHECKER_EFFECT_NAME);
			return effect;
		};

	};

	ShaderBuilder* ShaderBuilder::Create() {
		return new GLSLShaderBuilder();
	}

	/**
	 * Creates the uniform parameters needed for an Effect on the given ParamObject.
	 * @param {!o3d.Pack} pack Pack to create extra objects in like Samplers and
	 *     ParamArrays.
	 * @param {!o3d.Effect} effect Effect.
	 * @param {!o3d.ParamObject} paramObject ParamObject on which to create Params.
	 */
	void ShaderBuilder::createUniformParameters(
	    Pack* pack,
	    Effect* effect,
	    ParamObject* paramObject) {
		effect->CreateUniformParameters(paramObject);
		EffectParameterInfoArray infos;
		effect->GetParameterInfo(&infos);

		for(size_t ii = 0; ii < infos.size(); ++ii) {
			const EffectParameterInfo& info = infos[ii];

			if(info.sas_class_type() == NULL) {
				if(info.num_elements() > 0) {
					ParamArray* paramArray = pack->Create<ParamArray>();
					ParamParamArray* param =
					    paramObject->GetParam<ParamParamArray>(info.name());
					param->set_value(paramArray);
					paramArray->ResizeByClass(info.num_elements(), info.class_type());

					if(ObjectBase::ClassIsA(
					            info.class_type(),
					            ParamSampler::GetApparentClass())) {
						for(size_t jj = 0; jj < (size_t)info.num_elements(); ++jj) {
							Sampler* sampler = pack->Create<Sampler>();
							paramArray->GetParam<ParamSampler>(jj)->set_value(sampler);
						}
					}
				}
				else if(ObjectBase::ClassIsA(
				            info.class_type(),
				            ParamSampler::GetApparentClass())) {
					Sampler* sampler = pack->Create<Sampler>();
					ParamSampler* param =
					    paramObject->GetParam<ParamSampler>(info.name());
					param->set_value(sampler);
				}
			}
		}
	};



}  // namespace o3d_utils



