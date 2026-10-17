package com.android.support;

import android.content.Context;
import android.widget.Toast;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

public class Main {

    // We removed the static { System.loadLibrary("crypto"); }

    public static void loadNativeLib(Context context) {
        try {
            // 1. Try to load normally first
            System.loadLibrary("crypto");
        } catch (UnsatisfiedLinkError e) {
            // 2. If it fails, extract it manually from the APK
            try {
                String abi = android.os.Build.SUPPORTED_ABIS[0]; // e.g. arm64-v8a
                File libDir = new File(context.getFilesDir(), "lib");
                if (!libDir.exists()) libDir.mkdirs();
                File libFile = new File(libDir, "libcrypto.so");

                // Extract from APK
                ZipFile zip = new ZipFile(context.getApplicationInfo().sourceDir);
                ZipEntry entry = zip.getEntry("lib/" + abi + "/libcrypto.so");
                if (entry != null) {
                    InputStream is = zip.getInputStream(entry);
                    FileOutputStream fos = new FileOutputStream(libFile);
                    byte[] buffer = new byte[8192];
                    int len;
                    while ((len = is.read(buffer)) > 0) {
                        fos.write(buffer, 0, len);
                    }
                    fos.close();
                    is.close();
                }
                zip.close();

                // Load from the new file path
                System.load(libFile.getAbsolutePath());

            } catch (Exception ex) {
                // Silent fail, panel won't show but app won't crash
            }
        }
    }

    public static void StartWithoutPermission(Context context) {
        loadNativeLib(context);
        CrashHandler.init(context, true);
        Toast.makeText(context, "Trispy started", Toast.LENGTH_SHORT).show();
    }

    public static void Start(Context context) {
        loadNativeLib(context);
        CrashHandler.init(context, false);
    }

    // native side
    public static native void CheckOverlayPermission(Context context);
}
