package com.android.support;

import android.content.Context;

public class NativeBridge {
    public static native void Changes(Context ctx, String featName, int featNum, long lValue, boolean bool, String str);
}
