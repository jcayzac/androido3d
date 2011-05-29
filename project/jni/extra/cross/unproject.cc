/*
 * Copyright (C) 2010 Tonchidot Corporation.
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

#include "extra/cross/unproject.h"
#include "render_graph.h"
#include "core/cross/renderer.h"

o3d::Point3 o3d_extra::unprojectPoint(
	const o3d_utils::ViewInfo& view,
	const o3d::Renderer& renderer,
	const o3d::Point3& screenSpaceCoordinates
)
{
	// Size of the client window, in pixels
	const int clientWidth(renderer.width());
	const int clientHeight(renderer.height());

	// Size of the viewport, in pixels
	const o3d::Float4 viewport(view.viewport()->viewport());
	const float viewLeft(viewport[0] * (float) clientWidth);
	const float viewTop(viewport[1] * (float) clientHeight);
	const float viewWidth(viewport[2] * (float) clientWidth);
	const float viewHeight(viewport[3] * (float) clientHeight);

	// Drawing context
	const o3d::DrawContext& ctx(*view.draw_context());

	// screen>world matrix
	const o3d::Matrix4 screenToWorld(inverse(ctx.projection()*ctx.view()));

	// unified device coordinates
	const o3d::Point3 deviceCoordinates((screenSpaceCoordinates.getX() - viewLeft)/(viewWidth*.5f) - 1.f,
										1.f - (screenSpaceCoordinates.getY() - viewTop)/(viewHeight*.5f),
										screenSpaceCoordinates.getZ());

	// unprojected point
	const o3d::Vector4 unproj(screenToWorld*deviceCoordinates);

	// return point with homogenous coordinates
	return o3d::Point3(unproj.getXYZ()*(1.f/unproj.getW()));
}

