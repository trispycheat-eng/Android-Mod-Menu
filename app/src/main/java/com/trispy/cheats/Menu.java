package com.trispy.cheats;

import de.robv.android.xposed.IXposedHookLoadPackage;
import de.robv.android.xposed.callbacks.XC_LoadPackage;

public class Menu implements IXposedHookLoadPackage {
    @Override
    public void handleLoadPackage(XC_LoadPackage.LoadPackageParam lpparam) throws Throwable {
        if (lpparam.packageName.equals("com.dts.freefireth") ||
            lpparam.packageName.equals("com.dts.freefiremax")) {
            
            System.loadLibrary("MyLibName");
        }
    }
}
