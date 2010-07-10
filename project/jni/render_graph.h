// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef O3D_UTILS_RENDER_GRAPH_H_
#define O3D_UTILS_RENDER_GRAPH_H_

#include "core/cross/clear_buffer.h"
#include "core/cross/draw_context.h"
#include "core/cross/draw_list.h"
#include "core/cross/draw_pass.h"
#include "core/cross/pack.h"
#include "core/cross/state.h"
#include "core/cross/state_set.h"
#include "core/cross/tree_traversal.h"
#include "core/cross/transform.h"
#include "core/cross/viewport.h"

namespace o3d_utils {

// DrawPassInfo manages a draw pass.
class DrawPassInfo {
 public:
  DrawPassInfo();
  ~DrawPassInfo();

  /**
   * A class to manage a draw pass.
   * @constructor
   * @param {!o3d.Pack} pack Pack to manage created objects.
   * @param {!o3d.DrawContext} drawContext The DrawContext for this draw pass.
   * @param {o3d.DrawList.SortMethod} sortMethod How to sort this draw pass's
   *     DrawElements.
   * @param {!o3d.DrawList} opt_drawList The DrawList for this draw pass. If not
   *     passed in one will be created.
   * @param {!o3d.RenderNode} opt_parent The RenderNode to parent this draw pass
   *     under. If not passed the draw pass will not be parented.
   * @return {!o3djs.rendergraph.DrawPassInfo}
   */
  bool Initialize(
      o3d::Pack* pack,
      o3d::DrawList::SortMethod sort_method,
      o3d::RenderNode* parent,
      o3d::DrawList* draw_list);

  // The pack managing the objects created for this DrawPassInfo.
  o3d::Pack* pack() const {
    return pack_;
  }

  o3d::DrawContext* draw_context() const {
    return draw_context_;
  }

  // The State that affects all things drawn in this DrawPassInfo.
  o3d::State* state() const {
    return state_;
  }

  // The StateSet that applies the state for this DrawPassInfo.
  o3d::StateSet* state_set() const {
    return state_set_;
  }

  // The DrawPass for this DrawPassInfo.
  o3d::DrawPass* draw_pass() const {
    return draw_pass_;
  }

  // The DrawList for this DrawPassInfo.
  o3d::DrawList* draw_list() const {
    return draw_list_;
  }

  // The root RenderNode of this DrawPassInfo. This is the RenderNdoe you should
  // use if you want to turn this draw pass off or reparent it.
  o3d::RenderNode* root() const {
    return root_;
  }

 private:
  o3d::Pack* pack_;
  o3d::DrawContext* draw_context_;
  o3d::State* state_;
  o3d::StateSet* state_set_;
  o3d::DrawPass* draw_pass_;
  o3d::DrawList* draw_list_;
  o3d::RenderNode* root_;

  // A flag whether or not we created the DrawList for this DrawPassInfo.
  bool own_draw_list_;
};

// A ViewInfo object creates the standard o3d objects needed for
// a single 3d view. Those include a ClearBuffer followed by a TreeTraveral
// followed by 2 DrawPasses all of which are children of a Viewport. On top of
// those a DrawContext and optionally 2 DrawLists although you can pass in your
// own DrawLists if there is a reason to reuse the same DrawLists such was with
// mulitple views of the same scene.
//
// The render graph created is something like:
// <pre>
//        [Viewport]
//            |
//     +------+--------+------------------+---------------------+
//     |               |                  |                     |
// [ClearBuffer] [TreeTraversal] [Performance StateSet] [ZOrdered StateSet]
//                                        |                     |
//                               [Performance DrawPass] [ZOrdered DrawPass]
// </pre>
//
class ViewInfo {
 public:
  ViewInfo();
  ~ViewInfo();

  /**
   * @param {!o3d.Pack} pack Pack to manage created objects.
   * @param {!o3d.Transform} treeRoot root Transform of tree to render.
   * @param {!o3d.RenderNode} opt_parent RenderNode to build this view under.
   * @param {!o3djs.math.Vector4} opt_clearColor color to clear view.
   * @param {number} opt_priority Optional base priority for created objects.
   * @param {!o3djs.math.Vector4} opt_viewport viewport settings for view.
   * @param {!o3d.DrawList} opt_performanceDrawList DrawList to use for
   *     performanceDrawPass.
   * @param {!o3d.DrawList} opt_zOrderedDrawList DrawList to use for
   *     zOrderedDrawPass.
   * @param {!o3d.DrawContext} opt_drawContext Optional DrawContext to
   *     use. If not passed in one is created.
   */
  bool Initialize(
      o3d::Pack* pack,
      o3d::Transform* tree_root,
      o3d::RenderNode* parent,
      o3d::Float4* clear_color,
      float priority,
      o3d::Float4* viewport,
      o3d::DrawList* performance_draw_list,
      o3d::DrawList* z_ordered_draw_list,
      o3d::DrawContext* draw_context);

