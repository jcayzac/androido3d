// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shader_builder.h"

namespace o3d {

#if 0
class Material;

}

namespace o3d_utils {

class GLSLShaderBuilder {
 private
  /**
   * Caches the varying parameter declarations to be repeated in the case that
   * we"re in glsl and need to declare the varying parameters in both shaders.
   * @type {string}
   */
  const std::string* varying_decls_;

  /**
   * An integer value which keeps track of the next available interpolant.
   * @type {number}
   * @private
   */
  IntStringThing interpolant_;

  std::map<std::string, std::string> name_to_semantic_map_;

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

    for (size_t ii = 0; ii < sizeof(s_map) / sizeof(s_map[0]); ++ii) {
      if (semantic.compare(s_map[ii][0]) == 0) {
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
  std::string pixelShaderHeader() {
    return "\n// #o3d SplitMarker\n";
  };

  /**
   * Repeats the declarations for the varying parameters if necessary.
   * @param {string} opt_decls The declarations if you know them already.
   * @return {string} Code for the parameter declarations.
   */
  std::string repeatVaryingDecls(const std::string* opt_decls) {
    return (opt_decls ? *opt_decls :
            (varying_decls_ ? *varying_decls_ : buildVaryingDecls())) +
           "\n";
  };

  /**
   * The string that goes infront of the pixel shader main.
   * @return {string} The effect code for the start of the main.
   */
  std::string beginPixelShaderMain() {
    return std::string("void main() {\n");
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
      o3d::Material* material,
      bool diffuse,
      bool specular,
      bool bumpSampler) {
    std::string str(BEGIN_IN_STRUCT) +
              ATTRIBUTE + FLOAT4 + " " + "position" +
              semanticSuffix("POSITION") + ";\n";
    if (diffuse || specular) {
      str += ATTRIBUTE + FLOAT3 + " " + "normal" +
             semanticSuffix("NORMAL") + ";\n";
    }
    str += buildTexCoords(material, false) +
           buildBumpInputCoords(bumpSampler) +
           END_STRUCT;
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
      o3d::Material* material,
      bool diffuse,
      bool specular,
      bool bumpSampler) {
    std::string str(BEGIN_OUT_STRUCT) +
        VARYING + FLOAT4 + " " +
        VARYING_DECLARATION_PREFIX + "position" +
        semanticSuffix("POSITION") + ";\n" +
        buildTexCoords(material, true) +
        buildBumpOutputCoords(bumpSampler);
    if (diffuse || specular) {
      str += VARYING + FLOAT3 + " " +
          VARYING_DECLARATION_PREFIX + "normal" +
          semanticSuffix("TEXCOORD" +
             interpolant_++ + "") + ";\n" +
          VARYING + FLOAT3 + " " +
          VARYING_DECLARATION_PREFIX + "surfaceToLight" +
          semanticSuffix(
              "TEXCOORD" + interpolant_++ + "") + ";\n";
    }
    if (specular) {
      str += VARYING + FLOAT3 + " " +
          VARYING_DECLARATION_PREFIX + "surfaceToView" +
          semanticSuffix(
              "TEXCOORD" + interpolant_++ + "") + ";\n";
    }
    str += END_STRUCT;
    varying_decls_ = new std::string(str);
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
      o3d::Material* material, bool varying, const std::string& name) {
    // In the GLSL version we need to name the incoming attributes by
    // the semantic name in order for them to get hooked up correctly.
    if (material->GetUntypedParam(name + "Sampler")) {
      if (varying) {
        return std::string("  ") + VARYING + FLOAT2 + " " +
            VARYING_DECLARATION_PREFIX + name + "UV" +
            semanticSuffix(
                "TEXCOORD" + interpolant_++ + "") + ";\n";
      } else {
        std::string desiredName(name + "UV");
        std::string semantic("TEXCOORD") + interpolant_++;
        std::string outputName(getAttributeName_(desiredName, semantic));
        name_to_semantic_map_[desiredName] = semantic;
        return std::string("  ") + ATTRIBUTE + FLOAT2 + " " + outputName +
            semanticSuffix(semantic) + ";\n";
      }
    } else {
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
  std::string buildTexCoords(o3d::Material* aterial, bool varying) {
    interpolant_ = 0;
    if (!varying) {
      name_to_semantic_map_.clear();
    }
    return buildTexCoord(material, varying, "emissive") +
           buildTexCoord(material, varying, "ambient") +
           buildTexCoord(material, varying, "diffuse") +
           buildTexCoord(material, varying, "specular");
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
      o3d::Material* material, const std::string& name) {
    if (material->GetUntypedParam(name + "Sampler")) {
      std::string sourceName(name + "UV");
      std::string destName(sourceName);
      std::string semantic = name_to_semantic_map_[sourceName];
      if (!semantic.empty()) {
        sourceName = getAttributeName(sourceName, semantic);
      }
      return std::string("  ") + VERTEX_VARYING_PREFIX + destName + " = " +
          ATTRIBUTE_PREFIX + sourceName + ";\n";
    } else {
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
  std::string buildUVPassthroughs(o3d::Material* material) {
    // TODO(petersont): in the GLSL implementation we need to generate
    // the code for these attributes before we can pass their values
    // through, because in this implementation their names must be their
    // semantics (i.e., "texCoord4") rather than these chosen names.
    // Currently bumpUV is the only one which does not obey this rule.
    return buildUVPassthrough(material, "emissive") +
           buildUVPassthrough(material, "ambient") +
           buildUVPassthrough(material, "diffuse") +
           buildUVPassthrough(material, "specular") +
           buildUVPassthrough(material, "bump");
  };


  /**
   * Builds bump input coords if needed.
   * @param {boolean} bumpSampler Whether there is a bump sampler.
   * @return {string} The code for bump input coords.
   */
  std::string buildBumpInputCoords(bool bumpSampler) {
    return std::string() + bumpSampler ?
        ("  " + FLOAT3 + " tangent" +
            semanticSuffix("TANGENT") + ";\n" +
         "  " + FLOAT3 + " binormal" +
            semanticSuffix("BINORMAL") + ";\n" +
         "  " + FLOAT2 + " bumpUV" +
            semanticSuffix(
                "TEXCOORD" + interpolant_++) + ";\n") : "";
  };


  /**
   * Builds bump output coords if needed.
   * @param {boolean} bumpSampler Whether there is a bump sampler.
   * @return {string} The code for bump input coords.
   */
  std::string buildBumpOutputCoords(bool bumpSampler) {
    return std::string("") + bumpSampler ?
        ("  " + FLOAT3 + " tangent" +
            semanticSuffix(
                "TEXCOORD" + interpolant_++) + ";\n" +
         "  " + FLOAT3 + " binormal" +
            semanticSuffix("TEXCOORD" +
                interpolant_++) + ";\n" +
         "  " + FLOAT2 + " bumpUV" +
            semanticSuffix(
                "TEXCOORD" + interpolant_++) + ";\n") : "";
  };


  /**
   * Builds vertex and fragment shader string for a 2-color checker effect.
   * @return {string} The effect code for the shader, ready to be parsed.
   */
  std::string buildCheckerShaderString() {
    std::string varyingDecls(BEGIN_OUT_STRUCT) +
      VARYING + FLOAT4 + " " +
      VERTEX_VARYING_PREFIX + "position" +
      semanticSuffix("POSITION") + ";\n" +
      VARYING + FLOAT2 + " " +
      VERTEX_VARYING_PREFIX + "texCoord" +
      semanticSuffix("TEXCOORD0") + ";\n" +
      VARYING + FLOAT3 + " " +
      VERTEX_VARYING_PREFIX + "normal" +
      semanticSuffix("TEXCOORD1") + ";\n" +
      VARYING + FLOAT3 + " " +
      VERTEX_VARYING_PREFIX + "worldPosition" +
      semanticSuffix("TEXCOORD2") + ";\n" +
      END_STRUCT;

    return std::string("uniform ") + MATRIX4 + " worldViewProjection" +
    semanticSuffix("WORLDVIEWPROJECTION") + ";\n" +
      "uniform " + MATRIX4 + " worldInverseTranspose" +
      semanticSuffix("WORLDINVERSETRANSPOSE") + ";\n" +
      "uniform " + MATRIX4 + " world" +
      semanticSuffix("WORLD") + ";\n" +
      "\n" +
      BEGIN_IN_STRUCT +
      ATTRIBUTE + FLOAT4 + " position" +
      semanticSuffix("POSITION") + ";\n" +
      ATTRIBUTE + FLOAT3 + " normal" +
      semanticSuffix("NORMAL") + ";\n" +
      ATTRIBUTE + FLOAT2 + " texCoord0" +
      semanticSuffix("TEXCOORD0") + ";\n" +
      END_STRUCT +
      "\n" +
      varyingDecls +
      "\n" +
      beginVertexShaderMain() +
      "  " + VERTEX_VARYING_PREFIX + "position = " +
      mul(ATTRIBUTE_PREFIX + "position",
          "worldViewProjection") + ";\n" +
      "  " + VERTEX_VARYING_PREFIX + "normal = " +
      mul(FLOAT4 + "(" +
      ATTRIBUTE_PREFIX + "normal, 0.0)",
          "worldInverseTranspose") + ".xyz;\n" +
      "  " + VERTEX_VARYING_PREFIX + "worldPosition = " +
          mul(ATTRIBUTE_PREFIX + "position", "world") +
      ".xyz;\n" +
      "  " + VERTEX_VARYING_PREFIX + "texCoord = " +
      ATTRIBUTE_PREFIX + "texCoord0;\n" +
      endVertexShaderMain() +
      "\n" +
      pixelShaderHeader() +
      "uniform " + FLOAT4 + " color1;\n" +
      "uniform " + FLOAT4 + " color2;\n" +
      "uniform float checkSize;\n" +
      "uniform " + FLOAT3 + " lightWorldPos;\n" +
      "uniform " + FLOAT3 + " lightColor;\n" +
      "\n" +
      repeatVaryingDecls(varyingDecls) +
      FLOAT4 + " checker(" + FLOAT2 + " uv) {\n" +
      "  float fmodResult = " + MOD + "(" +
      "    floor(checkSize * uv.x) + \n" +
      "    floor(checkSize * uv.y), 2.0);\n" +
      "  return (fmodResult < 1.0) ? color1 : color2;\n" +
      "}\n\n" +
      beginPixelShaderMain() +
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
      " outColor = directionalIntensity * check;\n" +
      endPixelShaderMain(
          FLOAT4 + "(outColor.rgb, check.a)") +
      "\n" + entryPoints() +
      matrixLoadOrder();
  };



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
  o3djs.effect.COLLADA_LIGHTING_TYPE_PARAM_NAME = "collada.lightingType";

  /**
   * The collada standard lighting types.
   * @type {!Object}
   */
  o3djs.effect.COLLADA_LIGHTING_TYPES = {phong: 1,
                                         lambert: 1,
                                         blinn: 1,
                                         constant: 1};

  /**
   * The FCollada standard materials sampler parameter name prefixes.
   * @type {!Array.<string>}
   */
  o3djs.effect.COLLADA_SAMPLER_PARAMETER_PREFIXES = ["emissive",
                                                     "ambient",
                                                     "diffuse",
                                                     "specular",
                                                     "bump"];

  /**
   * Check if lighting type is a collada lighting type.
   * @param {string} lightingType Lighting type to check.
   * @return {boolean} true if it"s a collada lighting type.
   */
  o3djs.effect.isColladaLightingType = function(lightingType) {
    return o3djs.effect.COLLADA_LIGHTING_TYPES[lightingType.toLowerCase()] == 1;
  };

  /**
   * Returns the collada lighting type of a collada standard material.
   * @param {!o3d.Material} material Material to get lighting type from.
   * @return {string} The lighting type or "" if it"s not a collada standard
   *     material.
   */
  o3djs.effect.getColladaLightingType = function(material) {
    var lightingTypeParam = material.getParam(
        o3djs.effect.COLLADA_LIGHTING_TYPE_PARAM_NAME);
    if (lightingTypeParam) {
      var lightingType = lightingTypeParam.value.toLowerCase();
      if (o3djs.effect.isColladaLightingType(lightingType)) {
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
  o3djs.effect.getNumTexCoordStreamsNeeded = function(material) {
    var p = o3djs.effect;
    var lightingType = p.getColladaLightingType(material);
    if (!p.isColladaLightingType(lightingType)) {
      throw "not a collada standard material";
    }
    var colladaSamplers = p.COLLADA_SAMPLER_PARAMETER_PREFIXES;
    var numTexCoordStreamsNeeded = 0
    for (var cc = 0; cc < colladaSamplers.length; ++cc) {
      var samplerPrefix = colladaSamplers[cc];
      var samplerParam = material.getParam(samplerPrefix + "Sampler");
      if (samplerParam) {
        ++numTexCoordStreamsNeeded;
      }
    }
    return numTexCoordStreamsNeeded;
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
  o3djs.effect.buildStandardShaderString = function(material,
                                                    effectType) {
    var p = o3djs.effect;
    var bumpSampler = material.getParam("bumpSampler");
    var bumpUVInterpolant;

    /**
     * Extracts the texture type from a texture param.
     * @param {!o3d.ParamTexture} textureParam The texture parameter to
     *     inspect.
     * @return {string} The texture type (1D, 2D, 3D or CUBE).
     */
    var getTextureType = function(textureParam) {
      var texture = textureParam.value;
      if (!texture) return "2D";  // No texture value, have to make a guess.
      switch (texture.className) {
        case "o3d.Texture1D" : return "1D";
        case "o3d.Texture2D" : return "2D";
        case "o3d.Texture3D" : return "3D";
        case "o3d.TextureCUBE" : return "CUBE";
        default : return "2D";
      }
    }

    /**
     * Extracts the sampler type from a sampler param.  It does it by inspecting
     * the texture associated with the sampler.
     * @param {!o3d.ParamTexture} samplerParam The texture parameter to
     *     inspect.
     * @return {string} The texture type (1D, 2D, 3D or CUBE).
     */
    var getSamplerType = function(samplerParam) {
      var sampler = samplerParam.value;
      if (!sampler) return "2D";
      var textureParam = sampler.getParam("Texture");
      if (textureParam)
        return getTextureType(textureParam);
      else
        return "2D";
    };

    /**
     * Builds uniform variables common to all standard lighting types.
     * @return {string} The effect code for the common shader uniforms.
     */
    var buildCommonVertexUniforms = function() {
      return "uniform " + p.MATRIX4 + " worldViewProjection" +
          p.semanticSuffix("WORLDVIEWPROJECTION") + ";\n" +
          "uniform " + p.FLOAT3 + " lightWorldPos;\n";
    };

    /**
     * Builds uniform variables common to all standard lighting types.
     * @return {string} The effect code for the common shader uniforms.
     */
    var buildCommonPixelUniforms = function() {
      return "uniform " + p.FLOAT4 + " lightColor;\n";
    };

    /**
     * Builds uniform variables common to lambert, phong and blinn lighting types.
     * @return {string} The effect code for the common shader uniforms.
     */
    var buildLightingUniforms = function() {
      return "uniform " + p.MATRIX4 + " world" +
          p.semanticSuffix("WORLD") + ";\n" +
          "uniform " + p.MATRIX4 +
          " viewInverse" + p.semanticSuffix("VIEWINVERSE") + ";\n" +
          "uniform " + p.MATRIX4 + " worldInverseTranspose" +
          p.semanticSuffix("WORLDINVERSETRANSPOSE") + ";\n";
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
    var buildColorParam = function(material, descriptions, name,
                                   opt_addColorParam) {
      if (opt_addColorParam === undefined) {
        opt_addColorParam = true;
      }
      var samplerParam = material.getParam(name + "Sampler");
      if (samplerParam) {
        var type = getSamplerType(samplerParam);
        descriptions.push(name + type + "Texture");
        return "uniform sampler" + type + " " + name + "Sampler;\n"
      } else if (opt_addColorParam) {
        descriptions.push(name + "Color");
        return "uniform " + p.FLOAT4 + " " + name + ";\n";
      } else {
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
    var getColorParam = function(material, name) {
      var samplerParam = material.getParam(name + "Sampler");
      if (samplerParam) {
        var type = getSamplerType(samplerParam);
        return "  " + p.FLOAT4 + " " + name + " = " + p.TEXTURE + type +
               "(" + name + "Sampler, " +
               p.PIXEL_VARYING_PREFIX + name + "UV);\n"
      } else {
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
    var buildConstantShaderString = function(material, descriptions) {
      descriptions.push("constant");
      return buildCommonVertexUniforms() +
             buildVertexDecls(material, false, false) +
             p.beginVertexShaderMain() +
             positionVertexShaderCode() +
             p.buildUVPassthroughs(material) +
             p.endVertexShaderMain() +
             p.pixelShaderHeader(material, false, false, bumpSampler) +
             buildCommonPixelUniforms() +
             p.repeatVaryingDecls() +
             buildColorParam(material, descriptions, "emissive") +
             p.beginPixelShaderMain() +
             getColorParam(material, "emissive") +
             p.endPixelShaderMain("emissive") +
             p.entryPoints() +
             p.matrixLoadOrder();
    };

    /**
     * Builds vertex and fragment shader string for the Lambert lighting type.
     * @param {!o3d.Material} material The material for which to build
     *     shaders.
     * @param {!Array.<string>} descriptions Array to add descriptions too.
     * @return {string} The effect code for the shader, ready to be parsed.
     */
    var buildLambertShaderString = function(material, descriptions) {
      descriptions.push("lambert");
      return buildCommonVertexUniforms() +
             buildLightingUniforms() +
             buildVertexDecls(material, true, false) +
             p.beginVertexShaderMain() +
             p.buildUVPassthroughs(material) +
             positionVertexShaderCode() +
             normalVertexShaderCode() +
             surfaceToLightVertexShaderCode() +
             bumpVertexShaderCode() +
             p.endVertexShaderMain() +
             p.pixelShaderHeader(material, true, false) +
             buildCommonPixelUniforms() +
             p.repeatVaryingDecls() +
             buildColorParam(material, descriptions, "emissive") +
             buildColorParam(material, descriptions, "ambient") +
             buildColorParam(material, descriptions, "diffuse") +
             buildColorParam(material, descriptions, "bump", false) +
             p.utilityFunctions() +
             p.beginPixelShaderMain() +
             getColorParam(material, "emissive") +
             getColorParam(material, "ambient") +
             getColorParam(material, "diffuse") +
             getNormalShaderCode() +
             "  " + p.FLOAT3 + " surfaceToLight = normalize(" +
             p.PIXEL_VARYING_PREFIX + "surfaceToLight);\n" +
             "  " + p.FLOAT4 +
             " litR = lit(dot(normal, surfaceToLight), 0.0, 0.0);\n" +
             p.endPixelShaderMain(p.FLOAT4 +
             "((emissive +\n" +
             "      lightColor *" +
             " (ambient * diffuse + diffuse * litR.y)).rgb,\n" +
             "          diffuse.a)") +
             p.entryPoints() +
             p.matrixLoadOrder();
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
    var buildBlinnShaderString = function(material, descriptions) {
      descriptions.push("phong");
      return buildCommonVertexUniforms() +
          buildLightingUniforms() +
          buildVertexDecls(material, true, true) +
          p.beginVertexShaderMain() +
          p.buildUVPassthroughs(material) +
          positionVertexShaderCode() +
          normalVertexShaderCode() +
          surfaceToLightVertexShaderCode() +
          surfaceToViewVertexShaderCode() +
          bumpVertexShaderCode() +
          p.endVertexShaderMain() +
          p.pixelShaderHeader(material, true, true) +
          buildCommonPixelUniforms() +
          p.repeatVaryingDecls() +
          buildColorParam(material, descriptions, "emissive") +
          buildColorParam(material, descriptions, "ambient") +
          buildColorParam(material, descriptions, "diffuse") +
          buildColorParam(material, descriptions, "specular") +
          buildColorParam(material, descriptions, "bump", false) +
          "uniform float shininess;\n" +
          "uniform float specularFactor;\n" +
          p.utilityFunctions() +
          p.beginPixelShaderMain() +
          getColorParam(material, "emissive") +
          getColorParam(material, "ambient") +
          getColorParam(material, "diffuse") +
          getColorParam(material, "specular") +
          getNormalShaderCode() +
          "  " + p.FLOAT3 + " surfaceToLight = normalize(" +
          p.PIXEL_VARYING_PREFIX + "surfaceToLight);\n" +
          "  " + p.FLOAT3 + " surfaceToView = normalize(" +
          p.PIXEL_VARYING_PREFIX + "surfaceToView);\n" +
          "  " + p.FLOAT3 +
          " halfVector = normalize(surfaceToLight + " +
          p.PIXEL_VARYING_PREFIX + "surfaceToView);\n" +
          "  " + p.FLOAT4 +
          " litR = lit(dot(normal, surfaceToLight), \n" +
          "                    dot(normal, halfVector), shininess);\n" +
          p.endPixelShaderMain( p.FLOAT4 +
          "((emissive +\n" +
          "  lightColor *" +
          " (ambient * diffuse + diffuse * litR.y +\n" +
          "                        + specular * litR.z *" +
          " specularFactor)).rgb,\n" +
          "      diffuse.a)") +
          p.entryPoints() +
          p.matrixLoadOrder();
    };

    /**
     * Builds vertex and fragment shader string for the Phong lighting type.
     * @param {!o3d.Material} material The material for which to build
     *     shaders.
     * @param {!Array.<string>} descriptions Array to add descriptions too.
     * @return {string} The effect code for the shader, ready to be parsed.
     */
    var buildPhongShaderString = function(material, descriptions) {
      descriptions.push("phong");
      return buildCommonVertexUniforms() +
          buildLightingUniforms() +
          buildVertexDecls(material, true, true) +
          p.beginVertexShaderMain() +
          p.buildUVPassthroughs(material) +
          positionVertexShaderCode() +
          normalVertexShaderCode() +
          surfaceToLightVertexShaderCode() +
          surfaceToViewVertexShaderCode() +
          bumpVertexShaderCode() +
          p.endVertexShaderMain() +
          p.pixelShaderHeader(material, true, true) +
          buildCommonPixelUniforms() +
          p.repeatVaryingDecls() +
          buildColorParam(material, descriptions, "emissive") +
          buildColorParam(material, descriptions, "ambient") +
          buildColorParam(material, descriptions, "diffuse") +
          buildColorParam(material, descriptions, "specular") +
          buildColorParam(material, descriptions, "bump", false) +
          "uniform float shininess;\n" +
          "uniform float specularFactor;\n" +
          p.utilityFunctions() +
          p.beginPixelShaderMain() +
          getColorParam(material, "emissive") +
          getColorParam(material, "ambient") +
          getColorParam(material, "diffuse") +
          getColorParam(material, "specular") +
          getNormalShaderCode() +
          "  " + p.FLOAT3 + " surfaceToLight = normalize(" +
          p.PIXEL_VARYING_PREFIX + "surfaceToLight);\n" +
          "  " + p.FLOAT3 + " surfaceToView = normalize(" +
          p.PIXEL_VARYING_PREFIX + "surfaceToView);\n" +
          "  " + p.FLOAT3 +
          " halfVector = normalize(surfaceToLight + surfaceToView);\n" +
          "  " + p.FLOAT4 +
          " litR = lit(dot(normal, surfaceToLight), \n" +
          "                    dot(normal, halfVector), shininess);\n" +
          p.endPixelShaderMain(p.FLOAT4 +
          "((emissive +\n" +
          "  lightColor * (ambient * diffuse + diffuse * litR.y +\n" +
          "                        + specular * litR.z *" +
          " specularFactor)).rgb,\n" +
          "      diffuse.a)") +
          p.entryPoints() +
          p.matrixLoadOrder();
    };

    /**
     * Builds the position code for the vertex shader.
     * @return {string} The code for the vertex shader.
     */
    var positionVertexShaderCode = function() {
      return "  " + p.VERTEX_VARYING_PREFIX + "position = " +
          p.mul(p.ATTRIBUTE_PREFIX +
          "position", "worldViewProjection") + ";\n";
    };

    /**
     * Builds the normal code for the vertex shader.
     * @return {string} The code for the vertex shader.
     */
    var normalVertexShaderCode = function() {
      return "  " + p.VERTEX_VARYING_PREFIX + "normal = " +
          p.mul(p.FLOAT4 + "(" +
          p.ATTRIBUTE_PREFIX +
          "normal, 0)", "worldInverseTranspose") + ".xyz;\n";
    };

    /**
     * Builds the surface to light code for the vertex shader.
     * @return {string} The code for the vertex shader.
     */
    var surfaceToLightVertexShaderCode = function() {
      return "  " + p.VERTEX_VARYING_PREFIX +
          "surfaceToLight = lightWorldPos - \n" +
             "                          " +
             p.mul(p.ATTRIBUTE_PREFIX + "position",
                "world") + ".xyz;\n";
    };

    /**
     * Builds the surface to view code for the vertex shader.
     * @return {string} The code for the vertex shader.
     */
    var surfaceToViewVertexShaderCode = function() {
      return "  " + p.VERTEX_VARYING_PREFIX +
          "surfaceToView = (viewInverse[3] - " +
           p.mul(p.ATTRIBUTE_PREFIX + "position", "world") + ").xyz;\n";
    };

    /**
     * Builds the normal map part of the vertex shader.
     * @param {boolean} opt_bumpSampler Whether there is a bump
     *     sampler. Default = false.
     * @return {string} The code for normal mapping in the vertex shader.
     */
    var bumpVertexShaderCode = function(opt_bumpSampler) {
      return bumpSampler ?
          ("  " + p.VERTEX_VARYING_PREFIX + "binormal = " +
           p.mul(p.FLOAT4 + "(" +
           p.ATTRIBUTE_PREFIX + "binormal, 0)",
               "worldInverseTranspose") + ".xyz;\n" +
           "  " + p.VERTEX_VARYING_PREFIX + "tangent = " +
           p.mul(p.FLOAT4 +
           "(" + p.ATTRIBUTE_PREFIX + "tangent, 0)",
               "worldInverseTranspose") + ".xyz;\n") : "";
    };

    /**
     * Builds the normal calculation of the pixel shader.
     * @return {string} The code for normal computation in the pixel shader.
     */
    var getNormalShaderCode = function() {
      return bumpSampler ?
          (p.MATRIX3 + " tangentToWorld = " + p.MATRIX3 +
              "(" + p.ATTRIBUTE_PREFIX + "tangent,\n" +
           "                                   " +
           p.ATTRIBUTE_PREFIX + "binormal,\n" +
           "                                   " +
           p.ATTRIBUTE_PREFIX + "normal);\n" +
           p.FLOAT3 + " tangentNormal = tex2D(bumpSampler, " +
           p.ATTRIBUTE_PREFIX + "bumpUV.xy).xyz -\n" +
           "                       " + p.FLOAT3 +
           "(0.5, 0.5, 0.5);\n" + p.FLOAT3 + " normal = " +
           p.mul("tangentNormal", "tangentToWorld") + ";\n" +
           "normal = normalize(" + p.PIXEL_VARYING_PREFIX +
           "normal);\n") : "  " + p.FLOAT3 + " normal = normalize(" +
           p.PIXEL_VARYING_PREFIX + "normal);\n";
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
    var buildVertexDecls = function(material, diffuse, specular) {
      return p.buildAttributeDecls(
          material, diffuse, specular, bumpSampler) +
          p.buildVaryingDecls(
              material, diffuse, specular, bumpSampler);
    };


    // Create a shader string of the appropriate type, based on the
    // effectType.
    var str;
    var descriptions = [];
    if (effectType == "phong") {
      str = buildPhongShaderString(material, descriptions);
    } else if (effectType == "lambert") {
      str = buildLambertShaderString(material, descriptions);
    } else if (effectType == "blinn") {
      str = buildBlinnShaderString(material, descriptions);
    } else if (effectType == "constant") {
      str = buildConstantShaderString(material, descriptions);
    } else {
      throw ("unknown effect type "" + effectType + """);
    }

    return {description: descriptions.join("_"), shader: str};
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
  o3djs.effect.getStandardShader = function(pack,
                                            material,
                                            effectType) {
    var record = o3djs.effect.buildStandardShaderString(material,
                                                        effectType);
    var effects = pack.getObjectsByClassName("o3d.Effect");
    for (var ii = 0; ii < effects.length; ++ii) {
      if (effects[ii].name == record.description &&
          effects[ii].source == record.shader) {
        return effects[ii];
      }
    }
    var effect = pack.createObject("Effect");
    if (effect) {
      effect.name = record.description;
      if (effect.loadFromFXString(record.shader)) {
        return effect;
      }
      pack.removeObject(effect);
    }
    return null;
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
  o3djs.effect.attachStandardShader = function(pack,
                                               material,
                                               lightPos,
                                               effectType) {
    var effect = o3djs.effect.getStandardShader(pack,
                                                material,
                                                effectType);
    if (effect) {
      material.effect = effect;
      effect.createUniformParameters(material);

      // Set a couple of the default parameters in the hopes that this will
      // help the user get something on the screen. We check to make sure they
      // are not connected to something otherwise we"ll get an error.
      var param = material.getParam("lightWorldPos");
      if (!param.inputConnection) {
        param.value = lightPos;
      }
      var param = material.getParam("lightColor");
      if (!param.inputConnection) {
        param.value = [1, 1, 1, 1];
      }
      return true;
    } else {
      return false;
    }
  };

  /**
   * Creates the uniform parameters needed for an Effect on the given ParamObject.
   * @param {!o3d.Pack} pack Pack to create extra objects in like Samplers and
   *     ParamArrays.
   * @param {!o3d.Effect} effect Effect.
   * @param {!o3d.ParamObject} paramObject ParamObject on which to create Params.
   */
  o3djs.effect.createUniformParameters = function(pack, effect, paramObject) {
    effect.createUniformParameters(paramObject);
    var infos = effect.getParameterInfo();
    for (var ii = 0; ii < infos.length; ++ii) {
      var info = infos[ii];
      if (info.sasClassName.length == 0) {
        if (info.numElements > 0) {
          var paramArray = pack.createObject("ParamArray");
          var param = paramObject.getParam(info.name);
          param.value = paramArray;
          paramArray.resize(info.numElements, info.className);
          if (info.className == "o3d.ParamSampler") {
            for (var jj = 0; jj < info.numElements; ++jj) {
              var sampler = pack.createObject("Sampler");
              paramArray.getParam(jj).value = sampler;
            }
          }
        } else if (info.className == "o3d.ParamSampler") {
          var sampler = pack.createObject("Sampler");
          var param = paramObject.getParam(info.name);
          param.value = sampler;
        }
      }
    }
  };

  /**
   * Creates an effect that draws a 2 color procedural checker pattern.
   * @param {!o3d.Pack} pack The pack to create the effect in. If the pack
   *     already has an effect with the same name that effect will be returned.
   * @return {!o3d.Effect} The effect.
   */
  o3djs.effect.createCheckerEffect = function(pack) {
    var effects = pack.getObjects(o3djs.effect.TWO_COLOR_CHECKER_EFFECT_NAME,
                                  "o3d.Effect");
    if (effects.length > 0) {
      return effects[0];
    }

    var effect = pack.createObject("Effect");
    effect.loadFromFXString(o3djs.effect.TWO_COLOR_CHECKER_FXSTRING);
    effect.name = o3djs.effect.TWO_COLOR_CHECKER_EFFECT_NAME;
    return effect;
  };


  // For compatability with o3d code, the default language is o3d shading
  // language.
  o3djs.effect.setLanguage("o3d");
};

#endif

}  // namespace o3d_utils

