package com.monect.gameassistant;

import android.content.Intent;
import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;


public class MainActivity extends ActionBarActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Button button = (Button) findViewById(R.id.launch);
        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                boolean bservicerun = HelperClass.IsServiceRunning(MainActivity.this, GameAssistantService.class.getName());
                if (bservicerun) {
                    Intent it = new Intent(MainActivity.this, GameAssistantService.class);
                    MainActivity.this.stopService(it);
                    ((Button) v).setText("启动");
                } else {
                    HelperClass.runRootCommand("chmod 777 /dev/graphics/fb0");
                    HelperClass.runRootCommand("chmod 777 /dev/uinput");
                    if (NativeLib.OpenFrameBuffer()) {
                        //NativeLib.saveScreenShot();
                        Intent it = new Intent(MainActivity.this, GameAssistantService.class);
                        MainActivity.this.startService(it);
                        ((Button) v).setText("停止");
                    } else {
                        Toast.makeText(MainActivity.this, "无法启动，请确保手机已经root", Toast.LENGTH_LONG).show();
                    }

                }
            }
        });


        boolean bservicerun = HelperClass.IsServiceRunning(MainActivity.this, GameAssistantService.class.getName());
        if (!bservicerun) {
            button.setText("启动");
        } else {
            button.setText("停止");
        }


        DisplayMetrics dm = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(dm);

        HelperClass.screenWidth = dm.heightPixels > dm.widthPixels ? dm.heightPixels : dm.widthPixels;
        HelperClass.screenHeight = dm.heightPixels < dm.widthPixels ? dm.heightPixels : dm.widthPixels;


    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }
}
