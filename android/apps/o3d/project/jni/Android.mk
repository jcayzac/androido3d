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
LOCAL_SRC_FILES := $(addprefix third_party/zlib/, \
  contrib/minizip/ioapi.c \
  contrib/minizip/unzip.c \
  contrib/minizip/zip.c \
  adler32.c \
  compress.c \
  crc32.c \
  deflate.c \
  gzio.c \
  infback.c \
  inffast.c \
  inflate.c \
  inftrees.c \
  trees.c \
  uncompr.c \
  zutil.c \
  )

include $(BUILD_STATIC_LIBRARY)

#### libpng
#
include $(CLEAR_VARS)

LOCAL_MODULE    := libpng
LOCAL_CFLAGS    := -DCHROME_PNG_WRITE_SUPPORT -DPNG_USER_CONFIG
LOCAL_SRC_FILES := $(addprefix third_party/libpng/, \
  png.c \
  png.h \
  pngconf.h \
  pngerror.c \
  pnggccrd.c \
  pngget.c \
  pngmem.c \
  pngpread.c \
  pngread.c \
  pngrio.c \
  pngrtran.c \
  pngrutil.c \
  pngset.c \
  pngtrans.c \
  pngusr.h \
  pngvcrd.c \
  pngwio.c \
  pngwrite.c \
  pngwtran.c \
  pngwutil.c \
  )

include $(BUILD_STATIC_LIBRARY)

#### libjpeg
#
include $(CLEAR_VARS)

LOCAL_MODULE    := libjpeg
LOCAL_SRC_FILES := $(addprefix third_party/libjpeg/, \
  jcapimin.c \
  jcapistd.c \
  jccoefct.c \
  jccolor.c \
  jcdctmgr.c \
  jchuff.c \
  jcinit.c \
  jcmainct.c \
  jcmarker.c \
  jcmaster.c \
  jcomapi.c \
  jcparam.c \
  jcphuff.c \
  jcprepct.c \
  jcsample.c \
  jdapimin.c \
  jdapistd.c \
  jdatadst.c \
  jdatasrc.c \
  jdcoefct.c \
  jdcolor.c \
  jddctmgr.c \
  jdhuff.c \
  jdinput.c \
  jdmainct.c \
  jdmarker.c \
  jdmaster.c \
  jdmerge.c \
  jdphuff.c \
  jdpostct.c \
  jdsample.c \
  jerror.c \
  jfdctflt.c \
  jfdctfst.c \
  jfdctint.c \
  jidctflt.c \
  jidctfst.c \
  jidctint.c \
  jmemmgr.c \
  jmemnobs.c \
  jquant1.c \
  jquant2.c \
  jutils.c \
  )

include $(BUILD_STATIC_LIBRARY)

#### skia
#
include $(CLEAR_VARS)

