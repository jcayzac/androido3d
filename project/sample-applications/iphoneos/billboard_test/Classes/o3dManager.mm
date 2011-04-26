#include "o3dManager.h"
#include "core/cross/renderer_platform.h"
#include "primitives.h"
#include "materials.h"

#define CREATE_PARAM(x,type,key,value) { (x).CreateParam<type>(key)->set_value(value); }
#define GET_PARAM(x,type,key) (x).GetParam<type>(key)

#define CONST_FIELD_OF_VIEW	  37.0f
#define CONST_NEAR_CLIP		   3.0f
#define CONST_FAR_CLIP		6200.0f

O3DManager::O3DManager()
: root_(NULL),
hud_root_(NULL),
pack_(NULL),
main_view_(NULL),
hud_view_(NULL),
scene_(NULL),
time_(0.0f) {
}

O3DManager::~O3DManager()
{
	for (size_t ii = 0; ii < arraysize(billboards_); ++ii) {
		o3d_utils::DeleteBillboard(pack_, billboards_[ii]);
	}
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
	root_ = pack_->Create<o3d::Transform>();
	main_view_ = o3d_utils::ViewInfo::CreateBasicView(pack_, root_, client_->render_graph_root());

	// Create a camera
	camera_ = pack_->Create<o3d::Transform>();
	// The camera can't be the root of the scenegraph, it has to be a descendant of root.
	camera_->SetParent(root_);
	CREATE_PARAM(*camera_, o3d::ParamString, "collada.tags", "camera");
	CREATE_PARAM(*camera_, o3d::ParamFloat3, "collada.eyePosition", o3d::Float3(.0f, .0f, .0f));
	CREATE_PARAM(*camera_, o3d::ParamFloat3, "collada.targetPosition", o3d::Float3(.0f, .0f, -1.0f));
	CREATE_PARAM(*camera_, o3d::ParamFloat3, "collada.upVector", o3d::Float3(.0f, 1.0f, .0f));
	CREATE_PARAM(*camera_, o3d::ParamString, "collada.projectionType", "perspective");
	CREATE_PARAM(*camera_, o3d::ParamFloat, "collada.projectionNearZ", CONST_NEAR_CLIP);
	CREATE_PARAM(*camera_, o3d::ParamFloat, "collada.projectionFarZ", CONST_FAR_CLIP);
	CREATE_PARAM(*camera_, o3d::ParamFloat, "collada.perspectiveFovY", CONST_FIELD_OF_VIEW);
	
	const char *filename;
	GET_IMAGE_RESOURCE_FILENAME( filename, "horiz_stripes.png" );
	o3d::Texture* texture = pack_->CreateTextureFromFile(filename, filename, o3d::image::UNKNOWN, true);
	o3d::Texture2D* tex2d;
	if (!texture) {
		// If we couldn't load the texture use the ERROR texture.
		DLOG(ERROR) << "Could not load texture: " << filename;
		o3d::Renderer* renderer =
        pack_->service_locator()->GetService<o3d::Renderer>();
		tex2d = down_cast<o3d::Texture2D*>(renderer->error_texture());
	}
	else {
		texture->set_name(filename);
		DCHECK(texture->IsA(o3d::Texture2D::GetApparentClass()));
		tex2d = down_cast<o3d::Texture2D*>(texture);
	}

	o3d::Sampler *sampler_ = pack_->Create<o3d::Sampler>();
	sampler_->set_address_mode_u(o3d::Sampler::CLAMP);
	sampler_->set_address_mode_v(o3d::Sampler::CLAMP);
	sampler_->set_texture(tex2d);
	
	// Add billboards
    for (size_t ii = 0; ii < arraysize(billboards_); ++ii) {
        info[ii].pos[0] = (rand() % 300) - 150;
        info[ii].pos[1] = (rand() % 100) - 50;
        info[ii].pos[2] = (rand() % 300) - 150;
		info[ii].dir[0] = -1.0f + 2.0f * (rand() / (float)RAND_MAX);
		info[ii].dir[1] = -1.0f + 2.0f * (rand() / (float)RAND_MAX);
		info[ii].dir[2] = -1.0f + 2.0f * (rand() / (float)RAND_MAX);
		info[ii].dir = normalize( info[ii].dir );
		info[ii].speed = -1.0f + 2.0f * (rand() / (float)RAND_MAX);
		billboards_[ii] = GenerateBillboard(pack_, main_view_, root_,
											sampler_, info[ii].pos[0], info[ii].pos[1], info[ii].pos[2], 14, 14);
    }	

	// Setup the worldview & projection matrices. This code should also be called every time the viewport gets resized.
	scoped_ptr<o3d_utils::CameraInfo> camera_info(o3d_utils::Camera::getViewAndProjectionFromCamera(camera_, width, height));
	camera_->CreateParam<o3d::ParamMatrix4>("view_matrix");
	camera_->CreateParam<o3d::ParamMatrix4>("projection_matrix");
	GET_PARAM(*camera_, o3d::ParamMatrix4, "view_matrix")->set_value(camera_info->view);
	GET_PARAM(*camera_, o3d::ParamMatrix4, "projection_matrix")->set_value(camera_info->projection);
	
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
	
	return true;
}

