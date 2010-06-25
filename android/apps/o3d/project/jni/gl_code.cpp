/*
 * Copyright (C) 2009 The Android Open Source Project
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

// OpenGL ES 2.0 code

#include <jni.h>
#include <android/log.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "base/logging.h"
#include "core/cross/service_locator.h"
#include "core/cross/evaluation_counter.h"
#include "core/cross/client.h"
#include "core/cross/client_info.h"
#include "core/cross/class_manager.h"
#include "core/cross/display_window.h"
#include "core/cross/features.h"
#include "core/cross/object_manager.h"
#include "core/cross/primitive.h"
#include "core/cross/profiler.h"
#include "core/cross/renderer.h"
#include "core/cross/renderer_platform.h"
#include "core/cross/shape.h"
#include "core/cross/transform.h"
#include "core/cross/types.h"
#include "import/cross/collada.h"

#include "shader_builder.h"
#include "render_graph.h"
#include "camera.h"

#define  LOG_TAG    "libo3djni"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

using o3d_utils::ShaderBuilder;

class DisplayWindowAndroid : public o3d::DisplayWindow {
 public:
  ~DisplayWindowAndroid() { }
};

class O3DManager {
 public:
  o3d::Client* client() const {
    return client_.get();
  }

  bool Initialize(int width, int height);
  bool Render();

  // Prepares all the materials in a pack assuming they are materials as
  // imported from the collada loader.
  void PrepareMaterials(
      o3d::Pack* pack, o3d_utils::ViewInfo* view_info, o3d::Pack* effect_pack);

  // Prepares a material assuming it was imported from the collada loader.
  void PrepareMaterial(
      o3d::Pack* pack, o3d_utils::ViewInfo* view_info, o3d::Material* material,
      std::string opt_effect_type);

  void AttachStandardEffect(o3d::Pack* pack,
                            o3d::Material* material,
                            o3d_utils::ViewInfo* viewInfo,
                            const std::string& effectType);

  void SetBoundingBoxAndZSortPoint(o3d::Element* element);
  void AddMissingTexCoordStreams(o3d::Element* element);
  void AddMissingTexCoordStreams(o3d::Shape* shape);
  void SetBoundingBoxesAndZSortPoints(o3d::Shape* shape);
  void PrepareShape(o3d::Pack* pack, o3d::Shape* shape);
  void PrepareShapes(o3d::Pack* pack);

 private:
  bool hasNonOneAlpha(
      o3d::Material* material, const std::string& name,
      bool* nonOneAlpha);

  DisplayWindowAndroid display_window_;
  o3d::ServiceLocator service_locator_;
  o3d::Renderer* renderer_;
  scoped_ptr<o3d::EvaluationCounter> evaluation_counter_;
  scoped_ptr<o3d::ClassManager> class_manager_;
  scoped_ptr<o3d::ClientInfoManager> client_info_manager_;
  scoped_ptr<o3d::ObjectManager> object_manager_;
  scoped_ptr<o3d::Profiler> profiler_;
  scoped_ptr<o3d::Features> features_;
  scoped_ptr<o3d::Client> client_;

  o3d::Transform* root_;
  o3d::Pack* pack_;
  o3d_utils::ViewInfo* main_view_;
  o3d::Pack* scene_pack_;
};

/**
 * Checks a material's params by name to see if it possibly has non 1.0 alpha.
 * Given a name, checks for a ParamTexture called 'nameTexture' and if that
 * fails, checks for a ParamFloat4 'name'.
 * @private
 * @param {!o3d.Material} material Materal to check.
 * @param {string} name name of color params to check.
 * @return {{found: boolean, nonOneAlpha: boolean}} found is true if one of
 *     the params was found, nonOneAlpha is true if that param had non 1.0
 *     alpha.
 */