LOCAL_MODULE    := skia
LOCAL_CFLAGS    := -DSK_BUILD_FOR_UNIX
LOCAL_SRC_FILES := \
        third_party/skia/src/core/Sk64.cpp \
        third_party/skia/src/core/SkAlphaRuns.cpp \
        third_party/skia/src/core/SkBitmap.cpp \
        third_party/skia/src/core/SkBitmapProcShader.cpp \
        third_party/skia/src/core/SkBitmapProcState.cpp \
        third_party/skia/src/core/SkBitmapProcState_matrixProcs.cpp \
        third_party/skia/src/core/SkBitmapSampler.cpp \
        third_party/skia/src/core/SkBitmap_scroll.cpp \
        third_party/skia/src/core/SkBlitRow_D16.cpp \
        third_party/skia/src/core/SkBlitRow_D32.cpp \
        third_party/skia/src/core/SkBlitRow_D4444.cpp \
        third_party/skia/src/core/SkBlitter.cpp \
        third_party/skia/src/core/SkBlitter_4444.cpp \
        third_party/skia/src/core/SkBlitter_A1.cpp \
        third_party/skia/src/core/SkBlitter_A8.cpp \
        third_party/skia/src/core/SkBlitter_ARGB32.cpp \
        third_party/skia/src/core/SkBlitter_RGB16.cpp \
        third_party/skia/src/core/SkBlitter_Sprite.cpp \
        third_party/skia/src/core/SkBuffer.cpp \
        third_party/skia/src/core/SkCanvas.cpp \
        third_party/skia/src/core/SkChunkAlloc.cpp \
        third_party/skia/src/core/SkColor.cpp \
        third_party/skia/src/core/SkColorFilter.cpp \
        third_party/skia/src/core/SkColorTable.cpp \
        third_party/skia/src/core/SkComposeShader.cpp \
        third_party/skia/src/core/SkConcaveToTriangles.cpp \
        third_party/skia/src/core/SkCordic.cpp \
        third_party/skia/src/core/SkCubicClipper.cpp \
        third_party/skia/src/core/SkDebug.cpp \
        #third_party/skia/src/core/SkDebug_stdio.cpp \
        third_party/skia/src/core/SkDeque.cpp \
        third_party/skia/src/core/SkDevice.cpp \
        third_party/skia/src/core/SkDither.cpp \
        third_party/skia/src/core/SkDraw.cpp \
        #third_party/skia/src/core/SkDrawing.cpp \
        third_party/skia/src/core/SkEdgeBuilder.cpp \
        third_party/skia/src/core/SkEdgeClipper.cpp \
        third_party/skia/src/core/SkEdge.cpp \
        third_party/skia/src/core/SkFilterProc.cpp \
        third_party/skia/src/core/SkFlattenable.cpp \
        third_party/skia/src/core/SkFloat.cpp \
        third_party/skia/src/core/SkFloatBits.cpp \
        third_party/skia/src/core/SkGeometry.cpp \
        third_party/skia/src/core/SkGlobals.cpp \
        third_party/skia/src/core/SkGlyphCache.cpp \
        third_party/skia/src/core/SkGraphics.cpp \
        third_party/skia/src/core/SkLineClipper.cpp \
        third_party/skia/src/core/SkMMapStream.cpp \
        third_party/skia/src/core/SkMask.cpp \
        third_party/skia/src/core/SkMaskFilter.cpp \
        third_party/skia/src/core/SkMath.cpp \
        third_party/skia/src/core/SkMatrix.cpp \
        third_party/skia/src/core/SkPackBits.cpp \
        third_party/skia/src/core/SkPaint.cpp \
        third_party/skia/src/core/SkPath.cpp \
        third_party/skia/src/core/SkPathEffect.cpp \
        third_party/skia/src/core/SkPathHeap.cpp \
        third_party/skia/src/core/SkPathMeasure.cpp \
        third_party/skia/src/core/SkPicture.cpp \
        third_party/skia/src/core/SkPictureFlat.cpp \
        third_party/skia/src/core/SkPicturePlayback.cpp \
        third_party/skia/src/core/SkPictureRecord.cpp \
        third_party/skia/src/core/SkPixelRef.cpp \
        third_party/skia/src/core/SkPoint.cpp \
        third_party/skia/src/core/SkProcSpriteBlitter.cpp \
        third_party/skia/src/core/SkPtrRecorder.cpp \
        third_party/skia/src/core/SkQuadClipper.cpp \
        third_party/skia/src/core/SkRasterizer.cpp \
        third_party/skia/src/core/SkRect.cpp \
        third_party/skia/src/core/SkRefCnt.cpp \
        third_party/skia/src/core/SkRegion.cpp \
        third_party/skia/src/core/SkRegion_path.cpp \
        third_party/skia/src/core/SkScalerContext.cpp \
        third_party/skia/src/core/SkScan.cpp \
        third_party/skia/src/core/SkScan_AntiPath.cpp \
        third_party/skia/src/core/SkScan_Antihair.cpp \
        third_party/skia/src/core/SkScan_Hairline.cpp \
        third_party/skia/src/core/SkScan_Path.cpp \
        third_party/skia/src/core/SkShader.cpp \
        third_party/skia/src/core/SkShape.cpp \
        third_party/skia/src/core/SkSpriteBlitter_ARGB32.cpp \
        third_party/skia/src/core/SkSpriteBlitter_RGB16.cpp \
        third_party/skia/src/core/SkStream.cpp \
        third_party/skia/src/core/SkString.cpp \
        third_party/skia/src/core/SkStroke.cpp \
        third_party/skia/src/core/SkStrokerPriv.cpp \
        third_party/skia/src/core/SkTSearch.cpp \
        third_party/skia/src/core/SkTypeface.cpp \
        third_party/skia/src/core/SkUnPreMultiply.cpp \
        third_party/skia/src/core/SkUtils.cpp \
        third_party/skia/src/core/SkWriter32.cpp \
        third_party/skia/src/core/SkXfermode.cpp \
	\
        third_party/skia/src/effects/Sk1DPathEffect.cpp \
        third_party/skia/src/effects/Sk2DPathEffect.cpp \
        third_party/skia/src/effects/SkAvoidXfermode.cpp \
        third_party/skia/src/effects/SkBlurDrawLooper.cpp \
        third_party/skia/src/effects/SkBlurMask.cpp \
        third_party/skia/src/effects/SkBlurMaskFilter.cpp \
        third_party/skia/src/effects/SkColorFilters.cpp \
        third_party/skia/src/effects/SkColorMatrixFilter.cpp \
        third_party/skia/src/effects/SkCornerPathEffect.cpp \
        third_party/skia/src/effects/SkDashPathEffect.cpp \
        third_party/skia/src/effects/SkDiscretePathEffect.cpp \
        third_party/skia/src/effects/SkEmbossMask.cpp \
        third_party/skia/src/effects/SkEmbossMaskFilter.cpp \
        third_party/skia/src/effects/SkGradientShader.cpp \
        third_party/skia/src/effects/SkKernel33MaskFilter.cpp \
        third_party/skia/src/effects/SkLayerDrawLooper.cpp \
        third_party/skia/src/effects/SkLayerRasterizer.cpp \
        third_party/skia/src/effects/SkPaintFlagsDrawFilter.cpp \
        third_party/skia/src/effects/SkPorterDuff.cpp \
        third_party/skia/src/effects/SkPixelXorXfermode.cpp \
        third_party/skia/src/effects/SkTransparentShader.cpp \
	\
        third_party/skia/src/images/bmpdecoderhelper.cpp \
        third_party/skia/src/images/SkCreateRLEPixelRef.cpp \
        #third_party/skia/src/images/SkFDStream.cpp \
        #third_party/skia/src/images/SkFlipPixelRef.cpp \
        third_party/skia/src/images/SkImageDecoder.cpp \
        third_party/skia/src/images/SkImageDecoder_Factory.cpp \
        #third_party/skia/src/images/SkImageDecoder_fpdfemb.cpp \
        #third_party/skia/src/images/SkImageDecoder_libbmp.cpp \
        #third_party/skia/src/images/SkImageDecoder_libgif.cpp \
        #third_party/skia/src/images/SkImageDecoder_libico.cpp \
        #third_party/skia/src/images/SkImageDecoder_libjpeg.cpp \
        #third_party/skia/src/images/SkImageDecoder_libpng.cpp \
        #third_party/skia/src/images/SkImageDecoder_libpvjpeg.cpp \
        #third_party/skia/src/images/SkImageDecoder_wbmp.cpp \
        #third_party/skia/src/images/SKImageEncoder.cpp \
        #third_party/skia/src/images/SKImageEncoder_Factory.cpp \
        #third_party/skia/src/images/SkImageRef.cpp \
        #third_party/skia/src/images/SkImageRefPool.cpp \
        #third_party/skia/src/images/SkImageRef_GlobalPool.cpp \
        #third_party/skia/src/images/SkMovie.cpp \
        #third_party/skia/src/images/SkMovie_gif.cpp \
        third_party/skia/src/images/SkScaledBitmapSampler.cpp \
	\
        third_party/skia/src/opts/opts_check_SSE2.cpp \
	\
        #third_party/skia/src/ports/SkFontHost_FONTPATH.cpp \
        third_party/skia/src/ports/SkFontHost_FreeType.cpp \
        #third_party/skia/src/ports/SkFontHost_android.cpp \
        #third_party/skia/src/ports/SkFontHost_ascender.cpp \
        third_party/skia/src/ports/SkFontHost_tables.cpp \
        #third_party/skia/src/ports/SkFontHost_gamma.cpp \
        third_party/skia/src/ports/SkFontHost_gamma_none.cpp \
        #third_party/skia/src/ports/SkFontHost_linux.cpp \
        third_party/skia/src/ports/SkFontHost_mac.cpp \
        #third_party/skia/src/ports/SkFontHost_none.cpp \
        third_party/skia/src/ports/SkFontHost_win.cpp \
        third_party/skia/src/ports/SkGlobals_global.cpp \
        #third_party/skia/src/ports/SkImageDecoder_CG.cpp \
        #third_party/skia/src/ports/SkImageDecoder_empty.cpp \
        #third_party/skia/src/ports/SkImageRef_ashmem.cpp \
        #third_party/skia/src/ports/SkOSEvent_android.cpp \
        #third_party/skia/src/ports/SkOSEvent_dummy.cpp \
        third_party/skia/src/ports/SkOSFile_stdio.cpp \
        #third_party/skia/src/ports/SkThread_none.cpp \
        third_party/skia/src/ports/SkThread_pthread.cpp \
        third_party/skia/src/ports/SkThread_win.cpp \
        third_party/skia/src/ports/SkTime_Unix.cpp \
        #third_party/skia/src/ports/SkXMLParser_empty.cpp \
        #third_party/skia/src/ports/SkXMLParser_expat.cpp \
        #third_party/skia/src/ports/SkXMLParser_tinyxml.cpp \
        #third_party/skia/src/ports/SkXMLPullParser_expat.cpp \
	\
        third_party/skia/src/utils/mac/SkCreateCGImageRef.cpp \
	\
        skia/ext/bitmap_platform_device_linux.cc \
        skia/ext/bitmap_platform_device_mac.cc \
        skia/ext/bitmap_platform_device_win.cc \
        skia/ext/convolver.cc \
        skia/ext/google_logging.cc \
        skia/ext/image_operations.cc \
        skia/ext/platform_canvas.cc \
        skia/ext/platform_canvas_linux.cc \
        skia/ext/platform_canvas_mac.cc \
        skia/ext/platform_canvas_win.cc \
        skia/ext/platform_device_linux.cc \
        skia/ext/platform_device_mac.cc \
        skia/ext/platform_device_win.cc \
        skia/ext/SkMemory_new_handler.cpp \
        skia/ext/skia_utils.cc \
        skia/ext/skia_utils_gtk.cc \
        skia/ext/skia_utils_mac.mm \
        skia/ext/skia_utils_win.cc \
        skia/ext/vector_canvas.cc \
        skia/ext/vector_canvas_linux.cc \
        skia/ext/vector_canvas_win.cc \
        skia/ext/vector_platform_device_linux.cc \
        skia/ext/vector_platform_device_win.cc \

