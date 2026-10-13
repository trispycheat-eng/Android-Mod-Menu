package com.android.support;

import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.graphics.Typeface;
import android.graphics.drawable.GradientDrawable;
import android.view.Gravity;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.Switch;
import android.widget.TextView;

public class FloatingMenuView extends LinearLayout {

    public interface OnClose { void onClose(); }
    private OnClose closeCb;

    public FloatingMenuView(Context c, OnClose cb) {
        super(c);
        this.closeCb = cb;
        setOrientation(VERTICAL);
        setPadding(36, 28, 36, 28);

        GradientDrawable bg = new GradientDrawable();
        bg.setColor(0xEE0A0000);
        bg.setStroke(3, Color.parseColor("#FF4500"));
        bg.setCornerRadius(28);
        setBackground(bg);

        SharedPreferences p = c.getSharedPreferences("trispy_toggles", Context.MODE_PRIVATE);

        // ── header ──
        LinearLayout head = new LinearLayout(c);
        head.setOrientation(HORIZONTAL);
        head.setGravity(Gravity.CENTER_VERTICAL);

        TextView t = new TextView(c);
        t.setText("⋆ TRISPY ⋆");
        t.setTextColor(Color.parseColor("#FF4500"));
        t.setTextSize(18);
        t.setTypeface(null, Typeface.BOLD);
        t.setShadowLayer(12, 0, 0, Color.parseColor("#FF2200"));
        head.addView(t, new LinearLayout.LayoutParams(0, -2, 1f));

        TextView x = new TextView(c);
        x.setText("✕");
        x.setTextColor(Color.parseColor("#FFDDBB"));
        x.setTextSize(20);
        x.setPadding(24, 0, 0, 0);
        x.setOnClickListener(new OnClickListener() {
            @Override public void onClick(View v) { if (closeCb != null) closeCb.onClose(); }
        });
        head.addView(x);
        addView(head);

        addSection("AIM");
        addSwitch(p, "aim_head", "Aimlock Head");
        addSlider(p, "aim_smooth", "Aim Smooth", 1, 20);
        addSlider(p, "aim_fov", "Aim FOV", 1, 360);

        addSection("VISUAL");
        addSwitch(p, "esp_enemy", "ESP Enemy");
        addSwitch(p, "esp_glow", "ESP Glow");
        addSwitch(p, "esp_line", "ESP Line");
        addSwitch(p, "esp_box", "ESP Box");
        addSlider(p, "esp_range", "ESP Range", 1, 1000);

        addSection("SYSTEM");
        addSwitch(p, "anti_cheat", "Anti-Cheat Bypass");
        addButton("Clear Reports");
        addButton("Wipe Cache");
        addButton("STOP & WIPE");
    }

    private void addSection(String name) {
        TextView s = new TextView(getContext());
        s.setText("— " + name + " —");
        s.setTextColor(Color.parseColor("#CC9966"));
        s.setTextSize(12);
        s.setGravity(Gravity.CENTER);
        s.setPadding(0, 20, 0, 10);
        addView(s);
    }

    private void addSwitch(final SharedPreferences p, final String key, String label) {
        LinearLayout row = new LinearLayout(getContext());
        row.setOrientation(HORIZONTAL);
        row.setGravity(Gravity.CENTER_VERTICAL);
        row.setPadding(8, 8, 8, 8);

        TextView tv = new TextView(getContext());
        tv.setText(label);
        tv.setTextColor(Color.parseColor("#FFDDBB"));
        tv.setTextSize(13);
        row.addView(tv, new LinearLayout.LayoutParams(0, -2, 1f));

        final Switch sw = new Switch(getContext());
        sw.setChecked(p.getBoolean(key, false));
        sw.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override public void onCheckedChanged(CompoundButton b, boolean v) {
                p.edit().putBoolean(key, v).apply();
            }
        });
        row.addView(sw);
        addView(row);
    }

    private void addSlider(final SharedPreferences p, final String key, String label,
                           final int min, final int max) {
        LinearLayout box = new LinearLayout(getContext());
        box.setOrientation(VERTICAL);
        box.setPadding(8, 8, 8, 8);

        final TextView tv = new TextView(getContext());
        int val = p.getInt(key, min);
        tv.setText(label + ": " + val);
        tv.setTextColor(Color.parseColor("#FFDDBB"));
        tv.setTextSize(13);
        box.addView(tv);

        SeekBar sb = new SeekBar(getContext());
        sb.setMax(max - min);
        sb.setProgress(val - min);
        sb.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override public void onProgressChanged(SeekBar s, int prog, boolean fromUser) {
                int v = prog + min;
                tv.setText(label.split(":")[0] + ": " + v);
                p.edit().putInt(key, v).apply();
            }
            @Override public void onStartTrackingTouch(SeekBar s) {}
            @Override public void onStopTrackingTouch(SeekBar s) {}
        });
        box.addView(sb);
        addView(box);
    }

    private void addButton(String label) {
        Button b = new Button(getContext());
        b.setText(label);
        b.setTextColor(Color.parseColor("#FFEECC"));
        b.setTextSize(12);
        b.setBackgroundColor(Color.parseColor("#3a0505"));
        LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(-1, -2);
        lp.topMargin = 10;
        b.setLayoutParams(lp);
        addView(b);
    }
}