bool O3DManager::hasNonOneAlpha(
    o3d::Material* material, const std::string& name,
    bool* nonOneAlpha) {
  bool found = false;
  *nonOneAlpha = false;
  o3d::Texture* texture = NULL;
  o3d::ParamSampler* samplerParam =
      material->GetParam<o3d::ParamSampler>(name + "Sampler");
  if (samplerParam) {
    found = true;
    o3d::Sampler* sampler = samplerParam->value();
    if (sampler) {
      texture = sampler->texture();
    }
  } else {
    o3d::ParamTexture* textureParam =
        material->GetParam<o3d::ParamTexture>(name + "Texture");
    if (textureParam) {
      found = true;
      texture = textureParam->value();
    }
  }

  if (texture && !texture->alpha_is_one()) {
    *nonOneAlpha = true;
  }

  if (!found) {
    o3d::ParamFloat4* colorParam = material->GetParam<o3d::ParamFloat4>(name);
    if (colorParam) {
      found = true;
      // TODO: this check does not work. We need to check for the
      // <transparency> and <transparent> elements or something.
      // if (colorParam.value[3] < 1) {
      //   nonOneAlpha = true;
      // }
    }
  }
  return found;
};

/**
 * Prepares a material by setting their drawList and possibly creating
 * an standard effect if one does not already exist.
 *
 * This function is very specific to our sample importer. It expects that if
 * no Effect exists on a material that certain extra Params have been created
 * on the Material to give us instructions on what to Effects to create.
 *
 * @param {!o3d.Pack} pack Pack to manage created objects.
 * @param {!o3djs.rendergraph.ViewInfo} viewInfo as returned from
 *     o3djs.rendergraph.createView.
 * @param {!o3d.Material} material to prepare.
 * @param {string} opt_effectType type of effect to create ('phong',
 *     'lambert', 'constant').
 *
 * @see o3djs.material.attachStandardEffect
 */
void O3DManager::PrepareMaterial(
    o3d::Pack* pack, o3d_utils::ViewInfo* view_info, o3d::Material* material,
    std::string opt_effect_type) {
  // Assume we want the performance list
  o3d::DrawList* draw_list =
      view_info->performance_draw_pass_info()->draw_list();
  // First check if we have a tag telling us that it is or is not
  // transparent
  if (!material->draw_list()) {
    o3d::ParamBoolean* param = material->GetParam<o3d::ParamBoolean>(
        "collada.transparent");
    if (param) {
      material->set_draw_list(param->value() ?
          view_info->z_ordered_draw_pass_info()->draw_list() :
          view_info->performance_draw_pass_info()->draw_list());
    }
  }

  // If the material has no effect, try to build shaders for it.
  if (!material->effect()) {
    // If the user didn't pass an effect type in see if one was stored there
    // by our importer.
    if (opt_effect_type.empty()) {
      // Retrieve the lightingType parameter from the material, if any.
      std::string lightingType =
          ShaderBuilder::getColladaLightingType(material);
      if (!lightingType.empty()) {
        opt_effect_type = lightingType;
      }
    }
    if (!opt_effect_type.empty()) {
      AttachStandardEffect(pack,
                           material,
                           view_info,
                           opt_effect_type);
      // For collada common profile stuff guess what drawList to use. Note: We
      // can only do this for collada common profile stuff because we supply
      // the shaders and therefore now the inputs and how they are used.
      // For other shaders you've got to do this stuff yourself. On top of
      // that this is a total guess. Just because a texture has no alpha
      // it does not follow that you don't want it in the zOrderedDrawList.
      // That is application specific. Here we are just making a guess and
      // hoping that it covers most cases.
      if (!material->draw_list()) {
        // Check the common profile params.
        bool nonOneAlpha = false;
        bool found = hasNonOneAlpha(material, "diffuse", &nonOneAlpha);
        if (!found) {
          found = hasNonOneAlpha(material, "emissive", &nonOneAlpha);
        }
        if (nonOneAlpha) {
          draw_list = view_info->z_ordered_draw_pass_info()->draw_list();
        }
      }
    }
  }

  if (!material->draw_list()) {
    material->set_draw_list(draw_list);
  }
}