include $(BUILD_STATIC_LIBRARY)

#### LibXML for fcollada
#
include $(CLEAR_VARS)

LOCAL_MODULE    := libxml
LOCAL_CFLAGS    := \
  -DLINUX \
  -D__ANDROID__ \
  -DUNICODE \
  -DFCOLLADA_EXCEPTION=0 \
  -I$(LOCAL_PATH)/third_party/fcollada/files/LibXML/include \
  -I$(LOCAL_PATH)/third_party/fcollada/files \
  -I$(LOCAL_PATH)/third_party/stlport/stlport \
  -I$(LOCAL_PATH)/third_party/chromium \

LOCAL_SRC_FILES := $(addprefix third_party/fcollada/files/, \
  LibXML/DOCBparser.c \
  LibXML/HTMLparser.c \
  LibXML/HTMLtree.c \
  LibXML/SAX.c \
  LibXML/SAX2.c \
  LibXML/c14n.c \
  LibXML/catalog.c \
  LibXML/chvalid.c \
  LibXML/debugXML.c \
  LibXML/dict.c \
  LibXML/encoding.c \
  LibXML/entities.c \
  LibXML/error.c \
  LibXML/globals.c \
  LibXML/hash.c \
  LibXML/legacy.c \
  LibXML/list.c \
  LibXML/nanoftp.c \
  LibXML/nanohttp.c \
  LibXML/parser.c \
  LibXML/parserInternals.c \
  LibXML/pattern.c \
  LibXML/relaxng.c \
  LibXML/threads.c \
  LibXML/tree.c \
  LibXML/uri.c \
  LibXML/valid.c \
  LibXML/xinclude.c \
  LibXML/xlink.c \
  LibXML/xmlIO.c \
  LibXML/xmlcatalog.c \
  LibXML/xmlmemory.c \
  LibXML/xmlmodule.c \
  LibXML/xmlreader.c \
  LibXML/xmlregexp.c \
  LibXML/xmlsave.c \
  LibXML/xmlstring.c \
  LibXML/xmlunicode.c \
  LibXML/xmlwriter.c \
  StdAfx.cpp \
  )

