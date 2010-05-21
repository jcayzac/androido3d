# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# the purpose of this sample is to demonstrate how one can
# generate two distinct shared libraries and have them both
# uploaded in
#

LOCAL_PATH:= $(call my-dir)

#### STLPort
#
include $(CLEAR_VARS)

LOCAL_MODULE    := stdport
LOCAL_CFLAGS    := -I$(LOCAL_PATH)/third_party/stlport/stlport -I$(LOCAL_PATH)/third_party/stlport/src
LOCAL_SRC_FILES := $(addprefix third_party/stlport/src/,$(notdir $(wildcard $(LOCAL_PATH)/third_party/stlport/src/*.cpp $(LOCAL_PATH)/third_party/stlport/src/*.c))))

include $(BUILD_STATIC_LIBRARY)

#### Vectormath
#

#### zlib
#
include $(CLEAR_VARS)

LOCAL_MODULE    := zlib
LOCAL_SRC_FILES := $(addprefix third_party/zlib/, contrib/minizip/ioapi.c contrib/minizip/unzip.c contrib/minizip/zip.c adler32.c compress.c crc32.c deflate.c gzio.c infback.c inffast.c inflate.c inftrees.c trees.c uncompr.c zutil.c)

include $(BUILD_STATIC_LIBRARY)

#### libjpeg
#
include $(CLEAR_VARS)

LOCAL_MODULE    := libjpeg
LOCAL_SRC_FILES := $(addprefix third_party/libjpeg/, jcapimin.c jcapistd.c jccoefct.c jccolor.c jcdctmgr.c jchuff.c jcinit.c jcmainct.c jcmarker.c jcmaster.c jcomapi.c jcparam.c jcphuff.c jcprepct.c jcsample.c jdapimin.c jdapistd.c jdatadst.c jdatasrc.c jdcoefct.c jdcolor.c jddctmgr.c jdhuff.c jdinput.c jdmainct.c jdmarker.c jdmaster.c jdmerge.c jdphuff.c jdpostct.c jdsample.c jerror.c jfdctflt.c jfdctfst.c jfdctint.c jidctflt.c jidctfst.c jidctint.c jmemmgr.c jmemnobs.c jquant1.c jquant2.c jutils.c)

include $(BUILD_STATIC_LIBRARY)

#### fcollada
#
include $(CLEAR_VARS)

LOCAL_MODULE    := fcollada
LOCAL_CFLAGS    := -I$(LOCAL_PATH)/third_party/fcollada/files/LibXML/include -I$(LOCAL_PATH)/third_party/fcollada/files -I$(LOCAL_PATH)/third_party/stlport/stlport
LOCAL_SRC_FILES := $(addprefix third_party/fcollada/files/, DLLEntry.cpp FArchiveXML/FAXAnimationExport.cpp FArchiveXML/FAXAnimationImport.cpp FArchiveXML/FAXCameraExport.cpp FArchiveXML/FAXCameraImport.cpp FArchiveXML/FAXColladaParser.cpp FArchiveXML/FAXColladaWriter.cpp FArchiveXML/FAXControllerExport.cpp FArchiveXML/FAXControllerImport.cpp FArchiveXML/FAXEmitterExport.cpp FArchiveXML/FAXEmitterImport.cpp FArchiveXML/FAXEntityExport.cpp FArchiveXML/FAXEntityImport.cpp FArchiveXML/FAXForceFieldExport.cpp FArchiveXML/FAXForceFieldImport.cpp FArchiveXML/FAXGeometryExport.cpp FArchiveXML/FAXGeometryImport.cpp FArchiveXML/FAXImportLinking.cpp FArchiveXML/FAXInstanceExport.cpp FArchiveXML/FAXInstanceImport.cpp FArchiveXML/FAXLightExport.cpp FArchiveXML/FAXLightImport.cpp FArchiveXML/FAXMaterialExport.cpp FArchiveXML/FAXMaterialImport.cpp FArchiveXML/FAXPhysicsExport.cpp FArchiveXML/FAXPhysicsImport.cpp FArchiveXML/FAXSceneExport.cpp FArchiveXML/FAXSceneImport.cpp FArchiveXML/FArchiveXML.cpp FArchiveXML/StdAfx.cpp FCDocument/FCDAnimated.cpp FCDocument/FCDAnimation.cpp FCDocument/FCDAnimationChannel.cpp FCDocument/FCDAnimationClip.cpp FCDocument/FCDAnimationClipTools.cpp FCDocument/FCDAnimationCurve.cpp FCDocument/FCDAnimationCurveTools.cpp FCDocument/FCDAnimationKey.cpp FCDocument/FCDAnimationMultiCurve.cpp FCDocument/FCDAsset.cpp FCDocument/FCDCamera.cpp FCDocument/FCDController.cpp FCDocument/FCDControllerInstance.cpp FCDocument/FCDControllerTools.cpp FCDocument/FCDEffect.cpp FCDocument/FCDEffectCode.cpp FCDocument/FCDEffectParameter.cpp FCDocument/FCDEffectParameterFactory.cpp FCDocument/FCDEffectParameterSampler.cpp FCDocument/FCDEffectParameterSurface.cpp FCDocument/FCDEffectPass.cpp FCDocument/FCDEffectPassShader.cpp FCDocument/FCDEffectPassState.cpp FCDocument/FCDEffectProfile.cpp FCDocument/FCDEffectProfileFX.cpp FCDocument/FCDEffectStandard.cpp FCDocument/FCDEffectTechnique.cpp FCDocument/FCDEffectTools.cpp FCDocument/FCDEmitter.cpp FCDocument/FCDEmitterInstance.cpp FCDocument/FCDEmitterObject.cpp FCDocument/FCDEmitterParticle.cpp FCDocument/FCDEntity.cpp FCDocument/FCDEntityInstance.cpp FCDocument/FCDEntityReference.cpp FCDocument/FCDExternalReferenceManager.cpp FCDocument/FCDExtra.cpp FCDocument/FCDForceDeflector.cpp FCDocument/FCDForceDrag.cpp FCDocument/FCDForceField.cpp FCDocument/FCDForceGravity.cpp FCDocument/FCDForcePBomb.cpp FCDocument/FCDForceWind.cpp FCDocument/FCDGeometry.cpp FCDocument/FCDGeometryInstance.cpp FCDocument/FCDGeometryMesh.cpp FCDocument/FCDGeometryNURBSSurface.cpp FCDocument/FCDGeometryPolygons.cpp FCDocument/FCDGeometryPolygonsInput.cpp FCDocument/FCDGeometryPolygonsTools.cpp FCDocument/FCDGeometrySource.cpp FCDocument/FCDGeometrySpline.cpp FCDocument/FCDImage.cpp FCDocument/FCDLibrary.cpp FCDocument/FCDLight.cpp FCDocument/FCDLightTools.cpp FCDocument/FCDMaterial.cpp FCDocument/FCDMaterialInstance.cpp FCDocument/FCDMorphController.cpp FCDocument/FCDObject.cpp FCDocument/FCDObjectWithId.cpp FCDocument/FCDParameterAnimatable.cpp FCDocument/FCDParticleModifier.cpp FCDocument/FCDPhysicsAnalyticalGeometry.cpp FCDocument/FCDPhysicsForceFieldInstance.cpp FCDocument/FCDPhysicsMaterial.cpp FCDocument/FCDPhysicsModel.cpp FCDocument/FCDPhysicsModelInstance.cpp FCDocument/FCDPhysicsRigidBody.cpp FCDocument/FCDPhysicsRigidBodyInstance.cpp FCDocument/FCDPhysicsRigidBodyParameters.cpp FCDocument/FCDPhysicsRigidConstraint.cpp FCDocument/FCDPhysicsRigidConstraintInstance.cpp FCDocument/FCDPhysicsScene.cpp FCDocument/FCDPhysicsShape.cpp FCDocument/FCDPlaceHolder.cpp FCDocument/FCDSceneNode.cpp FCDocument/FCDSceneNodeIterator.cpp FCDocument/FCDSceneNodeTools.cpp FCDocument/FCDSkinController.cpp FCDocument/FCDTargetedEntity.cpp FCDocument/FCDTexture.cpp FCDocument/FCDTransform.cpp FCDocument/FCDVersion.cpp FCDocument/FCDocument.cpp FCDocument/FCDocumentTools.cpp FCollada.cpp FColladaPlugin.cpp FMath/FMAllocator.cpp FMath/FMAngleAxis.cpp FMath/FMColor.cpp FMath/FMInterpolation.cpp FMath/FMLookAt.cpp FMath/FMMatrix33.cpp FMath/FMMatrix44.cpp FMath/FMQuaternion.cpp FMath/FMRandom.cpp FMath/FMSkew.cpp FMath/FMVector3.cpp FMath/FMVolume.cpp FMath/StdAfx.cpp FUtils/FUAssert.cpp FUtils/FUBase64.cpp FUtils/FUBoundingBox.cpp FUtils/FUBoundingSphere.cpp FUtils/FUCrc32.cpp FUtils/FUCriticalSection.cpp FUtils/FUDaeEnum.cpp FUtils/FUDateTime.cpp FUtils/FUDebug.cpp FUtils/FUError.cpp FUtils/FUErrorLog.cpp FUtils/FUFile.cpp FUtils/FUFileManager.cpp FUtils/FULogFile.cpp FUtils/FUObject.cpp FUtils/FUObjectType.cpp FUtils/FUParameter.cpp FUtils/FUParameterizable.cpp FUtils/FUPluginManager.cpp FUtils/FUSemaphore.cpp FUtils/FUStringBuilder.cpp FUtils/FUStringConversion.cpp FUtils/FUSynchronizableObject.cpp FUtils/FUThread.cpp FUtils/FUTracker.cpp FUtils/FUUniqueStringMap.cpp FUtils/FUUniqueStringMapTest.cpp FUtils/FUUri.cpp FUtils/FUXmlDocument.cpp FUtils/FUXmlParser.cpp FUtils/FUXmlWriter.cpp FUtils/StdAfx.cpp LibXML/DOCBparser.c LibXML/HTMLparser.c LibXML/HTMLtree.c LibXML/SAX.c LibXML/SAX2.c LibXML/c14n.c LibXML/catalog.c LibXML/chvalid.c LibXML/debugXML.c LibXML/dict.c LibXML/encoding.c LibXML/entities.c LibXML/error.c LibXML/globals.c LibXML/hash.c LibXML/legacy.c LibXML/list.c LibXML/nanoftp.c LibXML/nanohttp.c LibXML/parser.c LibXML/parserInternals.c LibXML/pattern.c LibXML/relaxng.c LibXML/threads.c LibXML/tree.c LibXML/uri.c LibXML/valid.c LibXML/xinclude.c LibXML/xlink.c LibXML/xmlIO.c LibXML/xmlcatalog.c LibXML/xmlmemory.c LibXML/xmlmodule.c LibXML/xmlreader.c LibXML/xmlregexp.c LibXML/xmlsave.c LibXML/xmlstring.c LibXML/xmlunicode.c LibXML/xmlwriter.c StdAfx.cpp)


include $(BUILD_STATIC_LIBRARY)

#### o3dcore
#
include $(CLEAR_VARS)

LOCAL_MODULE    := o3dcore
LOCAL_CFLAGS    := -I$(LOCAL_PATH)/third_party/stlport/stlport -I$(LOCAL_PATH)/third_party/vectormath
LOCAL_SRC_FILES := $(addprefix core/, cross/bitmap.cc cross/bitmap.h cross/bitmap_dds.cc cross/bitmap_jpg.cc cross/bitmap_png.cc cross/bitmap_tga.cc cross/bounding_box.cc cross/bounding_box.h cross/buffer.cc cross/buffer.h cross/callback.h cross/canvas.cc cross/canvas.h cross/canvas_paint.cc cross/canvas_paint.h cross/canvas_shader.cc cross/canvas_shader.h cross/canvas_utils.h cross/class_manager.cc cross/class_manager.h cross/clear_buffer.cc cross/clear_buffer.h cross/client.cc cross/client.h cross/client_info.cc cross/client_info.h cross/core_metrics.cc cross/core_metrics.h cross/counter.cc cross/counter.h cross/counter_manager.cc cross/counter_manager.h cross/cursor.h cross/curve.cc cross/curve.h cross/ddsurfacedesc.h cross/display_mode.h cross/display_window.h cross/draw_context.cc cross/draw_context.h cross/draw_element.cc cross/draw_element.h cross/draw_list.cc cross/draw_list.h cross/draw_list_manager.cc cross/draw_list_manager.h cross/draw_pass.cc cross/draw_pass.h cross/effect.cc cross/effect.h cross/element.cc cross/element.h cross/error.h cross/error_status.cc cross/error_status.h cross/error_stream_manager.cc cross/error_stream_manager.h cross/evaluation_counter.cc cross/evaluation_counter.h cross/event.cc	cross/event.h cross/event_callback.h cross/event_manager.cc cross/event_manager.h cross/fake_vertex_source.cc cross/fake_vertex_source.h cross/features.cc cross/features.h cross/field.cc cross/field.h cross/file_request.cc cross/file_request.h cross/float_n.h cross/function.cc cross/function.h cross/iclass_manager.cc cross/iclass_manager.h cross/id_manager.cc cross/id_manager.h cross/ierror_status.cc cross/ierror_status.h cross/image_utils.cc cross/imain_thread_task_poster.cc cross/imain_thread_task_poster.h cross/install_check.h	cross/lost_resource_callback.h cross/material.cc cross/material.h cross/math_types.h cross/math_utilities.cc cross/math_utilities.h cross/matrix4_axis_rotation.cc cross/matrix4_axis_rotation.h cross/matrix4_composition.cc cross/matrix4_composition.h cross/matrix4_scale.cc cross/matrix4_scale.h cross/matrix4_translation.cc cross/matrix4_translation.h	cross/message_commands.cc cross/message_commands.h cross/message_queue.cc cross/message_queue.h	cross/named_object.cc cross/named_object.h cross/object_base.cc	cross/object_base.h cross/object_manager.cc cross/object_manager.h cross/pack.cc cross/pack.h cross/param.cc cross/param.h cross/param_array.cc	cross/param_array.h cross/param_cache.cc cross/param_cache.h cross/param_object.cc cross/param_object.h	cross/param_operation.cc cross/param_operation.h cross/performance_timer.h cross/precompile.cc cross/precompile.h cross/primitive.cc cross/primitive.h cross/processed_path.cc cross/processed_path.h cross/profiler.cc	cross/profiler.h cross/ray_intersection_info.cc	cross/ray_intersection_info.h cross/render_context.cc cross/render_context.h cross/render_event.h cross/render_node.cc cross/render_node.h cross/render_surface.cc cross/render_surface.h cross/render_surface_set.cc cross/render_surface_set.h cross/renderer.cc cross/renderer.h cross/renderer_platform.h cross/sampler.cc cross/sampler.h cross/semantic_manager.cc cross/semantic_manager.h cross/service_dependency.h cross/service_implementation.h cross/service_interface_traits.h cross/service_locator.cc cross/service_locator.h cross/shape.cc cross/shape.h cross/skin.cc cross/skin.h cross/smart_ptr.h	cross/standard_param.cc	cross/standard_param.h cross/state.cc cross/state.h cross/state_set.cc cross/state_set.h cross/stream.cc cross/stream.h	cross/stream_bank.cc cross/stream_bank.h cross/texture.cc cross/texture.h cross/texture_base.cc	cross/texture_base.h cross/tick_event.h	cross/timer.cc cross/timer.h cross/timingtable.h cross/transform.cc cross/transform.h cross/transformation_context.cc cross/transformation_context.h cross/tree_traversal.cc cross/tree_traversal.h cross/types.h cross/vector_map.h cross/vertex_source.cc cross/vertex_source.h cross/viewport.cc cross/viewport.h cross/visitor_base.h cross/weak_ptr.h)




include $(BUILD_STATIC_LIBRARY)

# second lib, which will depend on and include the first one
#
include $(CLEAR_VARS)

LOCAL_MODULE    := libo3djni
LOCAL_CFLAGS    := -Werror
LOCAL_SRC_FILES := gl_code.cpp
LOCAL_LDLIBS    := -llog -lGLESv2

LOCAL_STATIC_LIBRARIES := o3dcore fcollada libjpeg zlib stdport

include $(BUILD_SHARED_LIBRARY)


