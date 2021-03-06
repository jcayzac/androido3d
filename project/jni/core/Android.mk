#### o3dcore
#
LOCAL_PATH      := $(call my-dir)

include $(O3D_START_MODULE)

LOCAL_MODULE    := o3dcore
LOCAL_CPP_EXTENSION := .cc
LOCAL_C_INCLUDES += \
  $(O3D_THIRD_PARTY)/libjpeg/include \
  $(O3D_THIRD_PARTY)/libpng/include \

LOCAL_SRC_FILES := $(addprefix cross/, \
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
  file_resource.cc \
  function.cc \
  iclass_manager.cc \
  id_manager.cc \
  ierror_status.cc \
  image_utils.cc \
  material.cc \
  math_utilities.cc \
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
  param_cache.cc \
  param_object.cc \
  param_operation.cc \
  picking_context.cc \
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
  shape.cc \
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

include $(O3D_BUILD_MODULE)

#### o3drenderer
#

include $(O3D_START_MODULE)

LOCAL_MODULE := o3drenderer
LOCAL_CPP_EXTENSION := .cc
LOCAL_C_INCLUDES += \
  $(O3D_THIRD_PARTY)/libjpeg/include \
  $(O3D_THIRD_PARTY)/libpng/include \

LOCAL_SRC_FILES := $(addprefix cross/gles2/, \
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

include $(O3D_BUILD_MODULE)