include $(BUILD_STATIC_LIBRARY)

#### fcollada
#
include $(CLEAR_VARS)

# DLLEntry.cpp \

LOCAL_MODULE    := fcollada
LOCAL_CFLAGS    := \
  -D__ANDROID__ \
  -DUNICODE \
  -DFCOLLADA_EXCEPTION=0 \
  -I$(LOCAL_PATH)/third_party/fcollada/files/LibXML/include \
  -I$(LOCAL_PATH)/third_party/fcollada/files \
  -I$(LOCAL_PATH)/third_party/stlport/stlport \
  -I$(LOCAL_PATH)/third_party/chromium \

LOCAL_SRC_FILES := $(addprefix third_party/fcollada/files/, \
  FArchiveXML/FAXAnimationExport.cpp \
  FArchiveXML/FAXAnimationImport.cpp \
  FArchiveXML/FAXCameraExport.cpp \
  FArchiveXML/FAXCameraImport.cpp \
  FArchiveXML/FAXColladaParser.cpp \
  FArchiveXML/FAXColladaWriter.cpp \
  FArchiveXML/FAXControllerExport.cpp \
  FArchiveXML/FAXControllerImport.cpp \
  FArchiveXML/FAXEmitterExport.cpp \
  FArchiveXML/FAXEmitterImport.cpp \
  FArchiveXML/FAXEntityExport.cpp \
  FArchiveXML/FAXEntityImport.cpp \
  FArchiveXML/FAXForceFieldExport.cpp \
  FArchiveXML/FAXForceFieldImport.cpp \
  FArchiveXML/FAXGeometryExport.cpp \
  FArchiveXML/FAXGeometryImport.cpp \
  FArchiveXML/FAXImportLinking.cpp \
  FArchiveXML/FAXInstanceExport.cpp \
  FArchiveXML/FAXInstanceImport.cpp \
  FArchiveXML/FAXLightExport.cpp \
  FArchiveXML/FAXLightImport.cpp \
  FArchiveXML/FAXMaterialExport.cpp \
  FArchiveXML/FAXMaterialImport.cpp \
  FArchiveXML/FAXPhysicsExport.cpp \
  FArchiveXML/FAXPhysicsImport.cpp \
  FArchiveXML/FAXSceneExport.cpp \
  FArchiveXML/FAXSceneImport.cpp \
  FArchiveXML/FArchiveXML.cpp \
  FArchiveXML/StdAfx.cpp \
  FCDocument/FCDAnimated.cpp \
  FCDocument/FCDAnimation.cpp \
  FCDocument/FCDAnimationChannel.cpp \
  FCDocument/FCDAnimationClip.cpp \
  FCDocument/FCDAnimationClipTools.cpp \
  FCDocument/FCDAnimationCurve.cpp \
  FCDocument/FCDAnimationCurveTools.cpp \
  FCDocument/FCDAnimationKey.cpp \
  FCDocument/FCDAnimationMultiCurve.cpp \
  FCDocument/FCDAsset.cpp \
  FCDocument/FCDCamera.cpp \
  FCDocument/FCDController.cpp \
  FCDocument/FCDControllerInstance.cpp \
  FCDocument/FCDControllerTools.cpp \
  FCDocument/FCDEffect.cpp \
  FCDocument/FCDEffectCode.cpp \
  FCDocument/FCDEffectParameter.cpp \
  FCDocument/FCDEffectParameterFactory.cpp \
  FCDocument/FCDEffectParameterSampler.cpp \
  FCDocument/FCDEffectParameterSurface.cpp \
  FCDocument/FCDEffectPass.cpp \
  FCDocument/FCDEffectPassShader.cpp \
  FCDocument/FCDEffectPassState.cpp \
  FCDocument/FCDEffectProfile.cpp \
  FCDocument/FCDEffectProfileFX.cpp \
  FCDocument/FCDEffectStandard.cpp \
  FCDocument/FCDEffectTechnique.cpp \
  FCDocument/FCDEffectTools.cpp \
  FCDocument/FCDEmitter.cpp \
  FCDocument/FCDEmitterInstance.cpp \
  FCDocument/FCDEmitterObject.cpp \
  FCDocument/FCDEmitterParticle.cpp \
  FCDocument/FCDEntity.cpp \
  FCDocument/FCDEntityInstance.cpp \
  FCDocument/FCDEntityReference.cpp \
  FCDocument/FCDExternalReferenceManager.cpp \
  FCDocument/FCDExtra.cpp \
  FCDocument/FCDForceDeflector.cpp \
  FCDocument/FCDForceDrag.cpp \
  FCDocument/FCDForceField.cpp \
  FCDocument/FCDForceGravity.cpp \
  FCDocument/FCDForcePBomb.cpp \
  FCDocument/FCDForceWind.cpp \
  FCDocument/FCDGeometry.cpp \
  FCDocument/FCDGeometryInstance.cpp \
  FCDocument/FCDGeometryMesh.cpp \
  FCDocument/FCDGeometryNURBSSurface.cpp \
  FCDocument/FCDGeometryPolygons.cpp \
  FCDocument/FCDGeometryPolygonsInput.cpp \
  FCDocument/FCDGeometryPolygonsTools.cpp \
  FCDocument/FCDGeometrySource.cpp \
  FCDocument/FCDGeometrySpline.cpp \
  FCDocument/FCDImage.cpp \
  FCDocument/FCDLibrary.cpp \
  FCDocument/FCDLight.cpp \
  FCDocument/FCDLightTools.cpp \
  FCDocument/FCDMaterial.cpp \
  FCDocument/FCDMaterialInstance.cpp \
  FCDocument/FCDMorphController.cpp \
  FCDocument/FCDObject.cpp \
  FCDocument/FCDObjectWithId.cpp \
  FCDocument/FCDParameterAnimatable.cpp \
  FCDocument/FCDParticleModifier.cpp \
  FCDocument/FCDPhysicsAnalyticalGeometry.cpp \
  FCDocument/FCDPhysicsForceFieldInstance.cpp \
  FCDocument/FCDPhysicsMaterial.cpp \
  FCDocument/FCDPhysicsModel.cpp \
  FCDocument/FCDPhysicsModelInstance.cpp \
  FCDocument/FCDPhysicsRigidBody.cpp \
  FCDocument/FCDPhysicsRigidBodyInstance.cpp \
  FCDocument/FCDPhysicsRigidBodyParameters.cpp \
  FCDocument/FCDPhysicsRigidConstraint.cpp \
  FCDocument/FCDPhysicsRigidConstraintInstance.cpp \
  FCDocument/FCDPhysicsScene.cpp \
  FCDocument/FCDPhysicsShape.cpp \
  FCDocument/FCDPlaceHolder.cpp \
  FCDocument/FCDSceneNode.cpp \
  FCDocument/FCDSceneNodeIterator.cpp \
  FCDocument/FCDSceneNodeTools.cpp \
  FCDocument/FCDSkinController.cpp \
  FCDocument/FCDTargetedEntity.cpp \
  FCDocument/FCDTexture.cpp \
  FCDocument/FCDTransform.cpp \
  FCDocument/FCDVersion.cpp \
  FCDocument/FCDocument.cpp \
  FCDocument/FCDocumentTools.cpp \
  FCollada.cpp \
  FColladaPlugin.cpp \
  FMath/FMAllocator.cpp \
  FMath/FMAngleAxis.cpp \
  FMath/FMColor.cpp \
  FMath/FMInterpolation.cpp \
  FMath/FMLookAt.cpp \
  FMath/FMMatrix33.cpp \
  FMath/FMMatrix44.cpp \
  FMath/FMQuaternion.cpp \
  FMath/FMRandom.cpp \
  FMath/FMSkew.cpp \
  FMath/FMVector3.cpp \
  FMath/FMVolume.cpp \
  FMath/StdAfx.cpp \
  FUtils/FUAssert.cpp \
  FUtils/FUBase64.cpp \
  FUtils/FUBoundingBox.cpp \
  FUtils/FUBoundingSphere.cpp \
  FUtils/FUCrc32.cpp \
  FUtils/FUCriticalSection.cpp \
  FUtils/FUDaeEnum.cpp \
  FUtils/FUDateTime.cpp \
  FUtils/FUDebug.cpp \
  FUtils/FUError.cpp \
  FUtils/FUErrorLog.cpp \
  FUtils/FUFile.cpp \
  FUtils/FUFileManager.cpp \
  FUtils/FULogFile.cpp \
  FUtils/FUObject.cpp \
  FUtils/FUObjectType.cpp \
  FUtils/FUParameter.cpp \
  FUtils/FUParameterizable.cpp \
  FUtils/FUPluginManager.cpp \
  FUtils/FUSemaphore.cpp \
  FUtils/FUStringBuilder.cpp \
  FUtils/FUStringConversion.cpp \
  FUtils/FUSynchronizableObject.cpp \
  FUtils/FUThread.cpp \
  FUtils/FUTracker.cpp \
  FUtils/FUUniqueStringMap.cpp \
  FUtils/FUUniqueStringMapTest.cpp \
  FUtils/FUUri.cpp \
  FUtils/FUXmlDocument.cpp \
  FUtils/FUXmlParser.cpp \
  FUtils/FUXmlWriter.cpp \
  FUtils/StdAfx.cpp \
  StdAfx.cpp \
  )