void O3DManager::AttachStandardEffect(o3d::Pack* pack,
                                      o3d::Material* material,
                                      o3d_utils::ViewInfo* viewInfo,
                                      const std::string& effectType) {
  if (!material->effect()) {
    scoped_ptr<ShaderBuilder> shader_builder(ShaderBuilder::Create());
    o3d::Vector3 light_pos =
        Vectormath::Aos::inverse(
            viewInfo->draw_context()->view()).getTranslation();
    if (!shader_builder->attachStandardShader(
        pack, material, light_pos, effectType)) {
       NOTREACHED() << "Could not attach a standard effect";
    }
  }
}

/**
 * Prepares all the materials in the given pack by setting their drawList and
 * if they don't have an Effect, creating one for them.
 *
 * This function is very specific to our sample importer. It expects that if
 * no Effect exists on a material that certain extra Params have been created
 * on the Material to give us instructions on what to Effects to create.
 *
 * @param {!o3d.Pack} pack Pack to prepare.
 * @param {!o3djs.rendergraph.ViewInfo} viewInfo as returned from
 *     o3djs.rendergraph.createView.
 * @param {!o3d.Pack} opt_effectPack Pack to create effects in. If this
 *     is not specifed the pack to prepare above will be used.
 *
 * @see o3djs.material.prepareMaterial
 */
void O3DManager::PrepareMaterials(
  o3d::Pack* pack, o3d_utils::ViewInfo* view_info, o3d::Pack* effect_pack) {
  std::vector<o3d::Material*> materials = pack->GetByClass<o3d::Material>();
  for (size_t ii = 0; ii < materials.size(); ++ii) {
    PrepareMaterial(effect_pack ? effect_pack : pack,
                    view_info,
                    materials[ii], "");
  }
}

/**
 * Sets the bounding box and z sort point of an element.
 * @param {!o3d.Element} element Element to set bounding box and z sort point
 *     on.
 */
void O3DManager::SetBoundingBoxAndZSortPoint(o3d::Element* element) {
  o3d::BoundingBox boundingBox;
  element->GetBoundingBox(0, &boundingBox);
  o3d::Point3 minExtent = boundingBox.min_extent();
  o3d::Point3 maxExtent = boundingBox.max_extent();
  element->set_bounding_box(boundingBox);
  element->set_cull(true);
  element->set_z_sort_point(o3d::Float3(
      (minExtent.getX() + maxExtent.getX()) / 2.0f,
      (minExtent.getY() + maxExtent.getY()) / 2.0f,
      (minExtent.getZ() + maxExtent.getZ()) / 2.0f));
};

/**
 * Adds missing texture coordinate streams to a primitive.
 *
 * This is very application specific but if it's a primitive
 * and if it uses a collada material the material builder
 * assumes 1 TEXCOORD stream per texture. In other words if you have
 * both a specular texture AND a diffuse texture the builder assumes
 * you have 2 TEXCOORD streams. This assumption is often false.
 *
 * To work around this we check how many streams the material
 * expects and if there are not enough UVs streams we duplicate the
 * last TEXCOORD stream until there are, making a BIG assumption that
 * that will work.
 *
 * The problem is maybe you have 4 textures and each of them share
 * texture coordinates. There is information in the collada file about
 * what stream to connect each texture to.
 *
 * @param {!o3d.Element} element Element to add streams to.
 */
