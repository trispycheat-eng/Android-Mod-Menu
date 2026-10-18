package com.android.support;

import android.content.Context;

public class Main {
    public static boolean isLoaded = false;
    
    static {
        try {
            System.loadLibrary("jni");
            isLoaded = true;
        } catch (Throwable t) {
            isLoaded = false;
        }
    }

    public static void StartWithoutPermission(Context context) {
        // Stripped CrashHandler to prevent VM crash
    }

    public static void Start(Context context) {
        // Stripped
    }

    public static native void CheckOverlayPermission(Context context);
}
