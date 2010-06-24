// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "render_graph.h"

namespace o3d_utils {

using o3d::ClearBuffer;
using o3d::DrawContext;
using o3d::DrawList;
using o3d::DrawPass;
using o3d::Pack;
using o3d::ParamBoolean;
using o3d::ParamInteger;
using o3d::RenderNode;
using o3d::State;
using o3d::StateSet;
using o3d::TreeTraversal;
using o3d::Transform;
using o3d::Viewport;
using o3d::Float4;

DrawPassInfo::DrawPassInfo()
    : own_draw_list_(false) {
}

DrawPassInfo::~DrawPassInfo() {
  pack_->RemoveObject(state_set_);
  pack_->RemoveObject(state_);
  pack_->RemoveObject(draw_pass_);
  if (own_draw_list_) {
    pack_->RemoveObject(draw_list_);
  }
  root_->SetParent(NULL);
}

bool DrawPassInfo::Initialize(
    Pack* pack,
    DrawList::SortMethod sort_method,
    RenderNode* parent,
    DrawList* draw_list) {
  DCHECK(pack);
  own_draw_list_ = draw_list == NULL;
  draw_list_ = draw_list ? draw_list : pack->Create<DrawList>();

  state_set_ = pack->Create<StateSet>();
  state_ = pack->Create<State>();
  state_set_->set_state(state_);
  state_set_->SetParent(parent);

  draw_pass_ = pack->Create<DrawPass>();
  draw_pass_->set_draw_list(draw_list_);
  draw_pass_->set_sort_method(sort_method);
  draw_pass_->SetParent(state_set_);

  pack_ = pack;
  root_ = state_set_;
};

ViewInfo::ViewInfo()
    : own_draw_context_(false) {
}

ViewInfo::~ViewInfo() {
  pack_->RemoveObject(viewport_);
  pack_->RemoveObject(clear_buffer_);
  pack_->RemoveObject(tree_traversal_);
  if (own_draw_context_) {
    pack_->RemoveObject(draw_context_);
  }
  for (size_t ii = 0; ii < draw_pass_infos_.size(); ++ii) {
    delete draw_pass_infos_[ii];
  }
}

bool ViewInfo::Initialize(
    Pack* pack,
    Transform* tree_root,
    RenderNode* parent,
    Float4* _clear_color,
    float priority,
    Float4* viewport,
    DrawList* performance_draw_list,
    DrawList* z_ordered_draw_list,
    DrawContext* draw_context) {
  DCHECK(pack);
  DCHECK(tree_root);

  Float4 clear_color =
      _clear_color ? *_clear_color : Float4(0.5f, 0.5f, 0.5f, 1.0f);

  // Create Viewport
  viewport_ = pack->Create<Viewport>();
  if (viewport) {
    viewport_->set_viewport(*viewport);
  }

  // Create a clear buffer.
  clear_buffer_ = pack->Create<ClearBuffer>();
  clear_buffer_->set_clear_color(clear_color);
  clear_buffer_->set_priority(priority++);
  clear_buffer_->SetParent(viewport_);

  // Creates a TreeTraversal and parents it to the root.
  tree_traversal_ = pack->Create<TreeTraversal>();
  tree_traversal_->set_priority(priority++);
  tree_traversal_->SetParent(viewport_);
  tree_traversal_->set_transform(tree_root);

  pack_ = pack;
  render_graph_root_ = parent;
  tree_root_ = tree_root;
  root_ = viewport_;
  draw_context_ = draw_context ? draw_context : pack->Create<DrawContext>();
  priority_ = priority;

  // Setup a Performance Ordered DrawPass
  performance_draw_pass_info_ = CreateDrawPass(
      DrawList::BY_PERFORMANCE,
      NULL, priority_++, NULL, performance_draw_list);

  // Setup a z Ordered DrawPass
  z_ordered_draw_pass_info_ = CreateDrawPass(
      DrawList::BY_Z_ORDER,
      NULL, priority_++, NULL, z_ordered_draw_list);

  draw_pass_infos_.push_back(performance_draw_pass_info_);
  draw_pass_infos_.push_back(z_ordered_draw_pass_info_);

  State* z_ordered_state = z_ordered_draw_pass_info_->state();

  z_ordered_state->GetStateParam<ParamBoolean>(
      State::kAlphaBlendEnableParamName)->set_value(true);
  z_ordered_state->GetStateParam<ParamInteger>(
      State::kSourceBlendFunctionParamName)->set_value(
          State::BLENDFUNC_SOURCE_ALPHA);
  z_ordered_state->GetStateParam<ParamInteger>(
      State::kDestinationBlendFunctionParamName)->set_value(
          State::BLENDFUNC_INVERSE_SOURCE_ALPHA);
  z_ordered_state->GetStateParam<ParamBoolean>(
      State::kAlphaTestEnableParamName)->set_value(true);
  z_ordered_state->GetStateParam<ParamInteger>(
      State::kAlphaComparisonFunctionParamName)->set_value(State::CMP_GREATER);

  // Parent whatever the root is to the parent passed in.
  root_->SetParent(parent);

  own_draw_context_ = draw_context == NULL;
};

DrawPassInfo* ViewInfo::CreateDrawPass(
    DrawList::SortMethod sort_method,
    DrawContext* draw_context,
    float priority,
    RenderNode* parent,
    DrawList* draw_list) {
  draw_context = draw_context ? draw_context : draw_context_;
  parent = parent ? parent : viewport_;
  DrawPassInfo* draw_pass_info = new DrawPassInfo();
  if (!draw_pass_info->Initialize(
      pack_,
      sort_method,
      parent,
      draw_list)) {
    delete draw_pass_info;
    return NULL;
  }
  draw_pass_info->root()->set_priority(priority);
  tree_traversal_->RegisterDrawList(
      draw_pass_info->draw_list(), draw_context, true);
  return draw_pass_info;
}

ViewInfo* ViewInfo::CreateBasicView(
      o3d::Pack* pack,
      o3d::Transform* tree_root,
      o3d::RenderNode* parent) {
  ViewInfo* view_info = new ViewInfo();
  if (!view_info->Initialize(
      pack, tree_root, parent, NULL, 0.0f, NULL, NULL, NULL, NULL)) {
    delete view_info;
    return NULL;
  }
  return view_info;
}

}  // namespace o3d_utils

