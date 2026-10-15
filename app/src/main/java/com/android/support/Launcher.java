package com.android.support;

import android.app.Service;
import android.content.Intent;
import android.os.Handler;
import android.os.IBinder;

public class Launcher extends Service {

    // Keep-alive service. MainActivity handles all overlay work.

    @Override
    public void onCreate() {
        super.onCreate();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        return START_NOT_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }
}
