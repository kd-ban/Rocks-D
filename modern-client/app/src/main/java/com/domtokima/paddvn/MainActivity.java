package com.domtokima.paddvn;

import android.app.Activity;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public final class MainActivity extends Activity {
    static {
        System.loadLibrary("game");
    }

    private GLSurfaceView surface;

    private static native void nativeInit();
    private static native void nativeResize(int width, int height);
    private static native void nativeRender();
    private static native void nativePause();
    private static native void nativeResume();

    @Override protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);
        getWindow().getDecorView().setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_FULLSCREEN
                        | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                        | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);

        surface = new GLSurfaceView(this);
        surface.setEGLContextClientVersion(2);
        surface.setPreserveEGLContextOnPause(true);
        surface.setRenderer(new Renderer());
        surface.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        setContentView(surface);
    }

    @Override protected void onPause() {
        nativePause();
        surface.onPause();
        super.onPause();
    }

    @Override protected void onResume() {
        super.onResume();
        surface.onResume();
        nativeResume();
    }

    private static final class Renderer implements GLSurfaceView.Renderer {
        @Override public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            nativeInit();
        }

        @Override public void onSurfaceChanged(GL10 gl, int width, int height) {
            nativeResize(width, height);
        }

        @Override public void onDrawFrame(GL10 gl) {
            nativeRender();
        }
    }
}
