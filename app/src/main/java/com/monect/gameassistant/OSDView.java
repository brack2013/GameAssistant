package com.monect.gameassistant;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PixelFormat;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;
import android.graphics.Rect;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

/**
 * Created by Monect on 7/7/15.
 */
public class OSDView extends SurfaceView implements SurfaceHolder.Callback {

    private SurfaceHolder holder;
    // private MyThread thread;

    public OSDView(Context context) {
        super(context);

        this.holder = this.getHolder();
        this.holder.addCallback(this);
        this.holder.setFormat(PixelFormat.TRANSPARENT);
        //this.thread = new MyThread(holder);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
//        this.thread.isRun = true;
//        this.thread.start();
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        //this.thread.isRun = false;
    }

    public void drawRect(Rect rect) {
        Canvas c = this.holder.lockCanvas();
        if (c != null) {
            c.drawColor(Color.TRANSPARENT);

            Paint p = new Paint();
            p.setColor(Color.CYAN);
            p.setStyle(Paint.Style.STROKE);
            p.setStrokeWidth(3);
            c.drawRect(rect, p);

            this.holder.unlockCanvasAndPost(c);
        }
    }

    public void clear() {
        Canvas c = this.holder.lockCanvas();
        if (c != null) {
            Paint paint = new Paint();
            paint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.CLEAR));
            c.drawPaint(paint);
            this.holder.unlockCanvasAndPost(c);
        }
    }

//    class MyThread extends Thread {
//        private SurfaceHolder holder;
//        public boolean isRun;
//
//        public MyThread(SurfaceHolder holder) {
//            this.holder = holder;
//            isRun = true;
//        }
//
//        @Override
//        public void run() {
//            int count = 0;
//            while (isRun) {
//                Canvas c = null;
//                try {
//                    synchronized (holder) {
//                        c = holder.lockCanvas();
//                        //c.drawColor(Color.TRANSPARENT);
//                        Paint p = new Paint();
//                        p.setColor(Color.WHITE);
//                        Rect r = new Rect(100, 50, 300, 250);
//                        c.drawRect(r, p);
//                        c.drawText("这是第" + (count++) + "秒", 100, 310, p);
//                        Thread.sleep(1000);
//                    }
//                } catch (Exception e) {
//                    e.printStackTrace();
//                } finally {
//                    if (c != null) {
//                        holder.unlockCanvasAndPost(c);
//                    }
//                }
//            }
//        }
//    }
}
