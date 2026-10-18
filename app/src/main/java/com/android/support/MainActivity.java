package com.android.support;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.view.Gravity;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;

public class MainActivity extends Activity {
    private static final String FF_MAX = "com.dts.freefiremax";
    private static final String FF_NORMAL = "com.dts.freefireth";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        LinearLayout root = new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setGravity(Gravity.CENTER);
        root.setBackgroundColor(Color.BLACK);
        root.setPadding(60, 60, 60, 60);

        TextView title = new TextView(this);
        title.setText("TRISPY VM");
        title.setTextColor(Color.RED);
        title.setTextSize(34);
        title.setGravity(Gravity.CENTER);
        root.addView(title);

        Button injectBtn = new Button(this);
        injectBtn.setText("LAUNCH GAME");
        injectBtn.setTextSize(18);
        
        injectBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                try {
                    Intent i = getPackageManager().getLaunchIntentForPackage(FF_MAX);
                    if (i == null) i = getPackageManager().getLaunchIntentForPackage(FF_NORMAL);
                    if (i != null) {
                        i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                        startActivity(i);
                        finish();
                    }
                } catch (Throwable t) {}
            }
        });
        root.addView(injectBtn);
        setContentView(root);
    }
}
