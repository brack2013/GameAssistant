package com.monect.gameassistant;

import java.io.DataOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.text.DecimalFormat;
import java.util.List;

import android.app.Activity;
import android.app.ActivityManager;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.Configuration;
import android.hardware.display.DisplayManager;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Environment;
import android.os.Handler;
import android.preference.PreferenceManager;
import android.util.DisplayMetrics;
import android.view.Gravity;
import android.view.Surface;
import android.view.WindowManager;
import android.widget.Toast;

public class HelperClass {

    public static void EnsureDir(String path) {
        File file = new File(path);
        if (!file.exists()) {
            file.mkdirs();
        }
    }

    public static int GetAppVer(Context context) {
        int iver = 0;
        PackageManager pm = context.getPackageManager();
        try {
            PackageInfo pi = pm.getPackageInfo("com.monect.portable", 0);
            iver = pi.versionCode;
        } catch (NameNotFoundException e) {
            e.printStackTrace();
        }
        return iver;
    }

    public static String GetAppVerName(Context context) {
        String iver = "";
        PackageManager pm = context.getPackageManager();
        try {
            PackageInfo pi = pm.getPackageInfo("com.monect.portable", 0);
            iver = pi.versionName;
        } catch (NameNotFoundException e) {
            e.printStackTrace();
        }
        return iver;
    }

    public static float byte2float(byte[] b, int index) {
        int l;
        l = b[index + 0];
        l &= 0xff;
        l |= ((long) b[index + 1] << 8);
        l &= 0xffff;
        l |= ((long) b[index + 2] << 16);
        l &= 0xffffff;
        l |= ((long) b[index + 3] << 24);
        return Float.intBitsToFloat(l);
    }

    public static int byteArrayToInt(byte[] b, int offset) {
        int value = 0;
        for (int i = 0; i < 4; i++) {
            int shift = (4 - 1 - i) * 8;
            value += (b[i + offset] & 0x000000FF) << shift;
        }
        return value;
    }

    public static long byteArrayToLong(byte[] b, int offset, int ilen) {
        long value = 0;
        for (int i = 0; i < ilen; i++) {
            int shift = (ilen - 1 - i) * 8;
            long tem = b[i + offset] & 0x00000000000000FF;
            value += tem << shift;
        }
        return value;
    }

    // input time in seconds
    public static String FormatTimeSpan(int time) {
        int hour = time / 3600;
        int minut = (time - (hour * 3600)) / 60;
        int second = time - (hour * 3600) - (minut * 60);
        if (hour != 0) {
            return String.format("%02d:%02d:%02d", hour, minut, second);
        } else {
            return String.format("%02d:%02d", minut, second);
        }
    }

    public static String[] filesize(long size) {
        String str = "";
        float fsize = size;
        if (fsize >= 1024) {
            str = "KB";
            fsize /= 1024;
            if (fsize >= 1024) {
                str = "MB";
                fsize /= 1024;
            }
            if (fsize >= 1024) {
                str = "GB";
                fsize /= 1024;
            }
        }
        String result[] = new String[2];
        result[0] = new DecimalFormat("###,###,###.##").format(fsize);
        result[1] = str;
        return result;
    }

