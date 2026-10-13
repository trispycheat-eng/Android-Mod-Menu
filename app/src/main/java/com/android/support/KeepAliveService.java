package com.android.support;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.Service;
import android.content.Intent;
import android.os.Build;
import android.os.IBinder;

public class KeepAliveService extends Service {

    private static final String CH = "trispy_keepalive";
    private static final int ID = 9182;

    @Override
    public IBinder onBind(Intent intent) { return null; }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        try {
            if (Build.VERSION.SDK_INT >= 26) {
                NotificationChannel c = new NotificationChannel(
                    CH, "Trispy Panel", NotificationManager.IMPORTANCE_LOW);
                c.setShowBadge(false);
                NotificationManager nm = getSystemService(NotificationManager.class);
                if (nm != null) nm.createNotificationChannel(c);
            }
            Notification.Builder b;
            if (Build.VERSION.SDK_INT >= 26) b = new Notification.Builder(this, CH);
            else b = new Notification.Builder(this);
            b.setContentTitle("Trispy Panel active")
             .setContentText("t.me/TrispyCheats")
             .setSmallIcon(android.R.drawable.ic_lock_lock)
             .setOngoing(true);
            startForeground(ID, b.build());
        } catch (Throwable t) {}
        return START_STICKY;
    }
}
