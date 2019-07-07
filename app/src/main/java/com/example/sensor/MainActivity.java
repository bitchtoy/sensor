package com.example.sensor;

import android.app.Activity;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MainActivity extends Activity {

    // Used to load the 'native-lib' library on application startup.
//    static {
//        System.loadLibrary("native-lib");
//    }
    GLSurfaceView view;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        view = new GLSurfaceView(getApplication());
        view.setEGLContextClientVersion(2);
        view.setRenderer(new GLSurfaceView.Renderer() {
            @Override
            public void onSurfaceCreated(GL10 gl, EGLConfig config) {
                Jni.surfaceCreate();
            }

            @Override
            public void onSurfaceChanged(GL10 gl, int width, int height) {
                Jni.change(width,height);
            }

            @Override
            public void onDrawFrame(GL10 gl) {
                Jni.update();
            }
        });
        view.queueEvent(new Runnable() {
            @Override
            public void run() {
                Jni.init(getAssets());
            }
        });
        setContentView(view);

        // Example of a call to a native method
//        TextView tv = findViewById(R.id.sample_text);
//        tv.setText(stringFromJNI());
    }

    @Override
    protected void onResume() {
        super.onResume();
        view.onResume();
        view.queueEvent(new Runnable() {
            @Override
            public void run() {
                Jni.resume();
            }
        });
    }

    @Override
    protected void onPause() {
        super.onPause();
        view.onPause();
        view.queueEvent(new Runnable() {
            @Override
            public void run() {
                Jni.pause();
            }
        });
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

    }
    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
//    public native String stringFromJNI();
}
