package mw.mwc.wallet;

import java.io.File;
import android.net.Uri;
import android.content.Intent;
import android.provider.DocumentsContract;
import android.content.ContentResolver;
import android.util.Log;
import android.app.Activity;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.FileOutputStream;
import java.io.FileInputStream;

public class QmlHelper
{
    private static final String TAG = "mwc-qt-wallet";

    // Type values:  .cfg -> text/plain     - it doesn't work this way.
    //  text/plain
    //  text/*
    public static Intent buildOpenFileIntent(String pickerInitialUri, String type ) {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType(type);

        // Optionally, specify a URI for the file that should appear in the
        // system file picker when it loads.
        if ( pickerInitialUri!=null && !pickerInitialUri.isEmpty()) {
            intent.putExtra(DocumentsContract.EXTRA_INITIAL_URI, Uri.parse(pickerInitialUri));
        }

        return intent;
    }

    // pickerInitialUri path where to create the file. Expected it to be Download directory
    // filename - file name that we want to save data in.
    // type - mime type, similar to what buildOpenFileIntent has
    public static Intent buildCreateFileIntent(String pickerInitialUri, String type, String filename ) {
            Intent intent = new Intent(Intent.ACTION_CREATE_DOCUMENT);
            intent.addCategory(Intent.CATEGORY_OPENABLE);
            intent.setType(type);
            intent.putExtra(Intent.EXTRA_TITLE, filename);

            // Optionally, specify a URI for the file that should appear in the
            // system file picker when it loads.
            if ( pickerInitialUri!=null && !pickerInitialUri.isEmpty()) {
                intent.putExtra(DocumentsContract.EXTRA_INITIAL_URI, Uri.parse(pickerInitialUri));
            }

            return intent;
    }

    public static boolean copyUriToFile( Activity act, String uriFN, String fsFN ) {
        InputStream is = null;
        try {
            ContentResolver cr = act.getContentResolver();
            is = cr.openInputStream(Uri.parse(uriFN));

            File file = new File(fsFN);
            int bytesCopy = 0;
            try (OutputStream output = new FileOutputStream(file)) {
                byte[] buffer = new byte[64 * 1024];
                int read;
                while ((read = is.read(buffer)) != -1) {
                    bytesCopy+=read;
                    output.write(buffer, 0, read);
                }
                output.flush();
                output.close();
                Log.e(TAG, "copyUriToFile copied " + bytesCopy + " from " + uriFN + " to " + fsFN);
            }
            return bytesCopy > 0;
        } catch (Exception ex) {
            Log.e(TAG, "copyUriToFile failed", ex);
            return false;
        } finally {
            if (is!=null) {
                try {
                    is.close();
                }
                catch (Exception ign) {}
            }
        }
    }

    public static boolean copyFileToUri( Activity act, String fsFN, String uriFN ) {
        OutputStream os = null;
        try {
            ContentResolver cr = act.getContentResolver();
            os = cr.openOutputStream(Uri.parse(uriFN));

            File file = new File(fsFN);
            int bytesCopy = 0;
            try (InputStream input = new FileInputStream(file)) {
                byte[] buffer = new byte[64 * 1024];
                int read;
                while ((read = input.read(buffer)) != -1) {
                    bytesCopy+=read;
                    os.write(buffer, 0, read);
                }
                os.flush();
                input.close();
                Log.e( TAG, "copyFileToUri copied " + bytesCopy + " from " + fsFN + " to " + uriFN );
            }
            return bytesCopy > 0;
        } catch (Exception ex) {
            Log.e(TAG, "copyFileToUri failed", ex);
            return false;
        } finally {
            if (os!=null) {
                try {
                    os.close();
                }
                catch (Exception ign) {}
            }
        }
    }


}