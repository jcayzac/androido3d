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

class O3DManager {
public:
	O3DManager(size_t width, size_t height);

	o3d::Client* client() const {
		return client_.get();
	}

	o3d_utils::ViewInfo* main_view() const {
		return main_view_;
	}

	bool Render();
	bool OnContextRestored();
	void CheckError();

	void touchedAt(int x, int y);

private:
	class window_t: public o3d::DisplayWindow {
	public:
		~window_t() { }
	};
	window_t display_window_;
	o3d::ServiceLocator service_locator_;
	o3d::Renderer* renderer_;
	scoped_ptr<o3d::EvaluationCounter> evaluation_counter_;
	scoped_ptr<o3d::ClassManager> class_manager_;
	scoped_ptr<o3d::ClientInfoManager> client_info_manager_;
	scoped_ptr<o3d::ObjectManager> object_manager_;
	scoped_ptr<o3d::Profiler> profiler_;
	scoped_ptr<o3d::Features> features_;
	scoped_ptr<o3d::Client> client_;

	int frame_width_, frame_height_;

	o3d::Transform* root_;
	o3d::Transform* camera_;
	o3d::Transform* obj1_;
	o3d::Transform* obj2_;
	o3d::Pack* pack_;
	o3d_utils::ViewInfo* main_view_;
	o3d::ElapsedTimeTimer timer_;
	float cameraTime_;
	float obj1Time_;
	float obj2Time_;
};
