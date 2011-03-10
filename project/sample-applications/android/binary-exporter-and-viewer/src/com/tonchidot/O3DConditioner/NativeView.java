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

package com.tonchidot.O3DConditioner;
import android.content.Context;
import android.graphics.PixelFormat;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.opengles.GL10;
import java.io.File;

public class NativeView extends GLSurfaceView  {
    private static String TAG = "NativeView";
    static {
      System.loadLibrary("o3dconditioner");
    }
    private long native_peer = 0;
    private static native long createPeer(long width, long height);
    private static native void destroyPeer(long native_peer);
    private static native void render(long native_peer);
    private static native void onResized(long native_peer, long width, long height);
    private static native void onContextRestored(long native_peer);
    private static native boolean loadScene(long native_peer, java.lang.String path);
    private static native boolean loadBinaryScene(long native_peer, java.lang.String path);
    private static native boolean exportScene(long native_peer, java.lang.String path);
    ////////////////////////////////
    public interface OnCompleteListener {
        public abstract void onComplete(boolean result);
    }
    public NativeView(Context context) {
        super(context);
        initNativeView();
    }
    public NativeView(Context context, AttributeSet attrs) {
        super(context, attrs);
        initNativeView();
    }
    @Override
    protected void finalize() throws Throwable {
        if (native_peer!=0) {
            destroyPeer(native_peer);
            native_peer = 0;
        }
        super.finalize();
    }
    @Override
    public void onResume() {
      super.onResume();
//      initNativeView();
    }
    private final void initNativeView() {
      getHolder().setFormat(PixelFormat.TRANSLUCENT);
      setEGLContextFactory(new ContextFactory());
      setEGLConfigChooser(new ConfigChooser());
      setRenderer(new Renderer());
    }
    public void loadScene(final File src, final OnCompleteListener listener) {
        final String path = src.getAbsolutePath();
        queueEvent(new Runnable(){
            boolean result;
            public void run() {
                result = false;
                if (path.toLowerCase().matches(".+\\.dae$")) {
                    Log.i(O3DConditioner.TAG, "Loading ["+path+"] (Collada)");
                    result = loadScene(native_peer, path);
                }
                else if (path.toLowerCase().matches(".+\\.o3dbin$")) {
                    Log.i(O3DConditioner.TAG, "Loading ["+path+"] (Protocol)");
                    result = loadBinaryScene(native_peer, path);
                }
                post(new Runnable() {
                    public void run() {
                        listener.onComplete(result);
                    }
                });
            }
        });
    }
    public void exportScene(final File dst, final OnCompleteListener listener) {
        final String path = dst.getAbsolutePath();
        queueEvent(new Runnable(){
            public void run() {
                final boolean result = exportScene(native_peer, path);
                post(new Runnable() {
                    public void run() {
                        listener.onComplete(result);
                    }
                });
            }
        });
    }
    private static class ContextFactory implements GLSurfaceView.EGLContextFactory {
        private static int EGL_CONTEXT_CLIENT_VERSION = 0x3098;
        public EGLContext createContext(EGL10 egl, EGLDisplay display, EGLConfig eglConfig) {
            Log.w(TAG, "creating OpenGL ES 2.0 context");
            int[] attrib_list = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL10.EGL_NONE };
            EGLContext context = egl.eglCreateContext(display, eglConfig, EGL10.EGL_NO_CONTEXT, attrib_list);
            int error = egl.eglGetError();
            if (error != EGL10.EGL_SUCCESS)
                Log.e(TAG, String.format("EGL error: 0x%x", error));
            return context;
        }
        public void destroyContext(EGL10 egl, EGLDisplay display, EGLContext context) {
            egl.eglDestroyContext(display, context);
        }
    }
    private static class ConfigChooser implements GLSurfaceView.EGLConfigChooser {
        private static int EGL_OPENGL_ES2_BIT = 4;
        private static int[] s_configAttribs2 = {
            EGL10.EGL_RED_SIZE, 8,
            EGL10.EGL_GREEN_SIZE, 8,
            EGL10.EGL_BLUE_SIZE, 8,
            EGL10.EGL_ALPHA_SIZE, 8,
            EGL10.EGL_DEPTH_SIZE, 16,
            EGL10.EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            // Hum, turning FSAAx4 on makes things lil slow, surprisingly^^
            EGL10.EGL_SAMPLE_BUFFERS, 1,
            EGL10.EGL_SAMPLES, 4,
            EGL10.EGL_NONE
        };
        public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display) {
            /* Get the number of minimally matching EGL configurations
             */
            egl.eglChooseConfig(display, s_configAttribs2, null, 0, mValue);

            int numConfigs = mValue[0];

            if (numConfigs <= 0) {
                throw new IllegalArgumentException("No configs match configSpec");
            }

            /* Allocate then read the array of minimally matching EGL configs
             */
            EGLConfig[] configs = new EGLConfig[numConfigs];
            egl.eglChooseConfig(display, s_configAttribs2, configs, numConfigs, mValue);

            /* Now return the "best" one
             */
            for(EGLConfig config : configs) {
                if (16 > findConfigAttrib(egl, display, config, EGL10.EGL_DEPTH_SIZE, 0)) continue;
                if (8 != findConfigAttrib(egl, display, config, EGL10.EGL_RED_SIZE, 0)) continue;
                if (8 != findConfigAttrib(egl, display, config, EGL10.EGL_GREEN_SIZE, 0)) continue;
                if (8 != findConfigAttrib(egl, display, config, EGL10.EGL_BLUE_SIZE, 0)) continue;
                if (8 != findConfigAttrib(egl, display, config, EGL10.EGL_ALPHA_SIZE, 0)) continue;
                return config;
            }
            return null;
        }
        private int findConfigAttrib(EGL10 egl, EGLDisplay display,
                                     EGLConfig config, int attribute, int defaultValue) {
            if (egl.eglGetConfigAttrib(display, config, attribute, mValue)) {
                return mValue[0];
            }
            return defaultValue;
        }
        private int[] mValue = new int[1];
    }

    private class Renderer implements GLSurfaceView.Renderer {
        private String scene = null;
        public synchronized void setScene(String scene) {
            this.scene = scene;
        }
        public synchronized void onDrawFrame(GL10 gl) {
            if (native_peer!=0) render(native_peer);
        }
        public synchronized void onSurfaceChanged(GL10 gl, int width, int height) {
            Log.i(O3DConditioner.TAG, "onSurfaceChanged()");
            if (native_peer!=0) onResized(native_peer, width, height);
            else                native_peer = createPeer(width, height);
        }
        public synchronized void onSurfaceCreated(GL10 gl, EGLConfig config) {
            Log.i(O3DConditioner.TAG, "onSurfaceCreated()");
            if (native_peer!=0) onContextRestored(native_peer);
        }
    }
}