include $(BUILD_STATIC_LIBRARY)

#### o3dcore
#
include $(CLEAR_VARS)

LOCAL_MODULE    := o3dcore
LOCAL_CPP_EXTENSION := .cc
LOCAL_CFLAGS    := \
  -D__ANDROID__ \
  -DO3D_NO_CANVAS \
  -DO3D_NO_GPU2D \
  -DO3D_NO_IPC \
  -DO3D_NO_ARCHIVE_REQUEST \
  -DO3D_NO_FILE_REQUEST \
  -DO3D_PLUGIN_VERSION=\"0.1.43.0\" \
  -I$(LOCAL_PATH)/third_party/stlport/stlport \
  -I$(LOCAL_PATH)/third_party/libjpeg \
  -I$(LOCAL_PATH)/third_party/libpng \
  -I$(LOCAL_PATH)/third_party/chromium \
  -I$(LOCAL_PATH)/third_party \
  -I$(LOCAL_PATH) \

# unused by Android version of O3D.
#  canvas.cc \
#  canvas_paint.cc \
#  core_metrics.cc \
#  fake_vertex_source.cc \
#  message_commands.cc \
#  message_queue.cc \

LOCAL_SRC_FILES := $(addprefix core/cross/, \
  bitmap.cc \
  bitmap_dds.cc \
  bitmap_jpg.cc \
  bitmap_png.cc \
  bitmap_tga.cc \
  bounding_box.cc \
  buffer.cc \
  class_manager.cc \
  clear_buffer.cc \
  client.cc \
  client_info.cc \
  counter.cc \
  counter_manager.cc \
  curve.cc \
  draw_context.cc \
  draw_element.cc \
  draw_list.cc \
  draw_list_manager.cc \
  draw_pass.cc \
  effect.cc \
  element.cc \
  error_status.cc \
  error_stream_manager.cc \
  evaluation_counter.cc \
  event.cc \
  event_manager.cc \
  features.cc \
  field.cc \
  file_request.cc \
  function.cc \
  iclass_manager.cc \
  id_manager.cc \
  ierror_status.cc \
  image_utils.cc \
  material.cc \
  matrix4_axis_rotation.cc \
  matrix4_composition.cc \
  matrix4_scale.cc \
  matrix4_translation.cc \
  named_object.cc \
  object_base.cc \
  object_manager.cc \
  pack.cc \
  param.cc \
  param_array.cc \
  param_object.cc \
  param_operation.cc \
  precompile.cc \
  primitive.cc \
  profiler.cc \
  ray_intersection_info.cc \
  render_context.cc \
  render_node.cc \
  render_surface.cc \
  render_surface_set.cc \
  renderer.cc \
  sampler.cc \
  semantic_manager.cc \
  service_locator.cc \
  skin.cc \
  standard_param.cc \
  state.cc \
  state_set.cc \
  stream.cc \
  stream_bank.cc \
  texture.cc \
  texture_base.cc \
  timer.cc \
  transform.cc \
  transformation_context.cc \
  tree_traversal.cc \
  vertex_source.cc \
  viewport.cc \
  )

