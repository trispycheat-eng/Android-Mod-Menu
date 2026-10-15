package com.android.support;

import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.graphics.Typeface;
import android.graphics.drawable.GradientDrawable;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.Switch;
import android.widget.TextView;

public class FloatingMenuView extends LinearLayout {

    public interface OnClose { void onClose(); }
    private OnClose closeCb;
    private WindowManager wm;
    private WindowManager.LayoutParams lp;

    public FloatingMenuView(Context c, WindowManager wm, WindowManager.LayoutParams lp, OnClose cb) {
        super(c);
        this.closeCb = cb;
        this.wm = wm;
        this.lp = lp;
        setOrientation(VERTICAL);
        setPadding(28, 22, 28, 22);

        GradientDrawable bg = new GradientDrawable();
        bg.setColor(0xEE0A0000);
        bg.setStroke(3, Color.parseColor("#FF4500"));
        bg.setCornerRadius(22);
        setBackground(bg);

        final SharedPreferences p = c.getSharedPreferences("trispy_toggles", Context.MODE_PRIVATE);

        // header — draggable
        LinearLayout head = new LinearLayout(c);
        head.setOrientation(HORIZONTAL);
        head.setGravity(Gravity.CENTER_VERTICAL);

        TextView t = new TextView(c);
        t.setText("★ TRISPY ★");
        t.setTextColor(Color.parseColor("#FF4500"));
        t.setTextSize(16);
        t.setTypeface(null, Typeface.BOLD);
        t.setShadowLayer(12, 0, 0, Color.parseColor("#FF2200"));
        head.addView(t, new LinearLayout.LayoutParams(0, -2, 1f));

        TextView x = new TextView(c);
        x.setText("✕");
        x.setTextColor(Color.parseColor("#FFDDBB"));
        x.setTextSize(18);
        x.setPadding(24, 0, 0, 0);
        x.setOnClickListener(new View.OnClickListener() {
            @Override public void onClick(View v) { if (closeCb != null) closeCb.onClose(); }
        });
        head.addView(x);

        head.setOnTouchListener(new View.OnTouchListener() {
            int ix, iy; float tx, ty;
            @Override public boolean onTouch(View v, MotionEvent e) {
                switch (e.getAction()) {
                    case MotionEvent.ACTION_DOWN:
                        ix = lp.x; iy = lp.y;
                        tx = e.getRawX(); ty = e.getRawY();
                        return true;
                    case MotionEvent.ACTION_MOVE:
                        lp.x = ix + (int)(e.getRawX() - tx);
                        lp.y = iy + (int)(e.getRawY() - ty);
                        try { wm.updateViewLayout(FloatingMenuView.this, lp); } catch (Throwable t) {}
                        return true;
                }
                return false;
            }
        });
        addView(head);

        addSection("AIM");
        addSwitch(p, "aim_head", "Aimlock Head", 2, "in-match only");
        addSlider(p, "aim_smooth", "Aim Smooth", 3, 1, 20, "1=instant 20=slow");
        addSlider(p, "aim_fov", "Aim FOV", 4, 1, 360, "range in degrees");
        addSwitch(p, "ignore_knocked", "Ignore Knocked", 5, "skip downed");
        addSwitch(p, "visible_check", "Visible Only", 6, "only visible targets");

        addSection("VISUAL");
        addSwitch(p, "esp_enemy", "ESP Enemy", 8, "in-match only");
        addSwitch(p, "esp_glow", "ESP Glow", 9, "highlight enemies");
        addSwitch(p, "esp_line", "ESP Line", 10, "line to each enemy");
        addSwitch(p, "esp_box", "ESP Box", 11, "box around enemies");
        addSlider(p, "esp_range", "ESP Range", 12, 1, 1000, "max distance");

        addSection("SYSTEM");
        addSwitch(p, "anti_cheat", "Anti-Cheat Bypass", 14, "ON in lobby");
        addButton("Clear Reports", 16);
        addButton("Wipe Cache", 17);
        addButton("STOP & WIPE", 18);
    }

    private void addSection(String name) {
        TextView s = new TextView(getContext());
        s.setText("— " + name + " —");
        s.setTextColor(Color.parseColor("#CC9966"));
        s.setTextSize(11);
        s.setGravity(Gravity.CENTER);
        s.setPadding(0, 14, 0, 4);
        addView(s);
    }

    private void addSwitch(SharedPreferences p, final String key, final String label, final int id, String hint) {
        LinearLayout row = new LinearLayout(getContext());
        row.setOrientation(HORIZONTAL);
        row.setGravity(Gravity.CENTER_VERTICAL);
        row.setPadding(6, 4, 6, 4);

        LinearLayout col = new LinearLayout(getContext());
        col.setOrientation(VERTICAL);
        TextView tv = new TextView(getContext());
        tv.setText(label);
        tv.setTextColor(Color.parseColor("#FFDDBB"));
        tv.setTextSize(12);
        col.addView(tv);

        TextView h = new TextView(getContext());
        h.setText("▶ " + hint);
        h.setTextColor(Color.parseColor("#887755"));
        h.setTextSize(9);
        col.addView(h);

        row.addView(col, new LinearLayout.LayoutParams(0, -2, 1f));

        final Switch sw = new Switch(getContext());
        sw.setChecked(p.getBoolean(key, false));
        sw.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override public void onCheckedChanged(CompoundButton b, boolean v) {
                p.edit().putBoolean(key, v).apply();
                Menu.Changes(getContext(), label, id, 0, v, "");
            }
        });
        row.addView(sw);
        addView(row);
    }

    private void addSlider(final SharedPreferences p, final String key, final String label,
                           final int id, final int min, final int max, String hint) {
        LinearLayout box = new LinearLayout(getContext());
        box.setOrientation(VERTICAL);
        box.setPadding(6, 4, 6, 4);

        final TextView tv = new TextView(getContext());
        int val = p.getInt(key, min);
        tv.setText(label + ": " + val);
        tv.setTextColor(Color.parseColor("#FFDDBB"));
        tv.setTextSize(12);
        box.addView(tv);

        TextView h = new TextView(getContext());
        h.setText("▶ " + hint);
        h.setTextColor(Color.parseColor("#887755"));
        h.setTextSize(9);
        box.addView(h);

        SeekBar sb = new SeekBar(getContext());
        sb.setMax(max - min);
        sb.setProgress(val - min);
        sb.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            public void onProgressChanged(SeekBar s, int prog, boolean fromUser) {
                int v = prog + min;
                tv.setText(label + ": " + v);
                p.edit().putInt(key, v).apply();
                if (fromUser) Menu.Changes(getContext(), label, id, v, false, "");
            }
            public void onStartTrackingTouch(SeekBar s) {}
            public void onStopTrackingTouch(SeekBar s) {}
        });
        box.addView(sb);
        addView(box);
    }

    private void addButton(String label, final int id) {
        Button b = new Button(getContext());
        b.setText(label);
        b.setTextColor(Color.parseColor("#FFEECC"));
        b.setTextSize(12);
        b.setBackgroundColor(Color.parseColor("#3a0505"));
        b.setOnClickListener(new View.OnClickListener() {
            @Override public void onClick(View v) {
                Menu.Changes(getContext(), "btn", id, 0, false, "");
            }
        });
        LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(-1, -2);
        lp.topMargin = 6;
        b.setLayoutParams(lp);
        addView(b);
    }
}
