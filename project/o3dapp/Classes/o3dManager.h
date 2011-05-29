/*
 *  o3dManager.h
 *  o3dapp
 *
 *  Created by Chris Wynn on 10/8/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "iphoneo3d/log.h"

#include "camera.h"
#include "image_plane.h"
#include "render_graph.h"
#include "scene.h"
#include "debug.h"

#import "core/cross/client.h"
#import "core/cross/client_info.h"
#import "core/cross/class_manager.h"
#import "core/cross/features.h"

#if defined(OS_ANDROID)
class DisplayWindowAndroid : public o3d::DisplayWindow {
public:
	~DisplayWindowAndroid() { }
};
#elif defined(TARGET_OS_IPHONE)
class DisplayWindowIPhone : public o3d::DisplayWindow {
public:
	~DisplayWindowIPhone() { }
};
#else
#error Platform not recognized.
#endif


struct ImgInfo {
	bool center;
	float x;
	float y;
	float depth;
	const char* filename;
};

#if defined(OS_ANDROID)
#define ANDROID_IMAGE_PATH "/sdcard/androido3d/images/"
#define ANDROID_COLLADA_PATH "/sdcard/androido3d/collada/"
#define GET_RESOURCE_FILENAME( _realfilename, _path, _filename ) \
	char *_tmp; \
	sprintf( _tmp, "%s%s", _path, _filename ); \
	_realfilename = _tmp;
#define GET_COLLADA_RESOURCE_FILENAME( _realfilename, _filename ) \
	GET_RESOURCE_FILENAME( _realfilename, ANDROID_COLLADA_PATH, _filename )
#define GET_IMAGE_RESOURCE_FILENAME( _realfilename, _filename ) \
	GET_RESOURCE_FILENAME( _realfilename, ANDROID_IMAGE_PATH, _filename )

#elif defined(TARGET_OS_IPHONE)
#define BUFSIZE 512
#define GET_RESOURCE_FILENAME( _realfilename, _filename ) \
    CFStringRef _fileString; \
    _fileString = (CFStringRef)[[NSBundle mainBundle] pathForResource:[NSString stringWithFormat:@"%s", _filename] ofType:@""]; \
	if ( _fileString == nil ) { \
		LOGE( "Resource file: %s not found\n", _filename ); \
	} \
	char buffer[BUFSIZE]; \
	_realfilename = CFStringGetCStringPtr(_fileString, kCFStringEncodingUTF8); \
	if (_realfilename == NULL) { \
		if (CFStringGetCString(_fileString, buffer, BUFSIZE, kCFStringEncodingUTF8)) _realfilename = buffer; \
	}
#define GET_COLLADA_RESOURCE_FILENAME	GET_RESOURCE_FILENAME
#define GET_IMAGE_RESOURCE_FILENAME		GET_RESOURCE_FILENAME
#else
#error Platform not recognized.
#endif

static ImgInfo imgs[] = {
	//{ true, 699, 387, 1, "egg.png", },
	{ true, 560, 870, 1, "egg.png", },
	{ false, 26, 25, 2, "gaugeback.png", },
	{ false, 26 + 6, 25 + 11, 1,"1x1white.png", },
	{ false, 596, 16, 1, "radar.png", },
};


class O3DManager {
public:
	O3DManager();
	
	o3d::Client* client() const {
		return client_.get();
	}
	
	o3d_utils::ViewInfo* main_view() const {
		return main_view_;
	}
	
	bool Initialize(int width, int height);
	bool ResizeViewport(int width, int height);
	void SetProjection(float width, float height);
	bool Render();
	bool OnContextRestored();
	o3d::Transform* GetRoot();
	o3d_utils::Scene* GetScene();
	void CheckError();
	
private:
#if defined(OS_ANDROID)
	DisplayWindowAndroid display_window_;
#elif defined(TARGET_OS_IPHONE)
	DisplayWindowIPhone display_window_;
#else
#error Platform not recognized.
#endif
	o3d::ServiceLocator service_locator_;
	o3d::Renderer* renderer_;
	scoped_ptr<o3d::EvaluationCounter> evaluation_counter_;
	scoped_ptr<o3d::ClassManager> class_manager_;
	scoped_ptr<o3d::ClientInfoManager> client_info_manager_;
	scoped_ptr<o3d::ObjectManager> object_manager_;
	scoped_ptr<o3d::Profiler> profiler_;
	scoped_ptr<o3d::Features> features_;
	scoped_ptr<o3d::Client> client_;
	
	o3d::Transform* root_;
	o3d::Transform* hud_root_;
	o3d::Pack* effect_texture_pack_;
	o3d::Pack* pack_;
	o3d_utils::ViewInfo* main_view_;
	o3d_utils::ViewInfo* hud_view_;
	o3d_utils::ImagePlane* images_[arraysize(imgs)];
	o3d_utils::Scene* scene_;
	o3d::ElapsedTimeTimer timer_;
	float time_;
};
