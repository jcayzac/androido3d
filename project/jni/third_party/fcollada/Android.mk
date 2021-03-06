LOCAL_PATH := $(call my-dir)

#### LibXML for fcollada
#
include $(O3D_START_MODULE)

LOCAL_MODULE := xml
LOCAL_CFLAGS += \
  -DLINUX \
  -DFCOLLADA_EXCEPTION=0 \
  -DRETAIL \

LOCAL_C_INCLUDES += $(addprefix $(O3D_THIRD_PARTY)/fcollada/include/, \
  FCollada \
  FCollada/LibXML/include \
  FColladaPlugins \
)

LOCAL_SRC_FILES := $(addprefix current/FCollada/, \
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

include $(O3D_BUILD_MODULE)

#### fcollada
#
include $(O3D_START_MODULE)

LOCAL_MODULE := fcollada
LOCAL_CFLAGS += \
  -DLINUX \
  -DFCOLLADA_EXCEPTION=0 \
  -DRETAIL \

LOCAL_C_INCLUDES += $(addprefix $(O3D_THIRD_PARTY)/fcollada/include/, \
  FCollada \
  FCollada/LibXML/include \
  FColladaPlugins \
)

LOCAL_SRC_FILES := $(addprefix current/FColladaPlugins/FArchiveXML/, \
  FAXAnimationExport.cpp \
  FAXAnimationImport.cpp \
  FAXCameraExport.cpp \
  FAXCameraImport.cpp \
  FAXColladaParser.cpp \
  FAXColladaWriter.cpp \
  FAXControllerExport.cpp \
  FAXControllerImport.cpp \
  FAXEmitterExport.cpp \
  FAXEmitterImport.cpp \
  FAXEntityExport.cpp \
  FAXEntityImport.cpp \
  FAXForceFieldExport.cpp \
  FAXForceFieldImport.cpp \
  FAXGeometryExport.cpp \
  FAXGeometryImport.cpp \
  FAXImportLinking.cpp \
  FAXInstanceExport.cpp \
  FAXInstanceImport.cpp \
  FAXLightExport.cpp \
  FAXLightImport.cpp \
  FAXMaterialExport.cpp \
  FAXMaterialImport.cpp \
  FAXPhysicsExport.cpp \
  FAXPhysicsImport.cpp \
  FAXSceneExport.cpp \
  FAXSceneImport.cpp \
  FArchiveXML.cpp \
  StdAfx.cpp \
  ) $(addprefix current/FCollada/, \
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
  FUtils/FUUri.cpp \
  FUtils/FUXmlDocument.cpp \
  FUtils/FUXmlParser.cpp \
  FUtils/FUXmlWriter.cpp \
  FUtils/StdAfx.cpp \
  StdAfx.cpp \
  )

include $(O3D_BUILD_MODULE)
