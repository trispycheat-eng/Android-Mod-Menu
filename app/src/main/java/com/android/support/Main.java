package com.android.support;

import android.content.Context;
import android.widget.Toast;

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
        if (isLoaded) {
            try {
                CrashHandler.init(context, true);
            } catch (Throwable t) {}
        }
    }

    public static void Start(Context context) {
        if (isLoaded) {
            try {
                CrashHandler.init(context, false);
            } catch (Throwable t) {}
        }
    }

    public static native void CheckOverlayPermission(Context context);
}
