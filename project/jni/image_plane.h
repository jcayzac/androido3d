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

#ifndef O3D_UTILS_IMAGE_PLANE_H_
#define O3D_UTILS_IMAGE_PLANE_H_

#include <string>
#include "core/cross/types.h"
#include "core/cross/param.h"

namespace o3d {

	class Pack;
	class ParamSampler;
	class Sampler;
	class Shape;
	class Transform;
	class Texture2D;

}  // namespace o3d.

namespace o3d_utils {

	class ViewInfo;

// An image plane is a Quad with an image mapped to it.
	class ImagePlane {
	public:
		// Creates an ImagePlane. If the requested texture is not already loaded,
		// loads it.
		static ImagePlane* Create(o3d::Pack* plane_pack,
		                          o3d::Pack* texture_pack,
		                          ViewInfo* view_info,
		                          const std::string& filename,
		                          bool origin_at_center);

		// Gets a texture from the pack. If it doesn't exist, loads it.
		static o3d::Texture2D* GetTexture(
		    o3d::Pack* pack, const std::string& filename);

		// Gets an image plane shape with associated effect and material. If
		// they don't exist in the given pack they will be created.
		static o3d::Shape* GetImagePlaneShape(
		    o3d::Pack* pack, ViewInfo* view_info);

		o3d::Transform* transform() const {
			return transform_;
		}

		void SetColorMult(const o3d::Float4& color) {
			color_mult_param_->set_value(color);
		}
		void SetColorMult(const float* color) {
			SetColorMult(o3d::Float4(color[0], color[1], color[2], color[3]));
		}

		o3d::Float4 GetColorMult() const {
			return color_mult_param_->value();
		}

	private:
		ImagePlane();

		bool Init(
		    o3d::Texture2D* texture,
		    o3d::Pack* plane_pack,
		    ViewInfo* view_info,
		    const std::string& filename,
		    bool origin_at_center);

		o3d::Sampler* sampler_;
		o3d::ParamSampler* sampler_param_;
		o3d::ParamFloat4* color_mult_param_;
		o3d::Transform* transform_;
		o3d::Transform* scale_transform_;
	};

}  // namespace o3d_utils

#endif  // O3D_UTILS_IMAGE_PLANE_H_

