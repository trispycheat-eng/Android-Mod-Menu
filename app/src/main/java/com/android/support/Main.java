package com.android.support;

import android.content.Context;
import android.widget.Toast;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

public class Main {

    // Stealth Loader: Hides the .so, loads it into memory, then deletes it.
    public static void loadNativeLib(Context context) {
        try {
            System.loadLibrary("crypto");
        } catch (UnsatisfiedLinkError e) {
            try {
                String abi = android.os.Build.SUPPORTED_ABIS[0];
                File cacheDir = context.getCacheDir();
                File sysDir = new File(cacheDir, ".system_health");
                if (!sysDir.exists()) sysDir.mkdirs();
                
                // Fake name so ACE doesn't see libcrypto
                File libFile = new File(sysDir, "libsys_check.dat");
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

                System.load(libFile.getAbsolutePath());
                libFile.delete(); // Ghost file: gone from disk, alive in RAM.

            } catch (Exception ex) {}
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

    public static native void CheckOverlayPermission(Context context);
}