void O3DManager::AddMissingTexCoordStreams(o3d::Element* element) {
  // TODO: We should store that info. The conditioner should either
  // make streams that way or pass on the info so we can do it here.
  if (element->IsA(o3d::Primitive::GetApparentClass())) {
    o3d::Primitive* primitive = static_cast<o3d::Primitive*>(element);
    o3d::Material* material = element->material();
    o3d::StreamBank* streamBank = primitive->stream_bank();
    std::string lightingType = ShaderBuilder::getColladaLightingType(material);
    if (!lightingType.empty()) {
      int numTexCoordStreamsNeeded =
          ShaderBuilder::getNumTexCoordStreamsNeeded(material);
      // Count the number of TEXCOORD streams the streamBank has.
      const o3d::StreamParamVector& streams =
            streamBank->vertex_stream_params();
      const o3d::Stream* lastTexCoordStream = NULL;
      int numTexCoordStreams = 0;
      for (int ii = 0; ii < static_cast<int>(streams.size()); ++ii) {
        const o3d::Stream* stream = &streams[ii]->stream();
        if (stream->semantic() == o3d::Stream::TEXCOORD) {
          lastTexCoordStream = stream;
          ++numTexCoordStreams;
        }
      }
      // Add any missing TEXCOORD streams. It might be more efficient for
      // the GPU to create an effect that doesn't need the extra streams
      // but this is a more generic solution because it means we can reuse
      // the same effect.
      for (int ii = numTexCoordStreams;
           ii < numTexCoordStreamsNeeded;
           ++ii) {
        streamBank->SetVertexStream(
            lastTexCoordStream->semantic(),
            lastTexCoordStream->semantic_index() + ii - numTexCoordStreams + 1,
            &lastTexCoordStream->field(),
            lastTexCoordStream->start_index());
      }
    }
  }
};

/**
 * Adds missing tex coord streams to a shape's elements.
 * @param {!o3d.Shape} shape Shape to add missing streams to.
 * @see o3djs.element.addMissingTexCoordStreams
 */
void O3DManager::AddMissingTexCoordStreams(o3d::Shape* shape) {
  const o3d::ElementRefArray& elements = shape->GetElementRefs();
  for (size_t ee = 0; ee < elements.size(); ++ee) {
    AddMissingTexCoordStreams(elements[ee]);
  }
};

/**
 * Sets the bounding box and z sort points of a shape's elements.
 * @param {!o3d.Shape} shape Shape to set info on.
 */
void O3DManager::SetBoundingBoxesAndZSortPoints(o3d::Shape* shape) {
  const o3d::ElementRefArray& elements = shape->GetElementRefs();
  for (size_t ee = 0; ee < elements.size(); ++ee) {
    SetBoundingBoxAndZSortPoint(elements[ee]);
  }
};

/**
 * Prepares a shape by setting its boundingBox, zSortPoint and creating
 * DrawElements.
 * @param {!o3d.Pack} pack Pack to manage created objects.
 * @param {!o3d.Shape} shape Shape to prepare.
 */
void O3DManager::PrepareShape(o3d::Pack* pack, o3d::Shape* shape) {
  shape->CreateDrawElements(pack, NULL);
  SetBoundingBoxesAndZSortPoints(shape);
  AddMissingTexCoordStreams(shape);
};

/**
 * Prepares all the shapes in the given pack by setting their boundingBox,
 * zSortPoint and creating DrawElements.
 * @param {!o3d.Pack} pack Pack to manage created objects.
 */
void O3DManager::PrepareShapes(o3d::Pack* pack) {
  std::vector<o3d::Shape*> shapes = pack->GetByClass<o3d::Shape>();
  for (size_t ss = 0; ss < shapes.size(); ++ss) {
    PrepareShape(pack, shapes[ss]);
  }
};

void DumpRenderGraph(o3d::RenderNode* render_node, const std::string& indent) {
  if (render_node) {
    LOGI("%s%s\n", indent.c_str(), render_node->GetClass()->name());
    const o3d::RenderNodeRefArray& children = render_node->children();
    if (!children.empty()) {
      std::string inner = indent + "    ";
      for (size_t ii = 0; ii < children.size(); ++ii) {
        DumpRenderGraph(children[ii], inner);
      }
    }
  }
}

void DumpMatrix(const o3d::Matrix4& mat) {
  for (int ii = 0; ii < 4; ++ii) {
    LOGI("   %g, %g, %g, %g\n", mat[ii][0], mat[ii][1], mat[ii][2], mat[ii][3]);
  }
}

