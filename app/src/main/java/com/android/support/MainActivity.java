package com.android.support;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
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
                
                // Run spoofing in background to prevent freeze
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        nukeBanCache();
                        spoofProperties();
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                startPanelAndGame();
                            }
                        });
                    }
                }).start();
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
            // Give it a second to apply
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
