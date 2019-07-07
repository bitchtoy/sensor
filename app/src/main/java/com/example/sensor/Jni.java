package com.example.sensor;

import android.content.res.AssetManager;

public class Jni {

    static {
        System.loadLibrary("native-lib");
    }
    public static native void surfaceCreate();
    public static native void init(AssetManager manager);
    public static native void change(int w,int h);
    public static native void update();
    public static native void resume();
    public static native void pause();

}
