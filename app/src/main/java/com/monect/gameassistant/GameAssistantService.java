package com.monect.gameassistant;

import android.app.Activity;
import android.app.Notification;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Intent;
import android.graphics.PixelFormat;
import android.graphics.Rect;
import android.os.IBinder;
import android.util.Log;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageView;

public class GameAssistantService extends Service {

    WindowManager windowManager;
    View floatingView;
    OSDView osdView;
    boolean isStop = true;
    Button dot;
    View setup;

    int latestX;
    int latestY;

    public GameAssistantService() {
    }

    @Override
    public IBinder onBind(Intent intent) {
        // TODO: Return the communication channel to the service.
        throw new UnsupportedOperationException("Not yet implemented");
    }

    @Override
    public void onCreate() {
        super.onCreate();

        PendingIntent p_intent = PendingIntent.getActivity(this, 0, new Intent(
                this, MainActivity.class), 0);
        Notification noti = new Notification.Builder(this)
                .setContentIntent(p_intent)
                .setContentTitle("游戏助手正在运行")
                .setContentText("dfdfdsa")
                .setSmallIcon(R.drawable.notification_template_icon_bg)
                .getNotification();

        startForeground(1986, noti);
        this.floatingView = View.inflate(this, R.layout.view_floating, null);
        this.dot = (Button) this.floatingView.findViewById(R.id.dot);
        this.setup = this.floatingView.findViewById(R.id.setup);
        this.dot.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {

                switch (event.getActionMasked()) {
                    case MotionEvent.ACTION_MOVE:
                        WindowManager.LayoutParams lop = (WindowManager.LayoutParams) GameAssistantService.this.floatingView.getLayoutParams();
                        lop.x = GameAssistantService.this.latestX = (int) event.getRawX() - (v.getWidth() / 2);
                        lop.y = GameAssistantService.this.latestY = (int) event.getRawY() - (v.getHeight() / 2);
                        GameAssistantService.this.windowManager.updateViewLayout(GameAssistantService.this.floatingView, lop);

                        break;
                }

                return false;
            }
        });
        this.dot.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                showSetup(true);
            }
        });

        this.floatingView.findViewById(R.id.close).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                showSetup(false);
            }
        });

        this.floatingView.findViewById(R.id.run).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        findAndClick(new FindTarget("/sdcard/mrhd.jpg", 0.7f, 0.7f, 0.3f, 0.3f));
                        try {
                            Thread.sleep(1000);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                        Rect rc = scrollAndFind(new FindTarget("/sdcard/Download/dscj(2).jpg", 0.15f, 0.15f, 0.15f, 0.75f));
                        findAndClick(new FindTarget("/sdcard/Download/qianwang.jpg", (float) rc.right / HelperClass.screenWidth, (float) rc.top / HelperClass.screenHeight,
                                1 - ((float) rc.right / HelperClass.screenWidth), (float) rc.height() / HelperClass.screenHeight));

                        try {
                            Thread.sleep(1000);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                        findAndClick(new FindTarget("/sdcard/shiyong.jpg", 0.0f, 0f, 1f, 1f));
                        findAndClick(new FindTarget("/sdcard/guani.jpg", 0.0f, 0f, 1f, 1f));
                        findAndClick(new FindTarget("/sdcard/ljl.jpg", 0.5f, 0f, 0.5f, 0.5f));

                        //findAndClick(new findtarget("/sdcard/plus.jpg", 0.1f, 0f, 0.333f, 0.2f));
                        //findAndClick(new findtarget("/sdcard/gold.jpg", 0.1f, 0f, 0.333f, 0.2f));
                        //findAndClick(new findtarget("/sdcard/ljl.jpg", 0.5f, 0f, 0.5f, 0.5f));
                        //findAndClick(new findtarget("/sdcard/queding.jpg", 0.2f, 0.5f, 0.6f, 0.5f));
                    }
                }).start();
                showSetup(false);
            }
        });

        this.dot.setVisibility(View.VISIBLE);
        this.setup.setVisibility(View.GONE);

        this.osdView = new OSDView(this);

        this.windowManager = (WindowManager) this.getSystemService(Activity.WINDOW_SERVICE);

    }

    void showSetup(boolean isShow) {
        if (isShow) {
            GameAssistantService.this.dot.setVisibility(View.GONE);
            GameAssistantService.this.setup.setVisibility(View.VISIBLE);

            WindowManager.LayoutParams lop = (WindowManager.LayoutParams) GameAssistantService.this.floatingView.getLayoutParams();
            lop.width = 400;
            lop.height = 300;
            lop.x = (HelperClass.screenWidth - lop.width) / 2;
            lop.y = (HelperClass.screenHeight - lop.height) / 2;
            GameAssistantService.this.windowManager.updateViewLayout(GameAssistantService.this.floatingView, lop);
        } else {
            GameAssistantService.this.dot.setVisibility(View.VISIBLE);
            GameAssistantService.this.setup.setVisibility(View.GONE);

            WindowManager.LayoutParams lop = (WindowManager.LayoutParams) GameAssistantService.this.floatingView.getLayoutParams();
            lop.width = 100;
            lop.height = 100;
            lop.x = GameAssistantService.this.latestX;
            lop.y = GameAssistantService.this.latestY;
            GameAssistantService.this.windowManager.updateViewLayout(GameAssistantService.this.floatingView, lop);
        }
    }

    class FindTarget {
        String name;
        float roiX;
        float roiY;
        float roiWidth;
        float roiHeight;

        public FindTarget(String targetPath, float roiX, float roiY, float roiWidth, float roiHeight) {
            this.name = targetPath;
            this.roiX = roiX;
            this.roiY = roiY;
            this.roiWidth = roiWidth;
            this.roiHeight = roiHeight;
        }
    }

    public void findAndClick(FindTarget target) {
        NativeLib.setTarget(target.name, target.roiX, target.roiY, target.roiWidth, target.roiHeight);

        float[] xs = new float[4];
        float[] ys = new float[4];

        while (!GameAssistantService.this.isStop) {

            int ret = NativeLib.findTargetInScreen(xs, ys);
            if (ret < 10) {
                int left = (int) ys[0];
                int top = (int) (HelperClass.screenHeight - xs[1]);
                int right = (int) ys[2];
                int bottom = (int) (HelperClass.screenHeight - xs[0]);
                GameAssistantService.this.osdView.clear();
                GameAssistantService.this.osdView.drawRect(new Rect(left, top, right, bottom));

                click((int) ((xs[0] + xs[2]) / 2), (int) ((ys[0] + ys[2]) / 2));
                break;
            }
        }
    }

    Rect scrollAndFind(FindTarget target) {
        NativeLib.setTarget(target.name, target.roiX, target.roiY, target.roiWidth, target.roiHeight);

        float[] xs = new float[4];
        float[] ys = new float[4];

        Rect rc = new Rect();

        int findtime = 0;
        while (!GameAssistantService.this.isStop) {

            int ret = NativeLib.findTargetInScreen(xs, ys);
            if (ret < 3) {
                int left = (int) ys[0];
                int top = (int) (HelperClass.screenHeight - xs[1]);
                int right = (int) ys[2];
                int bottom = (int) (HelperClass.screenHeight - xs[0]);
                rc = new Rect(left, top, right, bottom);

                GameAssistantService.this.osdView.clear();
                GameAssistantService.this.osdView.drawRect(new Rect(left, top, right, bottom));
                break;
            } else if (findtime < 2) {
                findtime++;
            } else {
                touchMove((float) HelperClass.screenHeight / 3, (float) HelperClass.screenWidth / 2, (float) HelperClass.screenHeight * 3 / 5, (float) HelperClass.screenWidth / 2);
                findtime = 0;
            }
        }

        return rc;
    }

    void click(int x, int y) {
        NativeLib.touchevent(1, x, y);
        NativeLib.touchend();

        NativeLib.touchevent(-1, -1, -1);
        NativeLib.touchend();
    }

    void touchMove(float startX, float startY, float endX, float endY) {

        float a = (startY - endY) / (startX - endX);
        float b = endY - (startX * (startY - endY) / (startX - endX));
        for (float x = startX; x <= endX; x += 2) {
            float y = a * x + b;
            NativeLib.touchevent(1, (int) x, (int) y);
            NativeLib.touchend();
            try {
                Thread.sleep(3);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        NativeLib.touchevent(-1, -1, -1);
        NativeLib.touchend();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {

        this.isStop = false;
        NativeLib.openInputs(NativeLib.XINPUTS_TOUCHSCREEN, HelperClass.screenWidth, HelperClass.screenHeight);
        showSystemDialog();

        return START_STICKY;
    }

    @Override
    public void onDestroy() {
        this.isStop = true;
        this.windowManager.removeView(this.floatingView);
        this.windowManager.removeView(this.osdView);
        NativeLib.closeInputs();
        super.onDestroy();
    }


    private void showSystemDialog() {

        WindowManager.LayoutParams loparams = new WindowManager.LayoutParams();
        loparams.type = WindowManager.LayoutParams.TYPE_SYSTEM_ALERT | WindowManager.LayoutParams.TYPE_SYSTEM_OVERLAY;
        loparams.format = PixelFormat.RGBA_8888;
        loparams.flags = WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE;
        loparams.width = 100;
        loparams.height = 100;
        loparams.alpha = 0.8f;
        loparams.gravity = Gravity.LEFT | Gravity.TOP;
        loparams.x = 0;
        loparams.y = 10;
        this.floatingView.setVisibility(View.VISIBLE);
        this.windowManager.addView(this.floatingView, loparams);


        WindowManager.LayoutParams loparams_osd = new WindowManager.LayoutParams();
        loparams_osd.type = WindowManager.LayoutParams.TYPE_SYSTEM_ALERT | WindowManager.LayoutParams.TYPE_SYSTEM_OVERLAY;
        loparams_osd.format = PixelFormat.TRANSLUCENT;
        loparams_osd.format = PixelFormat.RGBA_8888;
        loparams_osd.flags = WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE | WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE;
        loparams_osd.width = HelperClass.screenWidth;
        loparams_osd.height = HelperClass.screenHeight;
        loparams_osd.alpha = 0.5f;
        loparams_osd.gravity = Gravity.LEFT | Gravity.TOP;
        loparams_osd.x = 0;
        loparams_osd.y = 0;
        this.windowManager.addView(this.osdView, loparams_osd);
    }
}
