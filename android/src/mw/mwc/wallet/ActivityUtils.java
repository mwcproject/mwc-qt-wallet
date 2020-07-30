package mw.mwc.wallet;

import android.content.Context;
import android.content.Intent;
import android.util.Log;
import android.content.BroadcastReceiver;
import android.content.IntentFilter;
import java.util.Timer;
import java.util.TimerTask;

public class ActivityUtils {

    Timer timer;

    private static native void sendToQt(String message);

    private static final String TAG = "ActivityUtils";
    public static final String BROADCAST_ACTION = "mw.mwc.wallet.broadcast";

    public void registerServiceBroadcastReceiver(Context context) {
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(BROADCAST_ACTION);
        context.registerReceiver(serviceMessageReceiver, intentFilter);
        Log.i(TAG, "Registered broadcast receiver");
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
                timer = new Timer();
                timer.schedule(new RemindTask(), 1000);
            }
        }

        class RemindTask extends TimerTask {
            public void run() {
                System.out.println("Time's up!");
                timer.cancel(); //Terminate the timer thread
                timer = new Timer();
                timer.schedule(new RemindTask(), 1000);
            }
        }
    };


}
