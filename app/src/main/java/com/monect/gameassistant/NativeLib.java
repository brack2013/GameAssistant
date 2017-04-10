package com.monect.gameassistant;

import android.util.Log;

public class NativeLib {

    static {
        System.loadLibrary("nativelibjni");
    }

    public static native boolean OpenFrameBuffer();

    public final static int XINPUTS_MOUSE = 0;
    public final static int XINPUTS_TOUCHSCREEN = 1;

    public static native int openInputs(int idevicetype, int screenWidth, int screenHeight);

    public static native int touchevent(int id, int ix, int iy);

    public static native int touchend();

    public static native int mouseevent(boolean bleft, boolean bright, int ix, int iy);

    public static native void closeInputs();

    public static native int setTarget(String targetPath, float roiX, float roiY, float roiWidth, float roiHeight);

    public static native int findTargetInScreen(float[] xArray, float[] yArray);

    public static native void saveScreenShot();
}