bool O3DManager::OnContextRestored() {
	// Reset the timer so we don't have some giant time slice.
	timer_.GetElapsedTimeAndReset();
	
	// Restore the resources.
	return down_cast<o3d::RendererGLES2*>(renderer_)->OnContextRestored();
}

bool O3DManager::ResizeViewport(int width, int height) {
	renderer_->Resize(width, height);
	
	scoped_ptr<o3d_utils::CameraInfo> camera_info(o3d_utils::Camera::getViewAndProjectionFromCamera(camera_, width, height));
	GET_PARAM(*camera_, o3d::ParamMatrix4, "view_matrix")->set_value(camera_info->view);
	GET_PARAM(*camera_, o3d::ParamMatrix4, "projection_matrix")->set_value(camera_info->projection);
	
	return true;
}

bool O3DManager::Render()
{
	float elapsedTimeSinceLastUpdateInSeconds = timer_.GetElapsedTimeAndReset();
	time_ += elapsedTimeSinceLastUpdateInSeconds;
	
	const float camSpeed = 0.15f;
	const float camModeDuration = 8.0f;;
	static float camTimer = 0.0f;
	static int camMode = 0;
	
	camTimer += elapsedTimeSinceLastUpdateInSeconds;
	
	// Move the camera
	switch ( camMode )
	{
		case 0:
			camera_->set_local_matrix(o3d::Matrix4::rotationY(camSpeed*time_));
			break;
		case 1:
			camera_->set_local_matrix(o3d::Matrix4::rotationZ(camSpeed*time_));
			break;
		case 2:
			camera_->set_local_matrix(o3d::Matrix4::rotationX(camSpeed*time_));
			break;
		case 3:
			camera_->set_local_matrix(o3d::Matrix4::translation(o3d::Vector3(50.0*sinf(camSpeed*time_), .0f, .0f))) ;
			break;
	}
	
	if (camTimer > camModeDuration)
	{
		camTimer = 0.0f;
		camMode = (camMode+1) % 4;
	}
		
	client_->Tick();
	
	main_view_->draw_context()->set_projection(GET_PARAM(*camera_, o3d::ParamMatrix4, "projection_matrix")->value());
	main_view_->draw_context()->set_view( GET_PARAM(*camera_, o3d::ParamMatrix4, "view_matrix")->value() * inverse(camera_->world_matrix()) );

	const float billModeDuration = camModeDuration * 4;
	static float billTimer = 0.0f;
	static int billStateMode = 0;
	
	billTimer += elapsedTimeSinceLastUpdateInSeconds;

	// Animate one billboard state
	switch ( billStateMode )
	{
		case 0:
			// Don't change any state
			break;
		case 1:
			for (size_t ii = 0; ii < arraysize(billboards_); ++ii) {
				float s = info[ii].speed * billTimer;
				o3d::Vector3 p = info[ii].pos + sinf(s) * info[ii].dir;
				o3d_utils::SetPosition( pack_, billboards_[ii], p[0], p[1], p[2] );
			}
			break;
		case 2:
			for (size_t ii = 0; ii < arraysize(billboards_); ++ii) {
				float s = 12.0f + 6.0f * sinf( info[ii].speed * billTimer );
				o3d_utils::SetSize( pack_, billboards_[ii], s, s );
			}
			break;
		case 3:
			for (size_t ii = 0; ii < arraysize(billboards_); ++ii) {
				float s = info[ii].speed * billTimer * info[ii].dir[0];
				float t = info[ii].speed * billTimer * info[ii].dir[1];
				o3d_utils::SetTexcoords( pack_, billboards_[ii], s, t, 1, 1 );
			}
			break;
		case 4:
			for (size_t ii = 0; ii < arraysize(billboards_); ++ii) {
				float s = info[ii].speed * billTimer * 0.3f;
				o3d_utils::SetZAxisRotationRadians( pack_, billboards_[ii], s );
			}
			break;
	}
	
	if (billTimer > billModeDuration)
	{
		// Reset bill board states
		billTimer = 0.0f;
		switch ( billStateMode )
		{
			case 0:
				// Nothing to reset
				break;
			case 1:
				for (size_t ii = 0; ii < arraysize(billboards_); ++ii) {
					o3d_utils::SetPosition( pack_, billboards_[ii], info[ii].pos[0], info[ii].pos[1], info[ii].pos[2] );
				}
				break;
			case 2:
				for (size_t ii = 0; ii < arraysize(billboards_); ++ii) {
					o3d_utils::SetSize( pack_, billboards_[ii], 14, 14 );
				}
				break;
			case 3:
				for (size_t ii = 0; ii < arraysize(billboards_); ++ii) {
					o3d_utils::SetTexcoords( pack_, billboards_[ii], 0, 0, 1, 1 );
				}
				break;
			case 4:
				for (size_t ii = 0; ii < arraysize(billboards_); ++ii) {
					o3d_utils::SetZAxisRotationRadians( pack_, billboards_[ii], 0 );
				}
				break;
		}
		billStateMode = (billStateMode+1) % 5;
	}
	
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
