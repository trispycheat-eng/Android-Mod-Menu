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
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;
import java.io.OutputStreamWriter;

public class MainActivity extends Activity {
    private static final String FF_MAX = "com.dts.freefiremax";
    private static final String FF_NORMAL = "com.dts.freefireth";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

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

        TextView sub = new TextView(this);
        sub.setText("VM Mobile Spoofer & Bypass");
        sub.setTextColor(Color.parseColor("#CC9966"));
        sub.setTextSize(14);
        sub.setGravity(Gravity.CENTER);
        sub.setPadding(0, 10, 0, 50);
        root.addView(sub);

        Button injectBtn = new Button(this);
        injectBtn.setText("INJECT & LAUNCH");
        injectBtn.setTextSize(18);
        injectBtn.setBackgroundColor(Color.parseColor("#FF2200"));
        injectBtn.setTextColor(Color.WHITE);
        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(-1, -2);
        injectBtn.setLayoutParams(params);
        
        injectBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                injectBtn.setText("SPOOFING DEVICE...");
                v.setEnabled(false);
                
                // 1. Nuke Ban Cache
                nukeBanCache();
                
                // 2. Spoof Device (PC Logo Bypass)
                spoofProperties();
                
                // 3. Start Panel & Game
                new Handler(Looper.getMainLooper()).postDelayed(new Runnable() {
                    @Override
                    public void run() {
                        startPanelAndGame();
                    }
                }, 1500);
            }
        });
        root.addView(injectBtn);
        setContentView(root);
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
            Toast.makeText(this, "Device Spoofed to Samsung S21", Toast.LENGTH_SHORT).show();
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

    private void startPanelAndGame() {
        // Show floating logo safely
        try {
            Main.StartWithoutPermission(this);
        } catch (Throwable t) {}
        
        try {
            Intent i = getPackageManager().getLaunchIntentForPackage(FF_MAX);
            if (i == null) i = getPackageManager().getLaunchIntentForPackage(FF_NORMAL);
            if (i != null) {
                i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                startActivity(i);
            }
        } catch (Throwable t) {}
    }
}