    public static void copyResourceFile(Context context, int rid,
                                        String targetFile) {
        InputStream fin = context.getResources().openRawResource(rid);
        FileOutputStream fos = null;
        int length;
        try {
            fos = new FileOutputStream(targetFile);
            byte[] buffer = new byte[1024];
            while ((length = fin.read(buffer)) != -1) {
                fos.write(buffer, 0, length);
            }
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            if (fin != null) {
                try {
                    fin.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            if (fos != null) {
                try {
                    fos.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }


    public static boolean Istablet(Context context) {
        boolean btablet = false;
        WindowManager windowManager = (WindowManager) context
                .getSystemService(Context.WINDOW_SERVICE);

        Configuration config = context.getResources().getConfiguration();

        int rotation = windowManager.getDefaultDisplay().getRotation();

        if (((rotation == Surface.ROTATION_0 || rotation == Surface.ROTATION_180) && config.orientation == Configuration.ORIENTATION_LANDSCAPE)
                || ((rotation == Surface.ROTATION_90 || rotation == Surface.ROTATION_270) && config.orientation == Configuration.ORIENTATION_PORTRAIT)) {
            btablet = true;
        } else {
            btablet = false;
        }
        return btablet;
    }

    public static boolean IsVIPValid(Context context) {

        // Get preference
        boolean bvip = false;
        SharedPreferences prefs = PreferenceManager
                .getDefaultSharedPreferences(context);
        long buytime = prefs.getLong("point purchase time", 0);
        long expiretime = prefs.getLong("point expire time", 0);
        if (expiretime > System.currentTimeMillis()
                && buytime < System.currentTimeMillis()) {
            bvip = true;
        }

        // String android_id = Secure.getString(context.getContentResolver(),
        // Secure.ANDROID_ID);
        // if (android_id.compareTo("37311cc65cafac12") == 0) {
        // bvip = true;
        // }
        return bvip;
    }

    public static String intToIp(int i) {

        return (i & 0xFF) + "." +

                ((i >> 8) & 0xFF) + "." +

                ((i >> 16) & 0xFF) + "." +

                (i >> 24 & 0xFF);

    }

    public static int screenWidth;
    public static int screenHeight;

    public static String GetLocalIP(Context context) {
        WifiManager wifiManager = (WifiManager) context
                .getSystemService(Context.WIFI_SERVICE);
        String ip = "";
        if (wifiManager.isWifiEnabled()) {

            WifiInfo wifiInfo = wifiManager.getConnectionInfo();

            int ipAddress = wifiInfo.getIpAddress();

            ip = intToIp(ipAddress);
        }
        return ip;
    }

    public static boolean IsServiceRunning(Context conetext, String claaname) {
        boolean isRunning = false;
        ActivityManager activityManager = (ActivityManager) conetext
                .getSystemService(Context.ACTIVITY_SERVICE);

        List<ActivityManager.RunningServiceInfo> serviceList = activityManager.getRunningServices(200);
        if (serviceList.size() <= 0) {
            return false;
        }

        for (int i = 0; i < serviceList.size(); i++) {
            if (serviceList.get(i).service.getClassName().equals(claaname)) {
                isRunning = true;
                break;
            }
        }

        return isRunning;
    }

    protected static boolean runRootCommand(String command) {

        Process process = null;
        DataOutputStream os = null;
        try {
            process = Runtime.getRuntime().exec("su");
            os = new DataOutputStream(process.getOutputStream());
            os.writeBytes(command + "\n");
            os.writeBytes("exit\n");
            os.flush();
            process.waitFor();
        } catch (Exception e) {
            return false;
        } finally {
            try {
                if (os != null) {
                    os.close();
                }
                process.destroy();
            } catch (Exception e) {
            }
        }
        return true;
    }

    public static int getScreenOrientation(Activity activity) {
        int rotation = activity.getWindowManager().getDefaultDisplay()
                .getRotation();
        DisplayMetrics dm = new DisplayMetrics();
        activity.getWindowManager().getDefaultDisplay().getMetrics(dm);
        int width = dm.widthPixels;
        int height = dm.heightPixels;
        int orientation;
        // if the device's natural orientation is portrait:
        if ((rotation == Surface.ROTATION_0 || rotation == Surface.ROTATION_180)
                && height > width
                || (rotation == Surface.ROTATION_90 || rotation == Surface.ROTATION_270)
                && width > height) {
            switch (rotation) {
                case Surface.ROTATION_0:
                    orientation = ActivityInfo.SCREEN_ORIENTATION_PORTRAIT;
                    break;
                case Surface.ROTATION_90:
                    orientation = ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE;
                    break;
                case Surface.ROTATION_180:
                    orientation = ActivityInfo.SCREEN_ORIENTATION_REVERSE_PORTRAIT;
                    break;
                case Surface.ROTATION_270:
                    orientation = ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE;
                    break;
                default:
                    orientation = ActivityInfo.SCREEN_ORIENTATION_PORTRAIT;
                    break;
            }
        }
        // if the device's natural orientation is landscape or if the device
        // is square:
        else {
            switch (rotation) {
                case Surface.ROTATION_0:
                    orientation = ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE;
                    break;
                case Surface.ROTATION_90:
                    orientation = ActivityInfo.SCREEN_ORIENTATION_PORTRAIT;
                    break;
                case Surface.ROTATION_180:
                    orientation = ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE;
                    break;
                case Surface.ROTATION_270:
                    orientation = ActivityInfo.SCREEN_ORIENTATION_REVERSE_PORTRAIT;
                    break;
                default:
                    orientation = ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE;
                    break;
            }
        }

        return orientation;
    }

    public static void KeepOrientation(Activity activity) {
        activity.setRequestedOrientation(getScreenOrientation(activity));
    }

    public static int dip2px(Context context, float dipValue) {
        float m = context.getResources().getDisplayMetrics().density;
        return (int) (dipValue * m + 0.5f);
    }

    public static byte[] longToByteArray(long integer) {
        int byteNum = (72 - Long.numberOfLeadingZeros(integer < 0 ? ~integer
                : integer)) / 8;
        byte[] byteArray = new byte[8];

        for (int n = 0; n < byteNum; n++)
            byteArray[7 - n] = (byte) (integer >>> (n * 8));

        return (byteArray);
    }

    public static byte[] intToByteArray(final int integer) {
        int byteNum = (40 - Integer.numberOfLeadingZeros(integer < 0 ? ~integer
                : integer)) / 8;
        byte[] byteArray = new byte[4];

        for (int n = 0; n < byteNum; n++)
            byteArray[3 - n] = (byte) (integer >>> (n * 8));

        return (byteArray);
    }
}
