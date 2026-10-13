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
    private Paint ringPaint;
    private Bitmap bitmap;
    private Matrix matrix = new Matrix();
    private long startTime;
    private float density;

    public CircularLogoView(Context c, Bitmap bmp) {
        super(c);
        this.bitmap = bmp;
        this.density = c.getResources().getDisplayMetrics().density;
        this.startTime = System.currentTimeMillis();

        imagePaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        imagePaint.setFilterBitmap(true);

        borderPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        borderPaint.setStyle(Paint.Style.STROKE);
        borderPaint.setColor(Color.parseColor("#FF4500"));
        borderPaint.setStrokeWidth(4f * density);

        ringPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        ringPaint.setStyle(Paint.Style.STROKE);

        setLayerType(View.LAYER_TYPE_SOFTWARE, null);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        int w = getWidth();
        int h = getHeight();
        float cx = w / 2f;
        float cy = h / 2f;
        float baseR = Math.min(cx, cy) - 14f * density;

        long now = System.currentTimeMillis();
        long t = now - startTime;

        // outer pulse waves (2 rings offset in phase)
        for (int i = 0; i < 2; i++) {
            float phase = ((t + i * 900L) % 1800L) / 1800f;
            float waveR = baseR + phase * 22f * density;
            int alpha = (int)(180 * (1f - phase));
            if (alpha > 0) {
                ringPaint.setColor(Color.parseColor("#FF4500"));
                ringPaint.setAlpha(alpha);
                ringPaint.setStrokeWidth(2.5f * density);
                canvas.drawCircle(cx, cy, waveR, ringPaint);
            }
        }

        // inner glow ring (steady)
        ringPaint.setColor(Color.parseColor("#FF6600"));
        ringPaint.setAlpha(80);
        ringPaint.setStrokeWidth(9f * density);
        canvas.drawCircle(cx, cy, baseR + 3f * density, ringPaint);

        // circular image (perfectly cropped)
        if (bitmap != null) {
            matrix.reset();
            float scale = (baseR * 2f) / Math.min(bitmap.getWidth(), bitmap.getHeight());
            matrix.setScale(scale, scale);
            matrix.postTranslate(cx - bitmap.getWidth() * scale / 2f,
                                cy - bitmap.getHeight() * scale / 2f);
            BitmapShader shader = new BitmapShader(bitmap,
                Shader.TileMode.CLAMP, Shader.TileMode.CLAMP);
            shader.setLocalMatrix(matrix);
            imagePaint.setShader(shader);
            canvas.drawCircle(cx, cy, baseR, imagePaint);
        } else {
            imagePaint.setShader(null);
            imagePaint.setColor(Color.parseColor("#3a0505"));
            canvas.drawCircle(cx, cy, baseR, imagePaint);
        }

        // solid orange border
        canvas.drawCircle(cx, cy, baseR, borderPaint);

        // redraw loop for animation
        invalidate();
    }
}