include $(BUILD_STATIC_LIBRARY)

#### o3drenderer
#

include $(CLEAR_VARS)

LOCAL_MODULE    := o3drenderer
LOCAL_CPP_EXTENSION := .cc
LOCAL_CFLAGS    := \
  -D__ANDROID__ \
  -DRENDERER_GLES2 \
  -DGLES2_BACKEND_NATIVE_GLES2 \
  -I$(LOCAL_PATH)/third_party/stlport/stlport \
  -I$(LOCAL_PATH)/third_party/libjpeg \
  -I$(LOCAL_PATH)/third_party/libpng \
  -I$(LOCAL_PATH)/third_party/chromium \
  -I$(LOCAL_PATH)/third_party \
  -I$(LOCAL_PATH) \

LOCAL_SRC_FILES := $(addprefix core/cross/gles2/, \
  buffer_gles2.cc \
  draw_element_gles2.cc \
  effect_gles2.cc \
  install_check.cc \
  param_cache_gles2.cc \
  primitive_gles2.cc \
  renderer_gles2.cc \
  render_surface_gles2.cc \
  sampler_gles2.cc \
  stream_bank_gles2.cc \
  texture_gles2.cc \
  utils_gles2.cc \
  )

include $(BUILD_STATIC_LIBRARY)

