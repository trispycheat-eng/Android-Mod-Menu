package com.android.support;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapShader;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Shader;
import android.view.View;

public class CircularLogoView extends View {

    private Paint imagePaint;
    private Paint borderPaint;
    private Paint glowPaint;
    private Bitmap bitmap;

    public CircularLogoView(Context c, Bitmap bmp) {
        super(c);
        this.bitmap = bmp;

        imagePaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        borderPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        borderPaint.setStyle(Paint.Style.STROKE);
        borderPaint.setColor(Color.parseColor("#FF4500"));
        borderPaint.setStrokeWidth(8);

        glowPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        glowPaint.setStyle(Paint.Style.STROKE);
        glowPaint.setColor(Color.parseColor("#FFFF6600"));
        glowPaint.setStrokeWidth(14);
        glowPaint.setAlpha(120);

        if (bitmap != null) {
            BitmapShader shader = new BitmapShader(bitmap,
                Shader.TileMode.CLAMP, Shader.TileMode.CLAMP);
            imagePaint.setShader(shader);
        }
        setLayerType(View.LAYER_TYPE_SOFTWARE, null);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        int w = getWidth();
        int h = getHeight();
        float cx = w / 2f;
        float cy = h / 2f;
        float radius = Math.min(cx, cy) - 12f;

        // outer glow ring
        canvas.drawCircle(cx, cy, radius + 10, glowPaint);

        // circular image (scaled)
        if (bitmap != null) {
            Matrix m = new Matrix();
            float scale = (radius * 2f) / Math.min(bitmap.getWidth(), bitmap.getHeight());
            m.setScale(scale, scale);
            m.postTranslate(cx - bitmap.getWidth() * scale / 2f,
                            cy - bitmap.getHeight() * scale / 2f);
            BitmapShader shader = new BitmapShader(bitmap,
                Shader.TileMode.CLAMP, Shader.TileMode.CLAMP);
            shader.setLocalMatrix(m);
            imagePaint.setShader(shader);
            canvas.drawCircle(cx, cy, radius, imagePaint);
        } else {
            imagePaint.setShader(null);
            imagePaint.setColor(Color.parseColor("#3a0505"));
            canvas.drawCircle(cx, cy, radius, imagePaint);
        }

        // border ring
        canvas.drawCircle(cx, cy, radius, borderPaint);
    }
}
