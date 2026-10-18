package com.android.support;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.provider.Settings;
import android.view.Gravity;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;
import java.io.OutputStreamWriter;

public class MainActivity extends Activity {
    private static final String FF_MAX = "com.dts.freefiremax";
    private static final String FF_NORMAL = "com.dts.freefireth";
    private static final int OVERLAY_REQ = 1002;
    
    private WindowManager wm;
    private View logoBtn;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Check for Overlay Permission first
        if (Build.VERSION.SDK_INT >= 23 && !Settings.canDrawOverlays(this)) {
            try {
                startActivityForResult(new Intent(
                        Settings.ACTION_MANAGE_OVERLAY_PERMISSION,
                        Uri.parse("package:" + getPackageName())), OVERLAY_REQ);
            } catch (Throwable t) {}
        }

        LinearLayout root = new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setGravity(Gravity.CENTER);
        root.setBackgroundColor(Color.parseColor("#0a0000"));
        root.setPadding(60, 60, 60, 60);

        TextView title = new TextView(this);
        title.setText("▲ TRISPY VM ▲");
        title.setTextColor(Color.parseColor("#FF4500"));
        title.setTextSize(34);
        title.setGravity(Gravity.CENTER);
        root.addView(title);

        Button injectBtn = new Button(this);
        injectBtn.setText("INJECT & LAUNCH");
        injectBtn.setTextSize(18);
        injectBtn.setBackgroundColor(Color.parseColor("#FF2200"));
        injectBtn.setTextColor(Color.WHITE);
        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(-1, -2);
        params.setMargins(0, 50, 0, 0);
        injectBtn.setLayoutParams(params);
        
        injectBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                injectBtn.setText("INJECTING...");
                v.setEnabled(false);
                
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        nukeBanCache();
                        spoofProperties();
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                showFloatingLogo();
                                launchGame();
                            }
                        });
                    }
                }).start();
            }
        });
        root.addView(injectBtn);
        setContentView(root);
    }

    @Override
    protected void onActivityResult(int req, int res, Intent data) {
        super.onActivityResult(req, res, data);
    }

    private void spoofProperties() {
        String[] props = {
            "setprop ro.product.model SM-G998B",
            "setprop ro.product.brand samsung",
            "setprop ro.product.manufacturer samsung",
            "setprop ro.hardware exynos2100",
            "setprop ro.kernel.qemu 0",
            "setprop ro.vphonegaga 0"
        };
        try {
            Process process = Runtime.getRuntime().exec("su");
            OutputStreamWriter writer = new OutputStreamWriter(process.getOutputStream());
            for (String prop : props) {
                writer.write(prop + "\n");
            }
            writer.close();
            Thread.sleep(1000);
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    Toast.makeText(MainActivity.this, "Device Spoofed to Samsung S21", Toast.LENGTH_SHORT).show();
                }
            });
        } catch (Exception e) {}
    }

    private void nukeBanCache() {
        try {
            File ffDir = new File("/data/data/" + FF_MAX + "/files");
            if (ffDir.exists()) {
                File[] files = ffDir.listFiles();
                if (files != null) {
                    for (File f : files) {
                        if (f.getName().contains("ban") || f.getName().contains("config") || f.getName().contains("device")) {
                            f.delete();
                        }
                    }
                }
            }
        } catch (Exception e) {}
    }

    private void showFloatingLogo() {
        try {
            wm = (WindowManager) getSystemService(Context.WINDOW_SERVICE);
            int type = Build.VERSION.SDK_INT >= 26
                    ? WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY
                    : WindowManager.LayoutParams.TYPE_PHONE;

            TextView logo = new TextView(this);
            logo.setText("T");
            logo.setTextColor(Color.WHITE);
            logo.setTextSize(24);
            logo.setGravity(Gravity.CENTER);
            logo.setBackgroundColor(Color.parseColor("#FF2200"));

            final WindowManager.LayoutParams lp = new WindowManager.LayoutParams(
                    120, 120, type,
                    WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE,
                    android.graphics.PixelFormat.TRANSLUCENT);
            lp.gravity = Gravity.TOP | Gravity.LEFT;
            lp.x = 50;
            lp.y = 300;

            wm.addView(logo, lp);
            logoBtn = logo;
            Toast.makeText(this, "Panel Injected!", Toast.LENGTH_SHORT).show();
        } catch (Throwable t) {
            Toast.makeText(this, "Overlay failed", Toast.LENGTH_SHORT).show();
        }
    }

    private void launchGame() {
        new Handler(Looper.getMainLooper()).postDelayed(new Runnable() {
            @Override
            public void run() {
                try {
                    Intent i = getPackageManager().getLaunchIntentForPackage(FF_MAX);
                    if (i == null) i = getPackageManager().getLaunchIntentForPackage(FF_NORMAL);
                    if (i != null) {
                        i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                        startActivity(i);
                        moveTaskToBack(true); // Move to back so game is in front
                    }
                } catch (Throwable t) {}
            }
        }, 1000);
    }
}
