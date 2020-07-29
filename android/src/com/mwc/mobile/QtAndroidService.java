package com.mwc.mobile;

import android.content.Context;
import android.content.Intent;
import android.util.Log;
import android.os.IBinder;
import org.qtproject.qt5.android.bindings.QtService;
import android.content.IntentFilter;

public class QtAndroidService extends QtService
{
    private static final String TAG = "QtAndroidService";

    @Override
    public void onCreate() {
        super.onCreate();
        Log.i(TAG, "Creating Service");
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.i(TAG, "Destroying Service");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        int ret = super.onStartCommand(intent, flags, startId);

        String message = new String(intent.getByteArrayExtra("message"));
        Intent sendToUiIntent = new Intent();
        sendToUiIntent.setAction(ActivityUtils.BROADCAST_ACTION);
        sendToUiIntent.putExtra("message", message);
        Log.i(TAG, "Service sending broadcast");
        sendBroadcast(sendToUiIntent);

        return ret;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return super.onBind(intent);
    }
}
