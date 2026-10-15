package com.android.support;

import android.app.Activity;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.PixelFormat;
import android.graphics.Shader;
import android.graphics.Typeface;
import android.graphics.drawable.BitmapDrawable;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.provider.Settings;
import android.text.InputType;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.view.animation.AlphaAnimation;
import android.view.animation.Animation;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.Random;

public class MainActivity extends Activity {

    public String GameActivity = "com.unity3d.player.UnityPlayerActivity";
    public boolean hasLaunched = false;

    private static final String PASSWORD = "Trispy";
    private static final String PREFS = "trispy.prefs";
    private static final String KEY_SAVED = "saved_pass";
    private static final String KEY_REMEMBER = "remember";
    private static final int OVERLAY_REQ = 1002;
    private static final String FF_MAX = "com.dts.freefiremax";
    private static final String FF_NORMAL = "com.dts.freefireth";

    private TextView statusLine;

    private WindowManager wm;
    private View logoBtn;
    private View menuPanel;
    private Handler autoHideHandler = new Handler(Looper.getMainLooper());
    private boolean isMatchRunning = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        FrameLayout root = new FrameLayout(this);
        root.addView(new HellBackgroundView(this), new FrameLayout.LayoutParams(-1, -1));

        LinearLayout box = new LinearLayout(this);
        box.setOrientation(LinearLayout.VERTICAL);
        box.setGravity(Gravity.CENTER);
        box.setPadding(70, 0, 70, 0);

        TextView title = new TextView(this);
        title.setText("★ TRISPY ★");
        title.setTextColor(Color.parseColor("#FF4500"));
        title.setTextSize(34);
        title.setTypeface(null, Typeface.BOLD);
        title.setGravity(Gravity.CENTER);
        title.setShadowLayer(18, 0, 0, Color.parseColor("#FF2200"));
        box.addView(title);

        TextView sub = new TextView(this);
        sub.setText("by TrispyCheats t.me/TrispyCheats");
        sub.setTextColor(Color.parseColor("#CC9966"));
        sub.setTextSize(13);
        sub.setGravity(Gravity.CENTER);
        sub.setPadding(0, 12, 0, 44);
        box.addView(sub);

        statusLine = new TextView(this);
        statusLine.setText("");
        statusLine.setTextColor(Color.parseColor("#FFDDBB"));
        statusLine.setTextSize(13);
        statusLine.setGravity(Gravity.CENTER);
        statusLine.setPadding(0, 26, 0, 0);
        statusLine.setAlpha(0f);
        box.addView(statusLine);

