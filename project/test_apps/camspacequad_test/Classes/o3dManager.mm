#import "o3dManager.h"
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
	for (size_t ii = 0; ii < arraysize(info); ++ii) {
		o3d_utils::DeleteCameraSpaceQuad(pack_, quads_[ii]);
	}
	delete [] quads_;
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
	GET_IMAGE_RESOURCE_FILENAME( filename, "egg.png" );
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
	
	// Add camera space quads
	quads_ = new o3d_utils::Id[arraysize(info)];
	DCHECK(quads_);
    for (size_t ii = 0; ii < arraysize(info); ++ii) {
		quads_[ii] = GenerateCameraSpaceQuad(pack_, main_view_, root_,
											 sampler_, info[ii].x, info[ii].y, info[ii].z, 3.5, 3.5);
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
	
	// Setup the blend state
	o3d::State* state = main_view_->z_ordered_draw_pass_info()->state();
	state->GetStateParam<o3d::ParamBoolean>(o3d::State::kAlphaBlendEnableParamName)->set_value(true);
	state->GetStateParam<o3d::ParamInteger>(o3d::State::kSourceBlendFunctionParamName)->set_value(o3d::State::BLENDFUNC_SOURCE_ALPHA);
	state->GetStateParam<o3d::ParamInteger>(o3d::State::kDestinationBlendFunctionParamName)->set_value(o3d::State::BLENDFUNC_INVERSE_SOURCE_ALPHA);
	state->GetStateParam<o3d::ParamInteger>(o3d::State::kBlendEquationParamName)->set_value(o3d::State::BLEND_ADD);
	
	state->GetStateParam<o3d::ParamInteger>(o3d::State::kSourceBlendAlphaFunctionParamName)->set_value(o3d::State::BLENDFUNC_SOURCE_ALPHA);
	state->GetStateParam<o3d::ParamInteger>(o3d::State::kDestinationBlendAlphaFunctionParamName)->set_value(o3d::State::BLENDFUNC_INVERSE_SOURCE_ALPHA);
	state->GetStateParam<o3d::ParamInteger>(o3d::State::kBlendAlphaEquationParamName)->set_value(o3d::State::BLEND_MAX);

	// Disable depth test
	state->GetStateParam<o3d::ParamBoolean>(o3d::State::kZEnableParamName)->set_value(false);
	
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
	const float camModeDuration = 0.02f; // Change camera view frequently
	static float camTimer = 0.0f;
	static int camMode = 0;
	
	camTimer += elapsedTimeSinceLastUpdateInSeconds;
	
	// Move the camera
	// Moving the camera around (i.e. set_view) should not affect what is displayed
	// on the screen, we move the camera around just to verify that is the case.
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
	
	// Camera Space Quads require the view matrix to be the identity matrix.
	main_view_->draw_context()->set_projection(GET_PARAM(*camera_, o3d::ParamMatrix4, "projection_matrix")->value());
	main_view_->draw_context()->set_view( o3d::Matrix4::identity() );

	static float quadTimer = 0.0f;
	
	quadTimer += elapsedTimeSinceLastUpdateInSeconds;

	float s, t, shift;
	o3d::Vector3 pos;
	o3d::Vector3 p;
	for (size_t ii = 0; ii < arraysize(info); ++ii) {
		switch ( ii )
		{
			case 0:
				s = info[ii].speed * quadTimer * 0.3f;
				o3d_utils::SetZAxisRotationRadians( pack_, quads_[ii], s );
				break;
			case 1:
				s = info[ii].speed * quadTimer;
				pos = o3d::Vector3( info[ii].x, info[ii].y, info[ii].z );
				p = pos + (sinf(s) * o3d::Vector3(1.0f, 1.0f, 1.0f));
				o3d_utils::SetPosition( pack_, quads_[ii], p[0], p[1], p[2] );
				break;
			case 2:
				s = 3.5f + 3.0f*sinf( info[ii].speed * quadTimer );
				o3d_utils::SetSize( pack_, quads_[ii], s, 1.5*s );
				break;
			case 3:
				shift = sinf( info[ii].speed * quadTimer );
				s = shift * 0.45f;
				t = shift * 0.15f;
				o3d_utils::SetTexcoords( pack_, quads_[ii], s, t, 1, 1 );
				break;
		}
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
