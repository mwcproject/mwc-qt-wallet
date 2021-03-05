package mw.mwc.wallet;

import android.app.Activity;
import android.app.ActivityManager;
import android.content.Context;
import android.content.Intent;
import android.util.Log;
import android.content.BroadcastReceiver;
import android.content.IntentFilter;
import android.os.PowerManager;
import android.provider.Settings;
import android.net.Uri;
import android.support.v4.content.ContextCompat; 
import android.widget.Toast; 
import android.content.pm.PackageManager;
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

        checkPermission(context, android.Manifest.permission.WRITE_EXTERNAL_STORAGE, 100);
        checkPermission(context, android.Manifest.permission.READ_EXTERNAL_STORAGE, 100);

/*        // Requesting permissions
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

        checkPermission(context, android.Manifest.permission.WRITE_EXTERNAL_STORAGE, 100);

        Log.i(TAG, "ACTION_IGNORE_BATTERY_OPTIMIZATION_SETTINGS permissions are requested");*/
    }
    
    // Function to check and request permission. 
    public void checkPermission(Context context, String permission, int requestCode) 
    {
        if (ContextCompat.checkSelfPermission(context, permission) == PackageManager.PERMISSION_DENIED) { 
            // Requesting the permission 
            ((Activity)context).requestPermissions(new String[] { permission }, requestCode); 
        } 
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
