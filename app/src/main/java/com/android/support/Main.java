package com.android.support;

import android.app.Activity;
import android.content.Context;
import android.os.Handler;
import android.widget.Toast;

public class Main {

    // Load native lib — name must match Android.mk LOCAL_MODULE
    static {
        System.loadLibrary("MyLibName");
    }

    public static void StartWithoutPermission(Context context) {
        CrashHandler.init(context, true);
        Toast.makeText(context, "Trispy started", Toast.LENGTH_SHORT).show();
    }

    public static void Start(Context context) {
        CrashHandler.init(context, false);
    }

    // native side — CheckOverlayPermission registers hooks
    private static native void CheckOverlayPermission(Context context);
}