  /**
   * Creates a draw pass in this ViewInfo.
   *
   * @param {o3d.DrawList.SortMethod} sortMethod How to sort this draw pass's
   *     DrawElements.
   * @param {!o3d.DrawContext} opt_drawContext The DrawContext for this draw pass.
   *     If not passed in the default DrawContext for this ViewInfo will be used.
   * @param {number} opt_priority The priority for this draw pass. If not passed
   *     in the priority will be the next priority for this ViewInfo.
   * @param {!o3d.RenderNode} opt_parent The RenderNode to parent this draw pass
   *     under. If not passed in the draw pass will be parented under the
   *     ViewInfo's viewport RenderNode.
   * @param {!o3d.DrawList} opt_drawList The DrawList for this draw pass. If not
   *     passed in one will be created.
   * @return {!o3djs.rendergraph.DrawPassInfo}
   */
  DrawPassInfo* CreateDrawPass(
      o3d::DrawList::SortMethod sort_method,
      o3d::DrawContext* draw_context,
      float priority,
      o3d::RenderNode* parent,
      o3d::DrawList* draw_list);

  /**
   * Creates a basic render graph setup to draw opaque and transparent
   * 3d objects.
   * @param {!o3d.Pack} pack Pack to manage created objects.
   * @param {!o3d.Transform} treeRoot root Transform of tree to render.
   * @param {!o3d.RenderNode} opt_parent RenderNode to build this view under.
   * @return {!o3djs.rendergraph.ViewInfo} A ViewInfo object with info about
   *     everything created.
   */
  static ViewInfo* CreateBasicView(
      o3d::Pack* pack,
      o3d::Transform* tree_root,
      o3d::RenderNode* parent);

  const std::vector<DrawPassInfo*>& draw_pass_infos() const {
    return draw_pass_infos_;
  }

  // Pack that manages the objects created for this ViewInfo.
  o3d::Pack* pack() const {
    return pack_;
  }


  // The RenderNode this ViewInfo render graph subtree is parented under.
  o3d::RenderNode* render_graph_root() const {
    return render_graph_root_;
  }


  // The root node of the transform graph this ViewInfo renders.
  o3d::Transform* tree_root() const {
    return tree_root_;
  }


  // The root of the subtree of the render graph this ViewInfo is managing.
  o3d::RenderNode* root() const {
    return root_;
  }


  // The Viewport RenderNode created for this ViewInfo.
  o3d::Viewport* viewport() const {
    return viewport_;
  }


  // The ClearBuffer RenderNode created for this ViewInfo.
  o3d::ClearBuffer* clear_buffer() const {
    return clear_buffer_;
  }


  // The DrawContext used by this ViewInfo.
  o3d::DrawContext* draw_context() const {
    return draw_context_;
  }

  // The TreeTraversal used by this ViewInfo.
  o3d::TreeTraversal* tree_traversal() const {
    return tree_traversal_;
  }

  // The DrawPassInfo for the performance draw pass.
  DrawPassInfo* performance_draw_pass_info() const {
    return performance_draw_pass_info_;
  }

  // The DrawPassInfo for the zOrdered draw pass.
  DrawPassInfo* z_ordered_draw_pass_info() const {
    return z_ordered_draw_pass_info_;
  }

  // The highest priority used for objects under the Viewport RenderNode created
  // by this ViewInfo.
  float priority() const {
    return priority_;
  }

 private:
  std::vector<DrawPassInfo*> draw_pass_infos_;
  o3d::Pack* pack_;
  o3d::RenderNode* render_graph_root_;
  o3d::Transform* tree_root_;
  o3d::RenderNode* root_;
  o3d::Viewport* viewport_;
  o3d::ClearBuffer* clear_buffer_;
  o3d::DrawContext* draw_context_;
  o3d::TreeTraversal* tree_traversal_;
  DrawPassInfo* performance_draw_pass_info_;
  DrawPassInfo* z_ordered_draw_pass_info_;

  float priority_;

  // A flag whether or not we created the DrawContext for this DrawPassInfo.
  bool own_draw_context_;
};

}  // namespace o3d_utils

#endif  // O3D_UTILS_RENDER_GRAPH_H_

