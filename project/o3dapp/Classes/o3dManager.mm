/*
 *  o3dManager.mm
 *  o3dapp
 *
 *  Created by Chris Wynn on 10/8/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "o3dManager.h"
#include "core/cross/renderer_platform.h"

O3DManager::O3DManager()
: root_(NULL),
hud_root_(NULL),
pack_(NULL),
main_view_(NULL),
hud_view_(NULL),
scene_(NULL),
time_(0.0f) {
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
	effect_texture_pack_ = client_->CreatePack();
	root_ = pack_->Create<o3d::Transform>();
	main_view_ = o3d_utils::ViewInfo::CreateBasicView(
													  pack_, root_, client_->render_graph_root());
	
	// Create a second view for the hud.
	hud_root_ = pack_->Create<o3d::Transform>();
	hud_view_ = o3d_utils::ViewInfo::CreateBasicView(
													 pack_, hud_root_, client_->render_graph_root());
	
	// Make sure the hud gets drawn after the 3d stuff
	hud_view_->root()->set_priority(main_view_->root()->priority() + 1);
	
	// Turn off clearing the color for the hud since that would erase the 3d
	// parts but leave clearing the depth and stencil so the HUD is unaffected
	// by anything done by the 3d parts.
	hud_view_->clear_buffer()->set_clear_color_flag(false);
	
	// Set culling to none so we can flip images using rotation or negative scale.
	o3d::State* state = hud_view_->z_ordered_draw_pass_info()->state();
	state->GetStateParam<o3d::ParamInteger>(
											o3d::State::kCullModeParamName)->set_value(o3d::State::CULL_NONE);
	state->GetStateParam<o3d::ParamBoolean>(
											o3d::State::kZWriteEnableParamName)->set_value(false);
	
	hud_view_->draw_context()->set_view(o3d::Matrix4::lookAt(
															 o3d::Point3(0.0f, 0.0f, 30.0f),
															 o3d::Point3(0.0f, 0.0f, 0.0f),
															 o3d::Vector3(0.0f, 1.0f, 0.0f)));
	
	for (size_t ii = 0; ii < arraysize(images_); ++ii) {
		const ImgInfo& info = imgs[ii];
		const char *filename;
		GET_IMAGE_RESOURCE_FILENAME( filename, info.filename );
		images_[ii] = o3d_utils::ImagePlane::Create(
													pack_, pack_, hud_view_, filename, info.center);
		images_[ii]->transform()->SetParent(hud_root_);
		images_[ii]->transform()->set_local_matrix(o3d::Matrix4::translation(
																			 o3d::Vector3(info.x, info.y, info.depth)));
	}
	
	images_[0]->transform()->GetChildrenRefs()[0]->GetShapeRefs()[0]->
	GetElementRefs()[0]->set_cull(false);
	
	const char *filename;
	GET_COLLADA_RESOURCE_FILENAME( filename, "character.zip" );
	scene_ = o3d_utils::Scene::LoadScene(
										 client_.get(),
										 main_view_,
										 filename,
										 effect_texture_pack_);
	scene_->SetParent(root_);
	
	o3d_utils::CameraInfo* camera_info =
	o3d_utils::Camera::getViewAndProjectionFromCameras(
													   root_, width, height);
	main_view_->draw_context()->set_view(camera_info->view);
	
	SetProjection(width, height);
	
	// Set the light pos on all the materials. We could link a param to
	// all of them so we could just set the one param but I'm lazy.
	o3d::Vector3 light_pos =
	Vectormath::Aos::inverse(camera_info->view).getTranslation();
	std::vector<o3d::Pack*> packs;
	packs.push_back(scene_->pack());
	for (size_t ii = 0; ii < packs.size(); ++ii) {
		std::vector<o3d::Material*> materials =
        packs[ii]->GetByClass<o3d::Material>();
		for (size_t ii = 0; ii < materials.size(); ++ii) {
			o3d::ParamFloat3* pos =
			materials[ii]->GetParam<o3d::ParamFloat3>("lightWorldPos");
			if (pos) {
				pos->set_value(o3d::Float3(light_pos[0], light_pos[1], light_pos[2]));
			}
		}
	}
	
	timer_.GetElapsedTimeAndReset();
	
	CheckError();
	
	return true;
}

bool O3DManager::OnContextRestored() {
	// Reset the timer so we don't have some giant time slice.
	timer_.GetElapsedTimeAndReset();
	
	// Restore the resources.
	return down_cast<o3d::RendererGLES2*>(renderer_)->OnContextRestored();
}

void O3DManager::SetProjection(float width, float height) {
	main_view_->draw_context()->set_projection(
											   o3d_utils::Camera::perspective(
																			  o3d_utils::degToRad(30.0f), width / height, 10, 1000));
	
	hud_view_->draw_context()->set_projection(
											  o3d::Matrix4::orthographic(
																		 0,
																		 width,
																		 height,
																		 0,
																		 0.001,
																		 1000));
}

bool O3DManager::ResizeViewport(int width, int height) {
	renderer_->Resize(width, height);
	
	o3d_utils::CameraInfo* camera_info =
	o3d_utils::Camera::getViewAndProjectionFromCameras(
													   root_, width, height);
	
	main_view_->draw_context()->set_view(camera_info->view);
	SetProjection(width, height);

	return true;
}

o3d::Matrix4 lookAt2(
					 const o3d::Point3& eye,
					 const o3d::Point3& target,
					 const o3d::Vector3& up) {
	o3d::Vector4 vz(Vectormath::Aos::normalize(eye - target));
	o3d::Vector4 vx(Vectormath::Aos::cross(up, vz.getXYZ()));
	o3d::Vector4 vy(Vectormath::Aos::cross(vz.getXYZ(), vx.getXYZ()));
	return o3d::Matrix4(vx, vy, vz, o3d::Vector4(eye));
}

bool O3DManager::Render() {
	float elapsedTimeSinceLastUpdateInSeconds = timer_.GetElapsedTimeAndReset();
	time_ += elapsedTimeSinceLastUpdateInSeconds;
	
	// Data for animations in 30hz frames. O3D uses seconds so divide by 30.
	//
	// idle1: {startFrame: 0, endFrame: 30},
	// walk: {startFrame: 31, endFrame: 71},
	// jumpStart: {startFrame: 72, endFrame: 87},
	// jumpUp: {startFrame: 87, endFrame: 87},
	// jumpCrest: {startFrame: 87, endFrame: 91},
	// jumpFall: {startFrame: 91, endFrame: 91},
	// jumpLand: {startFrame: 91, endFrame: 110},
	// run: {startFrame: 111, endFrame: 127},
	// idle2: {startFrame: 128, endFrame: 173},
	// idle3: {startFrame: 174, endFrame: 246},
	// idle4: {startFrame: 247, endFrame: 573}};
	static const float kWalkStart = 31.0f / 30.0f;
	static const float kWalkEnd = 71.0f / 30.0f;
	static const float kWalkDuration = kWalkEnd - kWalkStart;
	static const float kIdleStart = 247.0f / 30.0f;
	static const float kIdleEnd = 573.0f / 30.0f;
	static const float kIdleDuration = kIdleEnd - kIdleStart;
	
	static float moveTimer = 0.0f;
	static float animTimer = 0.0f;
	static bool idleing = false;
	
	animTimer += elapsedTimeSinceLastUpdateInSeconds;
	if (!idleing) {
		moveTimer += elapsedTimeSinceLastUpdateInSeconds * 0.5;
		
		// Move and animate the character
		// Move in a circle
		const float kMoveRadius = 10.0f;
		o3d::Point3 position(
							 sinf(moveTimer) * kMoveRadius,
							 0.0f,
							 cosf(moveTimer) * kMoveRadius);
		o3d::Point3 target(
						   sinf(moveTimer - 0.1f) * kMoveRadius,
						   0.0f,
						   cosf(moveTimer - 0.1f) * kMoveRadius);
		o3d::Vector3 up(0.0f, 1.0f, 0.0f);
		
		o3d::Matrix4 mat(o3d::Matrix4::lookAt(position, target, up));
		scene_->root()->set_local_matrix(mat);
		
		scene_->SetAnimationTime(kWalkStart + fmodf(animTimer, kWalkDuration));
		if (animTimer >= kWalkDuration * 4) {
			animTimer = 0.0f;
			idleing = true;
		}
	} else {
		scene_->SetAnimationTime(kIdleStart + std::min(animTimer, kIdleDuration));
		if (animTimer >= kIdleDuration) {
			animTimer = 0.0f;
			idleing = false;
		}
	}
	
	// Move the meter in the gauge.
	images_[2]->SetColorMult(o3d::Float4(
										 (sinf(time_ * 4.0f) + 1.0f) * 0.5f, 0.0f, 0.0f, 1.0f));
	images_[2]->transform()->set_local_matrix(
											  o3d::Matrix4::translation(
																		o3d::Vector3(imgs[2].x, imgs[2].y, imgs[2].depth)) *
											  o3d::Matrix4::scale(o3d::Vector3(fmodf(time_ / 2, 1.0f) * 384, 80, 1)));
	
	// Spin the egg.
	images_[0]->transform()->set_local_matrix(
											  o3d::Matrix4::translation(
																		o3d::Vector3(imgs[0].x, imgs[0].y, imgs[0].depth)) *
											  o3d::Matrix4::rotationZ(time_));
	
	// Fade the radar in/out.
	images_[3]->SetColorMult(
							 o3d::Float4(1.0f, 1.0f, 1.0f, (sinf(time_ * 2) + 1) * 0.5));
	
	client_->Tick();
	client_->RenderClient(true);
	CheckError();
	
	//{
	//  static bool once = false;
	//  if (once) {
	//    return true;
	//  }
	//  once = true;
	//}
	//LOGI("transforms processed   : %d\n", renderer_->transforms_processed());
	//LOGI("transforms culled      : %d\n", renderer_->transforms_culled());
	//LOGI("draw elements processed: %d\n", renderer_->draw_elements_processed());
	//LOGI("draw elements culled   : %d\n", renderer_->draw_elements_culled());
	//LOGI("draw elements rendered : %d\n", renderer_->draw_elements_rendered());
	//LOGI("primtives_rendered     : %d\n", renderer_->primitives_rendered());
	
	return true;
}

o3d::Transform* O3DManager::GetRoot() {
	return root_;
}

o3d_utils::Scene* O3DManager::GetScene() {
	return scene_;
}

void O3DManager::CheckError() {
	const std::string& error = client_->GetLastError();
	if (!error.empty()) {
		LOGI("================O3D ERROR====================\n%s", error.c_str());
		client_->ClearLastError();
	}
};
