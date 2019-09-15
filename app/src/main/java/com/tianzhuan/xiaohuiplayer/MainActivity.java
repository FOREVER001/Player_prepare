package com.tianzhuan.xiaohuiplayer;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceView;
import android.view.View;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;

public class MainActivity extends AppCompatActivity implements SeekBar.OnSeekBarChangeListener {
    public static final String TAG = "MainActivity";


    private SurfaceView mSurfaceView;
    private SeekBar seekBar; //进度条，播放总时长挂钩,如果是直播是没有进度条的。
    private NEPlayer mPlayer;
    private boolean isTouch;
    private boolean isSeek;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
//        checkPermission();
        mSurfaceView = findViewById(R.id.surfaceView);
        seekBar = findViewById(R.id.seekBar);
        mPlayer = new NEPlayer();
        mPlayer.setSurfaceView(mSurfaceView);
        mPlayer.setDataSource(new File(Environment.getExternalStorageDirectory() + File.separator + "demo.mp4").getAbsolutePath());
        mPlayer.setOnPreparedListener(new NEPlayer.OnPreparedListener() {
            @Override
            public void onPrepared() {
                int duration = mPlayer.getDuration();
                //如果是直播，duration 为 0
                //不为0 ，说明是点播或者本地文件，可以显示seekBar
                if (duration != 0) {
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            seekBar.setVisibility(View.VISIBLE);
                        }
                    });
                }

                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        Log.e(TAG, "开始播放");
                        Toast.makeText(MainActivity.this, "开始播放", Toast.LENGTH_SHORT).show();
                    }
                });
                //播放，调用到native去
                mPlayer.start();
            }
        });
        mPlayer.setOnProgressListener(new NEPlayer.OnProgressListener() {
            @Override
            public void onProgress(final int progress) {
                //progress:当前的播放进度
                Log.e(TAG, "progress:" + progress);
                //非人为干预进度条，让进度条自然的正常播放
                if(!isTouch){
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            //duration
                            int duration = mPlayer.getDuration();
                            Log.e(TAG, "duration:" + duration);
                            if (duration != 0) {
                                if(isSeek){
                                    isSeek = false;
                                    return;
                                }
                                seekBar.setProgress(progress * 100 / duration);
                            }
                        }
                    });
                }

            }
        });
        mPlayer.setOnErrorListener(new NEPlayer.OnErrorListener() {
            @Override
            public void onError(final int errorCode) {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        Toast.makeText(MainActivity.this, "出错了，错误码：" + errorCode, Toast.LENGTH_SHORT).show();
                    }
                });
            }
        });
        seekBar.setOnSeekBarChangeListener(this);
    }


    private void checkPermission() {

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {//android 6.0以上
            int writePermission = checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE);
            if (writePermission != PackageManager.PERMISSION_GRANTED) {
                requestPermissions(new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, 100);
            }
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == 100) {
            if (permissions[0].equals(Manifest.permission.WRITE_EXTERNAL_STORAGE) && grantResults[0]
                    == PackageManager.PERMISSION_GRANTED) {//允许

            } else {//拒绝
                Toast.makeText(this, "请赋予读写权限，否则应用将无法使用！", Toast.LENGTH_LONG).show();
                MainActivity.this.finish();
            }
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.e("=MainActivity=", "onResume");
        mPlayer.prepare();
    }

    @Override
    protected void onStop() {
        super.onStop();
        Log.e("=MainActivity=", "onstop");
        mPlayer.stop();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.e("=MainActivity=", "onDestroy");

        mPlayer.release();
    }

    /**
     * 进度条要跟随播放进度自动刷新进度,拿到每个时间点相对总播放时长的百分比进度 progress
     * 1.总时间 getDurationNative
     * 2.当前播放时间 :随播放进度动态变化的，
     * NEFFmpeg->native-lib->NEPlayer->MainActivity
     *
     * @param seekBar
     * @param progress
     * @param fromUser
     */
    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {

    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
        isTouch = true;
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        isSeek = true;
        isTouch=false;
        //获取seekbar的当前进度（百分比）
        int seekProgress = seekBar.getProgress();
        //将seekBar的进度转换成真实的播放进度
        int duration = mPlayer.getDuration();
        int playProgress = seekProgress * duration /100;
        //将播放进度传递个底层ffmpeg
        mPlayer.seekTo(playProgress);
    }
}
