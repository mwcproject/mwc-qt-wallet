package mw.mwc.wallet;

import android.content.Context;
import android.content.Intent;
import android.util.Log;
import android.content.BroadcastReceiver;
import android.content.IntentFilter;
import android.os.PowerManager;
import android.provider.Settings;
import android.net.Uri;

// import java.util.Timer;
// import java.util.TimerTask;

public class ActivityUtils {

    // Timer timer;

    private static native void sendToQt(String message);

    private static final String TAG = "ActivityUtils";
    public static final String BROADCAST_ACTION = "mw.mwc.wallet.broadcast";

    public void registerServiceBroadcastReceiver(Context context) {
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(BROADCAST_ACTION);
        context.registerReceiver(serviceMessageReceiver, intentFilter);
        Log.i(TAG, "Registered broadcast receiver");

        // Requesting permissions
        Intent intent = new Intent();
        String packageName = context.getPackageName();
        PowerManager pm = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
        if (pm.isIgnoringBatteryOptimizations(packageName))
            intent.setAction(Settings.ACTION_IGNORE_BATTERY_OPTIMIZATION_SETTINGS);
        else {
            intent.setAction(Settings.ACTION_REQUEST_IGNORE_BATTERY_OPTIMIZATIONS);
            intent.setData(Uri.parse("package:" + packageName));
        }
        context.startActivity(intent);

        Log.i(TAG, "ACTION_IGNORE_BATTERY_OPTIMIZATION_SETTINGS permissions are requested");
    }

    private BroadcastReceiver serviceMessageReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Log.i(TAG, "In OnReceive broadcast receiver");
            if (BROADCAST_ACTION.equals(intent.getAction())) {
                String message = intent.getStringExtra("message");
                Log.i(TAG, "Service received: " + message);
                sendToQt(message);
                Log.i(TAG, "Service sent back message: " + message);
                // timer = new Timer();
                // timer.schedule(new RemindTask(), 1000);
            }
        }

        // class RemindTask extends TimerTask {
        //     public void run() {
        //         System.out.println("Time's up!");
        //         timer.cancel(); //Terminate the timer thread
        //         timer = new Timer();
        //         timer.schedule(new RemindTask(), 1000);
        //     }
        // }
    };


}
