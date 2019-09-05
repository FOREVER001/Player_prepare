package com.tianzhuan.xiaohuiplayer;

import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.SurfaceView;
import android.widget.TextView;

import java.io.File;

public class MainActivity extends AppCompatActivity {



    private SurfaceView mSurfaceView;
    private NEPlayer mPlayer;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mSurfaceView=findViewById(R.id.surfaceView);
        mPlayer=new NEPlayer();
        mPlayer.setDataSource(new File(Environment.getExternalStorageDirectory()+File.separator+"demo.mp4").getAbsolutePath());
    }



    @Override
    protected void onResume() {
        super.onResume();
        mPlayer.prepare();
    }

//    public native String stringFromJNI();
}
