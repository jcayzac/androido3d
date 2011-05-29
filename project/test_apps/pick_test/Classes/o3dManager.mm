#include "o3dManager.h"
#include "core/cross/renderer_platform.h"
#include "primitives.h"
#include "materials.h"
#include "camera.h"
#include "debug.h"
#include <stdexcept>

static const char* test_shader_src =
"uniform mat4 worldViewProjection;\n"
"attribute vec4 position;\n"
"attribute vec3 normal;\n"
"varying vec3 v_normal;\n"
"void main() {\n"
"	v_normal = normal;\n"
"	gl_Position = (worldViewProjection * position);\n"
"}\n"
"// #o3d SplitMarker\n"
"uniform vec4 diffuse;\n"
"varying vec3 v_normal;\n"
"void main() {\n"
"	gl_FragColor = diffuse*vec4(v_normal, 1.0);\n"
"}\n"
"// #o3d MatrixLoadOrder RowMajor\n"
;

O3DManager::O3DManager(size_t width, size_t height)
: root_(0)
, pack_(0)
, main_view_(0)
, obj1_(0)
, obj2_(0)
, cameraTime_(.0f)
, obj1Time_(.0f)
, obj2Time_(.0f)
, evaluation_counter_(new o3d::EvaluationCounter(&service_locator_))
, class_manager_(new o3d::ClassManager(&service_locator_))
, client_info_manager_(new o3d::ClientInfoManager(&service_locator_))
, object_manager_(new o3d::ObjectManager(&service_locator_))
, profiler_(new o3d::Profiler(&service_locator_))
, features_(new o3d::Features(&service_locator_))
, renderer_(o3d::Renderer::CreateDefaultRenderer(&service_locator_))
, client_(new o3d::Client(&service_locator_))
, frame_width_(width)
, frame_height_(height)
{
	renderer_->Init(display_window_, false);
	client_->Init();
	pack_ = client_->CreatePack();
	root_ = pack_->Create<o3d::Transform>();
	main_view_ = o3d_utils::ViewInfo::CreateBasicView(pack_, root_, client_->render_graph_root());

	// Create scene
	camera_ = pack_->Create<o3d::Transform>();
	camera_->CreateParam<o3d::ParamString>("name")->set_value("the camera");
	camera_->SetParent(root_);

	obj1_ = pack_->Create<o3d::Transform>();
	std::cerr << "OBJ1: 0x" << std::hex << (uintptr_t)obj1_ << std::dec << "\n";
	obj1_->set_name("object 1");
	obj1_->CreateParam<o3d::ParamBoolean>("pickable")->set_value(true);
	o3d::Primitive* prim = o3d_utils::Primitives::CreateSphere(pack_, 10.0f, 4, 2, 0);
	prim->SetOwner(pack_->Create<o3d::Shape>());

	o3d::Effect* effect = pack_->Create<o3d::Effect>();
	effect->set_name("plouf");
	if (!effect->LoadFromFXString(test_shader_src)) throw std::runtime_error("Can't parse shader!");
	o3d::Material* material = pack_->Create<o3d::Material>();
	material->set_name("plouf_aussi");
	material->set_draw_list(main_view_->z_ordered_draw_pass_info()->draw_list());
	material->set_effect(effect);
	effect->CreateUniformParameters(material);
	obj1_->CreateParam<o3d::ParamFloat4>("diffuse")->set_value(o3d::Float4(1.f, .3f, 0.f, 1.f));

	prim->set_material(material);
	o3d_utils::Primitives::SetBoundingBoxAndZSortPoint(prim);
	obj1_->AddShape(prim->owner());
	obj1_->set_bounding_box(prim->bounding_box());
	obj1_->SetParent(root_);

	obj2_ = pack_->Create<o3d::Transform>();
	std::cerr << "OBJ2: 0x" << std::hex << (uintptr_t)obj2_ << std::dec << "\n";
	obj2_->set_name("object 2");
	obj2_->CreateParam<o3d::ParamBoolean>("pickable")->set_value(true);
	obj2_->SetParent(root_);
	prim = o3d_utils::Primitives::CreateSphere(pack_, 6.0f, 16, 16, 0);
	prim->SetOwner(pack_->Create<o3d::Shape>());
	o3d_utils::Materials material_builder;
	prim->set_material(material_builder.CreateCheckerMaterial(pack_, main_view_));

	o3d_utils::Primitives::SetBoundingBoxAndZSortPoint(prim);
	obj2_->AddShape(prim->owner());
	obj2_->set_bounding_box(prim->bounding_box());
	obj2_->SetParent(root_);

	main_view_->draw_context()->set_projection(o3d_utils::Camera::perspective(0.5235f, (float)width/(float)height, 10.f, 1000.f));

	// Set the light pos on all the materials. We could link a param to
	// all of them so we could just set the one param but I'm lazy.
	o3d::Vector3 light_pos(0,0,200.f);
	std::vector<o3d::Material*> materials = pack_->GetByClass<o3d::Material>();
	for (size_t ii = 0; ii < materials.size(); ++ii) {
		o3d::ParamFloat3* pos = materials[ii]->GetOrCreateParam<o3d::ParamFloat3>("lightWorldPos");
		if (pos) pos->set_value(o3d::Float3(light_pos[0], light_pos[1], 300.f));
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

bool O3DManager::Render() {
	// Update
	float elapsedTimeSinceLastUpdateInSeconds = timer_.GetElapsedTimeAndReset();

	// Update state
	o3d::ParamBoolean* picked;

	picked = obj1_->GetParam<o3d::ParamBoolean>("picked");
	bool animateObj1(!picked || !picked->value());

	picked = obj2_->GetParam<o3d::ParamBoolean>("picked");
	bool animateObj2(!picked || !picked->value());

	obj1Time_ += animateObj1?elapsedTimeSinceLastUpdateInSeconds:.0f;
	obj1_->set_local_matrix(animateObj1?
							o3d::Matrix4::translation(o3d::Vector3(sinf(obj1Time_)*10.f,0.0f,50.f+cosf(obj1Time_)*50.f))
							* o3d::Matrix4::rotationX(obj1Time_)
							* o3d::Matrix4::rotationZ(obj1Time_)
							:
							obj1_->local_matrix()
	);

	obj2Time_ += animateObj2?elapsedTimeSinceLastUpdateInSeconds:.0f;
	obj2_->set_local_matrix(animateObj2?o3d::Matrix4::rotationY(obj2Time_):obj2_->local_matrix());

	cameraTime_ += elapsedTimeSinceLastUpdateInSeconds;
	camera_->set_local_matrix(
		o3d::Matrix4::translation(o3d::Vector3(0.f, sinf(cameraTime_)*50.f,200.f))
	);

	// Render
	client_->Tick();
	main_view_->draw_context()->set_view(affineInverse(camera_->world_matrix()));

	client_->RenderClient(true);

	CheckError();
	return true;
}

void O3DManager::touchedAt(int x, int y) {
	o3d::ParamObject* object = client_->Pick(x, y);
	if (object) {
		std::cerr << "Picked an object: [" << object->name() << "]\n";

		o3d::ParamBoolean* picked = object->GetParam<o3d::ParamBoolean>("picked");
		if (!picked) {
			picked = object->CreateParam<o3d::ParamBoolean>("picked");
			picked->set_value(false);
		}
		picked->set_value(!picked->value());
	}
}


void O3DManager::CheckError() {
	const std::string& error = client_->GetLastError();
	if (!error.empty()) {
		LOGI("================O3D ERROR====================\n%s", error.c_str());
		client_->ClearLastError();
	}
};