#### o3dimport
#

include $(CLEAR_VARS)

LOCAL_MODULE    := o3dimport
LOCAL_CPP_EXTENSION := .cc
LOCAL_CFLAGS    := \
  -D__ANDROID__ \
  -DRENDERER_GLES2 \
  -DGLES2_BACKEND_NATIVE_GLES2 \
  -DO3D_IMPORT_NO_CG \
  -DO3D_IMPORT_NO_CONDITIONER \
  -DO3D_IMPORT_NO_DXT_DECOMPRESSION \
  -DFCOLLADA_EXCEPTION=0 \
  -DUNICODE \
  -I$(LOCAL_PATH)/third_party/stlport/stlport \
  -I$(LOCAL_PATH)/third_party/fcollada/files/LibXML/include \
  -I$(LOCAL_PATH)/third_party/fcollada/files \
  -I$(LOCAL_PATH)/third_party/chromium \
  -I$(LOCAL_PATH)/third_party/zlib \
  -I$(LOCAL_PATH)/third_party \
  -I$(LOCAL_PATH) \

LOCAL_SRC_FILES := $(addprefix import/cross/, \
  collada.cc \
  collada.h \
  collada_zip_archive.cc \
  collada_zip_archive.h \
  )

include $(BUILD_STATIC_LIBRARY)

# second lib, which will depend on and include the first one
#
include $(CLEAR_VARS)

LOCAL_MODULE    := libo3djni
LOCAL_CFLAGS    := \
  -Werror \
  -D__ANDROID__ \
  -I$(LOCAL_PATH)/third_party/stlport/stlport \
  -I$(LOCAL_PATH)/third_party/chromium \

LOCAL_LDLIBS    := -llog -lGLESv2
LOCAL_SRC_FILES := \
  gl_code.cpp \
  render_graph.cpp \
  shader_builder.cpp \

# LOCAL_STATIC_LIBRARIES := o3dcore fcollada libxml libpng libjpeg zlib stdport
LOCAL_STATIC_LIBRARIES := libxml o3dimport o3drenderer o3dcore fcollada libpng libjpeg zlib stdport

include $(BUILD_SHARED_LIBRARY)


