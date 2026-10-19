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
import android.view.MotionEvent;
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
    private View menuPanel;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

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
        
        injectBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                injectBtn.setText("INJECTING...");
                v.setEnabled(false);
                
                // Run spoofing in background to prevent main thread freeze
                new Thread(new Runnable() {
                    @Override
                    public void run() {
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
            Thread.sleep(800);
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

            // Click listener to open the menu
            logo.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    if (menuPanel == null) {
                        showMenu();
                    } else {
                        hideMenu();
                    }
                }
            });

            wm.addView(logo, lp);
            logoBtn = logo;
        } catch (Throwable t) {}
    }

    private void showMenu() {
        try {
            int type = Build.VERSION.SDK_INT >= 26
                    ? WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY
                    : WindowManager.LayoutParams.TYPE_PHONE;

            TextView panel = new TextView(this);
            panel.setText("CHEAT MENU\n\nAimlock: OFF\nESP: OFF\n\n(Click T to close)");
            panel.setTextColor(Color.WHITE);
            panel.setTextSize(18);
            panel.setGravity(Gravity.CENTER);
            panel.setBackgroundColor(Color.parseColor("#CC000000"));
            panel.setPadding(40, 40, 40, 40);

            final WindowManager.LayoutParams lp = new WindowManager.LayoutParams(
                    WindowManager.LayoutParams.WRAP_CONTENT, 
                    WindowManager.LayoutParams.WRAP_CONTENT, 
                    type,
                    WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE,
                    android.graphics.PixelFormat.TRANSLUCENT);
            lp.gravity = Gravity.CENTER;

            wm.addView(panel, lp);
            menuPanel = panel;
        } catch (Throwable t) {}
    }

    private void hideMenu() {
        try {
            if (menuPanel != null) {
                wm.removeView(menuPanel);
                menuPanel = null;
            }
        } catch (Throwable t) {}
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
                        moveTaskToBack(true); 
                    }
                } catch (Throwable t) {}
            }
        }, 1000);
    }
                                   }
