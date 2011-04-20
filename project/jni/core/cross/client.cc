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


// This file contains the implementation for the Client class.

#include "core/cross/client.h"
#include "core/cross/draw_context.h"
#include "core/cross/effect.h"
#include "core/cross/pack.h"
#include "core/cross/shape.h"
#include "core/cross/transform.h"
#include "core/cross/material.h"
#include "core/cross/renderer.h"
#include "core/cross/viewport.h"
#include "core/cross/clear_buffer.h"
#include "core/cross/state_set.h"
#include "core/cross/tree_traversal.h"
#include "core/cross/draw_pass.h"
#include "core/cross/bounding_box.h"
#include "core/cross/bitmap.h"
#include "core/cross/error.h"
#include "core/cross/evaluation_counter.h"
#include "core/cross/id_manager.h"
#include "core/cross/profiler.h"
#include "utils/cross/dataurl.h"

using std::map;
using std::vector;
using std::make_pair;

namespace o3d {

// Client constructor.  Creates the default root node for the scenegraph
Client::Client(ServiceLocator* service_locator)
    : service_locator_(service_locator),
      object_manager_(service_locator),
      error_status_(service_locator),
      draw_list_manager_(service_locator),
      counter_manager_(service_locator),
      transformation_context_(service_locator),
      picking_context_(service_locator),
      semantic_manager_(service_locator),
      profiler_(service_locator),
      renderer_(service_locator),
      evaluation_counter_(service_locator),
      render_tree_called_(false),
      render_mode_(RENDERMODE_CONTINUOUS),
      texture_on_hold_(false),
      event_manager_(),
      last_tick_time_(0),
      root_(NULL),
      rendergraph_root_(NULL),
      id_(IdManager::CreateId()) {
}

// Frees up all the resources allocated by the Client factory methods but
// does not destroy the "renderer_" object.
Client::~Client() {
  root_.Reset();
  rendergraph_root_.Reset();

  object_manager_->DestroyAllPacks();

  // Unmap the client from the renderer on exit.
  if (renderer_.IsAvailable()) {
    renderer_->UninitCommon();
  }
}

// Assigns a Renderer to the Client, and also assigns the Client
// to the Renderer and sets up the default render graph
void Client::Init() {
  if (!renderer_.IsAvailable())
    return;

  // Create the root node for the scenegraph.  Note that the root lives
  // outside of a pack object.  The root's lifetime is directly bound to that
  // of the client.
  root_ = Transform::Ref(new Transform(service_locator_));
  root_->set_name(O3D_STRING_CONSTANT("root"));

  // Creates the root for the render graph.
  rendergraph_root_ = RenderNode::Ref(new RenderNode(service_locator_));
  rendergraph_root_->set_name(O3D_STRING_CONSTANT("root"));

  // Let the renderer Init a few common things.
  renderer_->InitCommon();
}

void Client::Cleanup() {
  ClearRenderCallback();
  ClearPostRenderCallback();
  ClearTickCallback();
  event_manager_.ClearAll();
  counter_manager_.ClearAllCallbacks();

  // Disable continuous rendering because there is nothing to render after
  // Cleanup is called. This speeds up the the process of unloading the page.
  // It also preserves the last rendered frame so long as it does not become
  // invalid.
  render_mode_ = RENDERMODE_ON_DEMAND;

  // Destroy the packs here if possible. If there are a lot of objects it takes
  // a long time and seems to make Chrome timeout if it happens in NPP_Destroy.
  root_.Reset();
  rendergraph_root_.Reset();
  object_manager_->DestroyAllPacks();
}

Pack* Client::CreatePack() {
  if (!renderer_.IsAvailable()) {
    O3D_ERROR(service_locator_)
        << "No Renderer available, Pack creation not allowed.";
    return NULL;
  }

  return object_manager_->CreatePack();
}

// Tick Methods ----------------------------------------------------------------

void Client::SetTickCallback(
    TickCallback* tick_callback) {
  tick_callback_manager_.Set(tick_callback);
}

void Client::ClearTickCallback() {
  tick_callback_manager_.Clear();
}

bool Client::Tick() {
  ElapsedTimeTimer timer;
  float seconds_elapsed = tick_elapsed_time_timer_.GetElapsedTimeAndReset();
  tick_event_.set_elapsed_time(seconds_elapsed);
  profiler_->ProfileStart("Tick callback");
  tick_callback_manager_.Run(tick_event_);
  profiler_->ProfileStop("Tick callback");

  evaluation_counter_->InvalidateAllParameters();

  counter_manager_.AdvanceCounters(1.0f, seconds_elapsed);

  bool message_check_ok = true;
  bool has_new_texture = false;

  event_manager_.ProcessQueue();
  event_manager_.ProcessQueue();
  event_manager_.ProcessQueue();
  event_manager_.ProcessQueue();

  last_tick_time_ = timer.GetElapsedTimeAndReset();

  texture_on_hold_ |= has_new_texture;
  int max_fps = renderer_->max_fps();
  if (max_fps > 0 &&
      texture_on_hold_ &&
      render_mode() == RENDERMODE_ON_DEMAND &&
      render_elapsed_time_timer_.GetElapsedTimeWithoutClearing()
        > 1.0/max_fps) {
    renderer_->set_need_to_render(true);
    texture_on_hold_ = false;
  }

  return message_check_ok;
}

// Render Methods --------------------------------------------------------------

void Client::SetLostResourcesCallback(LostResourcesCallback* callback) {
  if (!renderer_.IsAvailable()) {
    O3D_ERROR(service_locator_) << "No Renderer";
  } else {
    renderer_->SetLostResourcesCallback(callback);
  }
}

void Client::ClearLostResourcesCallback() {
  if (renderer_.IsAvailable()) {
    renderer_->ClearLostResourcesCallback();
  }
}

void Client::RenderClientInner(bool present, bool send_callback) {
  ElapsedTimeTimer timer;
  render_tree_called_ = false;
  total_time_to_render_ = 0.0f;

  if (!renderer_.IsAvailable())
    return;

  if (renderer_->StartRendering()) {
    counter_manager_.AdvanceRenderFrameCounters(1.0f);

    profiler_->ProfileStart("Render callback");
    if (send_callback)
      render_callback_manager_.Run(render_event_);
    profiler_->ProfileStop("Render callback");

    if (!render_tree_called_) {
      RenderNode* rendergraph_root = render_graph_root();
      // If nothing was rendered and there are no render graph nodes then
      // clear the client area.
      if (!rendergraph_root || rendergraph_root->children().empty()) {
          renderer_->Clear(Float4(0.4f, 0.3f, 0.3f, 1.0f),
                           true, 1.0f, true, 0, true);
      } else if (rendergraph_root) {
        RenderTree(rendergraph_root);
      }
    }

    renderer_->FinishRendering();
    if (present) {
      renderer_->Present();
      // This has to be called before the POST render callback because
      // the post render callback may call Client::Render.
      renderer_->set_need_to_render(false);
    }

    // Call post render callback.
    profiler_->ProfileStart("Post-render callback");
    post_render_callback_manager_.Run(render_event_);
    profiler_->ProfileStop("Post-render callback");

    // Update Render stats.
    render_event_.set_elapsed_time(
        render_elapsed_time_timer_.GetElapsedTimeAndReset());
    render_event_.set_render_time(total_time_to_render_);
    render_event_.set_transforms_culled(renderer_->transforms_culled());
    render_event_.set_transforms_processed(renderer_->transforms_processed());
    render_event_.set_draw_elements_culled(renderer_->draw_elements_culled());
    render_event_.set_draw_elements_processed(
        renderer_->draw_elements_processed());
    render_event_.set_draw_elements_rendered(
        renderer_->draw_elements_rendered());

    render_event_.set_primitives_rendered(renderer_->primitives_rendered());

    render_event_.set_active_time(
        timer.GetElapsedTimeAndReset() + last_tick_time_);
    last_tick_time_ = 0.0f;
  }
}

void Client::RenderClient(bool send_callback) {
  if (!renderer_.IsAvailable())
    return;

  bool have_offscreen_surfaces =
      !(offscreen_render_surface_.IsNull() ||
        offscreen_depth_render_surface_.IsNull());

  if (have_offscreen_surfaces) {
    if (!renderer_->StartRendering()) {
      return;
    }
    renderer_->SetRenderSurfaces(offscreen_render_surface_,
                                 offscreen_depth_render_surface_,
                                 true);
  }

  RenderClientInner(!have_offscreen_surfaces, send_callback);

  if (have_offscreen_surfaces) {
    renderer_->SetRenderSurfaces(NULL, NULL, false);
    renderer_->FinishRendering();
  }
}

ParamObject* Client::Pick(int window_x, int window_y) {
	if (!render_graph_root()) return 0;
	if (render_graph_root()->children().empty()) return 0;
	if (!renderer_.IsAvailable()) return 0;

	// We always need to call StartRendering()/FinishRendering()
	// before and after rendering stuff, but if we have rendering
	// surfaces we also need to wrap their rendering around one
	// set of StartRendering()/FinishRendering(). Thus the code
	// becomes:
	//
	// No offscreen surfaces:
	//     StartRendering()
	//         ...render...
	//     FinishRendering()
	//
	// Offscreen surfaces:
	//     StartRendering()
	//         SetRenderSurfaces(...surfaces...)
	//             StartRendering()
	//                 ...render...
	//             FinishRendering()
	//         SetRenderSurfaces(...NULL...)
	//     FinishRendering()

	const bool have_offscreen_surfaces(!(offscreen_render_surface_.IsNull() || offscreen_depth_render_surface_.IsNull()));
	if (have_offscreen_surfaces) {
		if (!renderer_->StartRendering()) return 0;
		renderer_->SetRenderSurfaces(offscreen_render_surface_, offscreen_depth_render_surface_, true);
	}

	if (!renderer_->StartRendering()) return 0;

	ParamObject* result = 0;
	if (renderer_->BeginDraw()) {
		RenderContext render_context(renderer_.Get());
		renderer_->StartPicking(window_x, window_y);
		render_graph_root()->RenderTree(&render_context);
		draw_list_manager_.Reset();
		result = renderer_->FinishPicking();
		renderer_->EndDraw();
	}

	renderer_->FinishRendering();

	if (have_offscreen_surfaces) {
		renderer_->SetRenderSurfaces(NULL, NULL, false);
		renderer_->FinishRendering();
	}

	return result;
}

// Executes draw calls for all visible shapes in a subtree
void Client::RenderTree(RenderNode *tree_root) {
  if (!renderer_.IsAvailable())
    return;

  if (!renderer_->rendering()) {
    // Render tree can not be called if we are not rendering because all calls
    // to RenderTree must happen inside renderer->StartRendering() /
    // renderer->FinishRendering() calls.
    O3D_ERROR(service_locator_)
        << "RenderTree must not be called outside of rendering.";
    return;
  }

  render_tree_called_ = true;

  // Only render the shapes if BeginDraw() succeeds
  profiler_->ProfileStart("RenderTree");
  ElapsedTimeTimer time_to_render_timer;
  if (renderer_->BeginDraw()) {
    RenderContext render_context(renderer_.Get());

    if (tree_root) {
      tree_root->RenderTree(&render_context);
    }

    draw_list_manager_.Reset();

    // Finish up.
    renderer_->EndDraw();
  }
  total_time_to_render_ += time_to_render_timer.GetElapsedTimeAndReset();
  profiler_->ProfileStop("RenderTree");
}

void Client::SetRenderCallback(RenderCallback* render_callback) {
  render_callback_manager_.Set(render_callback);
}

void Client::ClearRenderCallback() {
  render_callback_manager_.Clear();
}

void Client::SetEventCallback(Event::Type type,
                              EventCallback* event_callback) {
  event_manager_.SetEventCallback(type, event_callback);
}

void Client::SetEventCallback(std::string type_name,
                              EventCallback* event_callback) {
  Event::Type type = Event::TypeFromString(type_name.c_str());
  if (!Event::ValidType(type)) {
    O3D_ERROR(service_locator_) << "Invalid event type: '" <<
        type_name << "'.";
  } else {
    event_manager_.SetEventCallback(type, event_callback);
  }
}

void Client::ClearEventCallback(Event::Type type) {
  event_manager_.ClearEventCallback(type);
}

void Client::ClearEventCallback(std::string type_name) {
  Event::Type type = Event::TypeFromString(type_name.c_str());
  if (!Event::ValidType(type)) {
    O3D_ERROR(service_locator_) << "Invalid event type: '" <<
        type_name << "'.";
  } else {
    event_manager_.ClearEventCallback(type);
  }
}

void Client::AddEventToQueue(const Event& event) {
  event_manager_.AddEventToQueue(event);
}

void Client::SendResizeEvent(int width, int height, bool fullscreen) {
  Event event(Event::TYPE_RESIZE);
  event.set_size(width, height, fullscreen);
  AddEventToQueue(event);
}

void Client::set_render_mode(RenderMode render_mode) {
  render_mode_ = render_mode;
}

void Client::SetPostRenderCallback(RenderCallback* post_render_callback) {
  post_render_callback_manager_.Set(post_render_callback);
}

void Client::ClearPostRenderCallback() {
  post_render_callback_manager_.Clear();
}

void Client::Render() {
  if (render_mode() == RENDERMODE_ON_DEMAND) {
    if (renderer_.IsAvailable()) {
      renderer_->set_need_to_render(true);
    }
  }
}

void Client::SetErrorTexture(Texture* texture) {
  renderer_->SetErrorTexture(texture);
}

void Client::InvalidateAllParameters() {
  evaluation_counter_->InvalidateAllParameters();
}

std::string Client::GetScreenshotAsDataURL()  {
  // To take a screenshot we create a render target and render into it
  // then get a bitmap from that.
  int pot_width =
      static_cast<int>(image::ComputePOTSize(renderer_->display_width()));
  int pot_height =
      static_cast<int>(image::ComputePOTSize(renderer_->display_height()));
  if (pot_width == 0 || pot_height == 0) {
    return dataurl::kEmptyDataURL;
  }
  Texture2D::Ref texture = renderer_->CreateTexture2D(
      pot_width,
      pot_height,
      Texture::ARGB8,
      1,
      true);
  if (texture.IsNull()) {
    return dataurl::kEmptyDataURL;
  }
  RenderSurface::Ref surface(texture->GetRenderSurface(0));
  if (surface.IsNull()) {
    return dataurl::kEmptyDataURL;
  }
  RenderDepthStencilSurface::Ref depth(renderer_->CreateDepthStencilSurface(
      pot_width,
      pot_height));
  if (depth.IsNull()) {
    return dataurl::kEmptyDataURL;
  }
  surface->SetClipSize(renderer_->display_width(), renderer_->display_height());
  depth->SetClipSize(renderer_->display_width(), renderer_->display_height());

  const RenderSurface* old_render_surface_;
  const RenderDepthStencilSurface* old_depth_surface_;
  bool is_back_buffer;

  renderer_->GetRenderSurfaces(&old_render_surface_, &old_depth_surface_,
                               &is_back_buffer);
  renderer_->SetRenderSurfaces(surface, depth, true);

  RenderClientInner(false, true);

  renderer_->SetRenderSurfaces(old_render_surface_, old_depth_surface_,
                               is_back_buffer);

  Bitmap::Ref bitmap(surface->GetBitmap());
  if (bitmap.IsNull()) {
    return dataurl::kEmptyDataURL;
  } else {
    bitmap->FlipVertically();
    return bitmap->ToDataURL();
  }
}

std::string Client::ToDataURL() {
  if (!renderer_.IsAvailable()) {
    O3D_ERROR(service_locator_) << "No Render Device Available";
    return dataurl::kEmptyDataURL;
  }

  if (renderer_->rendering()) {
    O3D_ERROR(service_locator_)
       << "Can not take a screenshot while rendering";
    return dataurl::kEmptyDataURL;
  }

  if (!renderer_->StartRendering()) {
    return dataurl::kEmptyDataURL;
  }

  std::string data_url(GetScreenshotAsDataURL());
  renderer_->FinishRendering();

  return data_url;
}

void Client::SetOffscreenRenderingSurfaces(
    RenderSurface::Ref surface,
    RenderDepthStencilSurface::Ref depth_surface) {
  offscreen_render_surface_ = surface;
  offscreen_depth_render_surface_ = depth_surface;
}

// Error Related methods -------------------------------------------------------

void Client::SetErrorCallback(ErrorCallback* callback) {
  error_status_.SetErrorCallback(callback);
}

void Client::ClearErrorCallback() {
  error_status_.ClearErrorCallback();
}

const std::string& Client::GetLastError() const {
  return error_status_.GetLastError();
}

void Client::ClearLastError() {
  error_status_.ClearLastError();
}

void Client::ProfileStart(const std::string& key) {
  profiler_->ProfileStart(key);
}

void Client::ProfileStop(const std::string& key) {
  profiler_->ProfileStop(key);
}

void Client::ProfileReset() {
  profiler_->ProfileReset();
}
}  // namespace o3d
