/*
 * Copyright (C) 2007 The Android Open Source Project
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

package com.android.o3djni;

import android.app.Activity;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.MotionEvent;


public class O3DJNIActivity extends Activity {

    O3DJNIView mView;

    @Override protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        mView = new O3DJNIView(getApplication());
        setContentView(mView);
        
        Thread webThread = new Thread(new WebServer(getAssets()));
        webThread.start();
    }

    @Override protected void onPause() {
        super.onPause();
        mView.onPause();
    }

    @Override protected void onResume() {
        super.onResume();
        mView.onResume();
    }

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		O3DJNILib.onKeyDown(keyCode);
		return super.onKeyDown(keyCode, event);
	}

	@Override
	public boolean onKeyUp(int keyCode, KeyEvent event) {
		O3DJNILib.onKeyUp(keyCode);
		return super.onKeyUp(keyCode, event);
	}

	@Override
	public boolean onTouchEvent(MotionEvent event) {
		O3DJNILib.onTouch((int)event.getRawX(), (int)event.getRawY(), 0.0f, 0.0f);
		return super.onTouchEvent(event);
	}

	@Override
	public boolean onTrackballEvent(MotionEvent event) {
		if (event.getAction() == MotionEvent.ACTION_MOVE) {
			O3DJNILib.onRoll(event.getX(), event.getY());
		}
		return super.onTrackballEvent(event);
	}
    
    
}