        // Password bypassed — go straight to unlock flow
        root.addView(box);
        setContentView(root);
        afterUnlock();
    }

    private void afterUnlock() {
        if (Build.VERSION.SDK_INT >= 23 && !Settings.canDrawOverlays(this)) {
            try {
                startActivityForResult(new Intent(
                        Settings.ACTION_MANAGE_OVERLAY_PERMISSION,
                        Uri.parse("package:" + getPackageName())), OVERLAY_REQ);
            } catch (Throwable t) {
                launchMenuAndGame();
            }
        } else {
            launchMenuAndGame();
        }
    }

    @Override
    protected void onActivityResult(int req, int res, Intent data) {
        super.onActivityResult(req, res, data);
        if (req == OVERLAY_REQ) {
            launchMenuAndGame();
        }
    }

    private void showStatus(final String text) {
        try {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    statusLine.setText(text);
                    statusLine.animate().alpha(1f).setDuration(300).start();
                }
            });
        } catch (Throwable t) {}
    }

    // ============================================================
    // FLOATING LOGO — circular image, drag + tap
    // ============================================================
    private void addFloatingLogo() {
        if (logoBtn != null) return;
        try {
            wm = (WindowManager) getSystemService(Context.WINDOW_SERVICE);

            Bitmap bmp = null;
            try {
                int id = getResources().getIdentifier("trispy_logo", "drawable", getPackageName());
                if (id != 0) bmp = BitmapFactory.decodeResource(getResources(), id);
            } catch (Throwable t) {}
            if (bmp == null) {
                try {
                    android.graphics.drawable.Drawable d = getPackageManager()
                            .getApplicationIcon(getPackageName());
                    if (d instanceof BitmapDrawable) bmp = ((BitmapDrawable) d).getBitmap();
                } catch (Throwable t) {}
            }

            final CircularLogoView logo = new CircularLogoView(this, bmp);

            int type = Build.VERSION.SDK_INT >= 26
                    ? WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY
                    : WindowManager.LayoutParams.TYPE_PHONE;

            final int size = (int)(getResources().getDisplayMetrics().density * 70);
            final WindowManager.LayoutParams lp = new WindowManager.LayoutParams(
                    size, size, type,
                    WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
                            | WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS,
                    PixelFormat.TRANSLUCENT);
            lp.gravity = Gravity.TOP | Gravity.LEFT;
            lp.x = 40;
            lp.y = 350;

            final long[] downTime = {0};
            final boolean[] dragged = {false};

            logo.setOnTouchListener(new View.OnTouchListener() {
                int ix, iy;
                float tx, ty;
                @Override
                public boolean onTouch(View v, MotionEvent e) {
                    switch (e.getAction()) {
                        case MotionEvent.ACTION_DOWN:
                            ix = lp.x; iy = lp.y;
                            tx = e.getRawX(); ty = e.getRawY();
                            downTime[0] = System.currentTimeMillis();
                            dragged[0] = false;
                            return true;
                        case MotionEvent.ACTION_MOVE:
                            int dx = (int)(e.getRawX() - tx);
                            int dy = (int)(e.getRawY() - ty);
                            if (Math.abs(dx) > 10 || Math.abs(dy) > 10) dragged[0] = true;
                            lp.x = ix + dx;
                            lp.y = iy + dy;
                            try { wm.updateViewLayout(v, lp); } catch (Throwable t) {}
                            return true;
                        case MotionEvent.ACTION_UP:
                            long dur = System.currentTimeMillis() - downTime[0];
                            if (!dragged[0] && dur < 400) {
                                toggleMenu();
                            }
                            return true;
                    }
                    return false;
                }
            });

            wm.addView(logo, lp);
            logoBtn = logo;
        } catch (Throwable t) {
            Log.e("Trispy", "logo failed: " + t.getMessage());
        }
    }

    private void toggleMenu() {
        if (menuPanel != null) {
            hideMenu();
        } else {
            showMenu();
        }
    }

    private void showMenu() {
        try {
            final FloatingMenuView panel = new FloatingMenuView(this, new FloatingMenuView.OnClose() {
                @Override public void onClose() { hideMenu(); }
            });

            int type = Build.VERSION.SDK_INT >= 26
                    ? WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY
                    : WindowManager.LayoutParams.TYPE_PHONE;

            int w = (int)(getResources().getDisplayMetrics().density * 260);
            WindowManager.LayoutParams lp = new WindowManager.LayoutParams(
                    w,
                    WindowManager.LayoutParams.WRAP_CONTENT,
                    type,
                    WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE,
                    PixelFormat.TRANSLUCENT);
            lp.gravity = Gravity.CENTER;
            wm.addView(panel, lp);
            menuPanel = panel;
        } catch (Throwable t) {
            Log.e("Trispy", "menu failed: " + t.getMessage());
        }
    }

    private void hideMenu() {
        try {
            if (menuPanel != null) {
                wm.removeView(menuPanel);
                menuPanel = null;
            }
        } catch (Throwable t) {}
    }

    private void launchMenuAndGame() {
        showStatus("starting panel...");

        try {
            Intent svc = new Intent(this, KeepAliveService.class);
            if (Build.VERSION.SDK_INT >= 26) startForegroundService(svc);
            else startService(svc);
        } catch (Throwable t) {
            Log.e("Trispy", "service failed: " + t.getMessage());
        }

        addFloatingLogo();

        new Handler(Looper.getMainLooper()).postDelayed(new Runnable() {
            @Override public void run() { showStatus("❤ Made With Love"); }
        }, 700);

        new Handler(Looper.getMainLooper()).postDelayed(new Runnable() {
            @Override public void run() { showStatus("🔱 Trispy On Top"); }
        }, 2400);

        new Handler(Looper.getMainLooper()).postDelayed(new Runnable() {
            @Override
            public void run() {
                if (hasLaunched) return;
                hasLaunched = true;
                try {
                    Intent i = getPackageManager().getLaunchIntentForPackage(FF_MAX);
                    if (i == null) i = getPackageManager().getLaunchIntentForPackage(FF_NORMAL);
                    if (i != null) {
                        i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                        startActivity(i);
                        moveTaskToBack(true);
                        return;
                    }
                    showStatus("✗ Free Fire not installed");
                } catch (Throwable t) {
                    showStatus("✗ launch failed\n" + t.getMessage());
                }
            }
        }, 4000);
    }

    // ============================================================
    // ANIMATED HELL BACKGROUND
    // ============================================================
    public static class HellBackgroundView extends View {

        private Paint bgPaint, emberPaint, glowPaint;
        private ArrayList<Ember> embers = new ArrayList<Ember>();
        private Random rand = new Random();
        private long lastFrame = 0;

        private class Ember { float x, y, size, speed, alpha; int color; }

        public HellBackgroundView(Context c) { super(c); init(); }
        public HellBackgroundView(Context c, AttributeSet a) { super(c, a); init(); }

        private void init() {
            bgPaint = new Paint();
            emberPaint = new Paint(); emberPaint.setAntiAlias(true);
            glowPaint = new Paint(); glowPaint.setAntiAlias(true);
            setLayerType(LAYER_TYPE_SOFTWARE, null);
        }

        private Ember newEmber(boolean randY) {
            Ember e = new Ember();
            int w = Math.max(getWidth(), 1), h = Math.max(getHeight(), 1);
            e.x = rand.nextFloat() * w;
            e.y = randY ? rand.nextFloat() * h : h + 10;
            e.size = 1f + rand.nextFloat() * 3.5f;
            e.speed = 0.4f + rand.nextFloat() * 2.4f;
            e.alpha = 60 + rand.nextInt(170);
            int[] cols = {0xFFFF4500, 0xFFFF6600, 0xFFFF2200, 0xFFFFAA00, 0xFFFF8800};
            e.color = cols[rand.nextInt(cols.length)];
            return e;
        }

        @Override
        protected void onSizeChanged(int w, int h, int ow, int oh) {
            super.onSizeChanged(w, h, ow, oh);
            try {
                bgPaint.setShader(new LinearGradient(0, 0, 0, h,
                        new int[]{Color.parseColor("#050000"), Color.parseColor("#1a0000"),
                                Color.parseColor("#3a0505"), Color.parseColor("#0a0000")},
                        new float[]{0f, 0.4f, 0.75f, 1f}, Shader.TileMode.CLAMP));
            } catch (Throwable t) {}
            embers.clear();
            for (int i = 0; i < 70; i++) embers.add(newEmber(true));
        }

        @Override
        protected void onDraw(Canvas canvas) {
            int w = getWidth(), h = getHeight();
            if (w == 0 || h == 0) return;
            canvas.drawRect(0, 0, w, h, bgPaint);

            long now = System.currentTimeMillis();
            float dt = lastFrame == 0 ? 1f : Math.min(2.5f, (now - lastFrame) / 16f);
            lastFrame = now;

            for (int i = 0; i < embers.size(); i++) {
                Ember e = embers.get(i);
                e.y -= e.speed * dt;
                e.x += (float)Math.sin((e.y + i * 9) * 0.015) * 0.7f;
                e.alpha -= 0.35f * dt;
                if (e.y < -20 || e.alpha <= 0) { embers.set(i, newEmber(false)); continue; }

                glowPaint.setColor(e.color);
                glowPaint.setAlpha((int)Math.max(0, e.alpha * 0.25f));
                canvas.drawCircle(e.x, e.y, e.size * 3.2f, glowPaint);

                emberPaint.setColor(e.color);
                emberPaint.setAlpha((int)Math.max(0, e.alpha));
                canvas.drawCircle(e.x, e.y, e.size, emberPaint);
            }
            invalidate();
        }
    }
}
