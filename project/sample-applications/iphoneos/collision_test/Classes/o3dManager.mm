#include "o3dManager.h"
#include "core/cross/renderer_platform.h"
#include "extra/cross/collision_detection.h"
#include "primitives.h"
#include "materials.h"

O3DManager::O3DManager(size_t width, size_t height)
: root_(0)
, pack_(0)
, main_view_(0)
, obj1_(0)
, obj2_(0)
, time_(0.0f)
, evaluation_counter_(new o3d::EvaluationCounter(&service_locator_))
, class_manager_(new o3d::ClassManager(&service_locator_))
, client_info_manager_(new o3d::ClientInfoManager(&service_locator_))
, object_manager_(new o3d::ObjectManager(&service_locator_))
, profiler_(new o3d::Profiler(&service_locator_))
, features_(new o3d::Features(&service_locator_))
, renderer_(o3d::Renderer::CreateDefaultRenderer(&service_locator_))
, client_(new o3d::Client(&service_locator_))
{
	renderer_->Init(display_window_, false);
	client_->Init();
	pack_ = client_->CreatePack();
	root_ = pack_->Create<o3d::Transform>();
	main_view_ = o3d_utils::ViewInfo::CreateBasicView(pack_, root_, client_->render_graph_root());

	// Create scene
	obj1_ = pack_->Create<o3d::Transform>();
	obj1_->SetParent(root_);
	o3d::Primitive* prim = o3d_utils::Primitives::CreateSphere(pack_, 10.0f, 4, 2, 0);
	prim->SetOwner(pack_->Create<o3d::Shape>());
	o3d_utils::Materials material_builder;
	o3d::Float4 color(1.f, .3f, 0.f, 1.f);
	prim->set_material(material_builder.CreateBasicMaterial(pack_, main_view_, &color, 0, false));
	o3d_utils::Primitives::SetBoundingBoxAndZSortPoint(prim);
	obj1_->AddShape(prim->owner());
	obj1_->set_bounding_box(prim->bounding_box());

	obj2_ = pack_->Create<o3d::Transform>();
	obj2_->SetParent(root_);
	prim = o3d_utils::Primitives::CreateSphere(pack_, 6.0f, 16, 16, 0);
	prim->SetOwner(pack_->Create<o3d::Shape>());
	o3d::Float4 color2(.3f, 1.f, 0.f, 1.f);
	prim->set_material(material_builder.CreateCheckerMaterial(pack_, main_view_));
	o3d_utils::Primitives::SetBoundingBoxAndZSortPoint(prim);
	obj2_->AddShape(prim->owner());
	obj2_->set_bounding_box(prim->bounding_box());

	o3d_utils::CameraInfo* camera_info = o3d_utils::Camera::getViewAndProjectionFromCameras(root_, width, height);
	main_view_->draw_context()->set_view(camera_info->view);
	SetProjection(width, height);

	// Set the light pos on all the materials. We could link a param to
	// all of them so we could just set the one param but I'm lazy.
	o3d::Vector3 light_pos = Vectormath::Aos::inverse(camera_info->view).getTranslation();
	std::vector<o3d::Material*> materials = pack_->GetByClass<o3d::Material>();
	for (size_t ii = 0; ii < materials.size(); ++ii) {
		o3d::ParamFloat3* pos = materials[ii]->GetParam<o3d::ParamFloat3>("lightWorldPos");
		if (pos) pos->set_value(o3d::Float3(light_pos[0], light_pos[1], light_pos[2]));
	}

	timer_.GetElapsedTimeAndReset();

	CheckError();
}

bool O3DManager::OnContextRestored() {
	// Reset the timer so we don't have some giant time slice.
	timer_.GetElapsedTimeAndReset();

	// Restore the resources.
	return down_cast<o3d::RendererGLES2*>(renderer_)->OnContextRestored();
}

void O3DManager::SetProjection(float width, float height) {
	main_view_->draw_context()->set_projection(
		o3d_utils::Camera::perspective(o3d_utils::degToRad(30.0f), width / height, 10, 1000)
	);
}

bool O3DManager::ResizeViewport(int width, int height) {
	renderer_->Resize(width, height);

	o3d_utils::CameraInfo* camera_info =
	o3d_utils::Camera::getViewAndProjectionFromCameras(root_, width, height);
	main_view_->draw_context()->set_view(camera_info->view);
	SetProjection(width, height);
	return true;
}

bool O3DManager::Render() {
	float elapsedTimeSinceLastUpdateInSeconds = timer_.GetElapsedTimeAndReset();
	time_ += elapsedTimeSinceLastUpdateInSeconds;

	// Update state
	obj1_->set_local_matrix(o3d::Matrix4::translation(o3d::Vector3(sinf(time_)*10.f,.0f,cosf(time_)*50.f))
							* o3d::Matrix4::rotationX(time_)
							* o3d::Matrix4::rotationZ(time_)
	);
	obj2_->set_local_matrix(o3d::Matrix4::rotationY(time_));
	client_->Tick();
	o3d::BoundingBox intersection;
	const bool collide(
		o3d_extra::CollisionDetection::ComputeIntersection(
			*obj1_,
			*obj2_,
			intersection,
			o3d_extra::CollisionDetection::PRECISION_LEVEL_PRIMITIVES_ANY
		)
	);
	// Render
	o3d::Float4 clearColors[2] = { o3d::Float4(.2f,.2f,.2f,0), o3d::Float4(.8f,.0f,.0f,0) };
	main_view_->clear_buffer()->set_clear_color(clearColors[collide]);
	client_->RenderClient(true);

	CheckError();
	return true;
}

void O3DManager::CheckError() {
	const std::string& error = client_->GetLastError();
	if (!error.empty()) {
		LOGI("================O3D ERROR====================\n%s", error.c_str());
		client_->ClearLastError();
	}
};