bool O3DManager::Initialize(int width, int height) {
  evaluation_counter_.reset(new o3d::EvaluationCounter(&service_locator_));
  class_manager_.reset(new o3d::ClassManager(&service_locator_));
  client_info_manager_.reset(new o3d::ClientInfoManager(&service_locator_));
  object_manager_.reset(new o3d::ObjectManager(&service_locator_));
  profiler_.reset(new o3d::Profiler(&service_locator_));
  features_.reset(new o3d::Features(&service_locator_));

  // create a renderer device based on the current platform
  renderer_ = o3d::Renderer::CreateDefaultRenderer(&service_locator_);

  LOGI("-----------------------------HERE1\n");

  if (renderer_->Init(display_window_, false) != o3d::Renderer::SUCCESS) {
  DLOG(ERROR) << "Window initialization failed!";
    return false;
  }

  LOGI("-----------------------------HERE2\n");

  client_.reset(new o3d::Client(&service_locator_));
  client_->Init();
  pack_ = client_->CreatePack();
  root_ = pack_->Create<o3d::Transform>();
  main_view_ = o3d_utils::ViewInfo::CreateBasicView(
      pack_, root_, client_->render_graph_root());

  LOGI("---Render Graph---(start)---\n");
  DumpRenderGraph(client_->render_graph_root(), "");
  LOGI("---Render Graph---(end)---\n");

  scene_pack_ = client_->CreatePack();
  o3d::Collada::Options options;
  o3d::Collada::Import(
      scene_pack_,
//      "/sdcard/collada/seven_shapes.zip",
      "/sdcard/collada/cube.zip",
      root_,
      NULL,
      options);

  o3d_utils::CameraInfo* camera_info =
      o3d_utils::Camera::getViewAndProjectionFromCameras(
          root_, width, height);

  main_view_->draw_context()->set_view(camera_info->view);
  main_view_->draw_context()->set_projection(camera_info->projection);

  PrepareMaterials(scene_pack_, main_view_, NULL);
  PrepareShapes(scene_pack_);

  LOGI("--view--\n");
  DumpMatrix(main_view_->draw_context()->view());
  LOGI("--projection--\n");
  DumpMatrix(main_view_->draw_context()->projection());

  return true;
}

bool O3DManager::Render() {
  //static int v = 0;
  //++v;
  //main_view_->clear_buffer()->set_clear_color(
  //    o3d::Float4(float(v % 100) / 100.0f, 0, 0, 1));
  client_->Tick();
  client_->RenderClient(true);
  //if (v < 4) {
  //  LOGI("-----------RENDER--------\n");
  //  LOGI("render_frame_count     : %d\n", renderer_->render_frame_count());
  //  LOGI("transforms_processed   : %d\n", renderer_->transforms_processed());
  //  LOGI("transforms_culled      : %d\n", renderer_->transforms_culled());
  //  LOGI("draw_elements_processed: %d\n", renderer_->draw_elements_processed());
  //  LOGI("draw_elements_culled   : %d\n", renderer_->draw_elements_culled());
  //  LOGI("draw_elements_rendered : %d\n", renderer_->draw_elements_rendered());
  //  LOGI("primitives_rendered    : %d\n", renderer_->primitives_rendered());
  //}
}

static O3DManager* g_mgr;

extern "C" {
    JNIEXPORT void JNICALL Java_com_android_o3djni_O3DJNILib_init(JNIEnv * env, jobject obj,  jint width, jint height);
    JNIEXPORT void JNICALL Java_com_android_o3djni_O3DJNILib_step(JNIEnv * env, jobject obj);
};

JNIEXPORT void JNICALL Java_com_android_o3djni_O3DJNILib_init(JNIEnv * env, jobject obj,  jint width, jint height)
{
    //setupGraphics(width, height);

    // The OpenGL ES Spec requires this to already be set but appearently
    // Android doesn't follow the spec?
    glViewport(0, 0, width, height);

    g_mgr = new O3DManager();
    g_mgr->Initialize(width, height);
}

JNIEXPORT void JNICALL Java_com_android_o3djni_O3DJNILib_step(JNIEnv * env, jobject obj)
{
    //renderFrame();
    g_mgr->Render();
}


