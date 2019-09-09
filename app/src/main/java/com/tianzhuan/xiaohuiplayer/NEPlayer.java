package com.tianzhuan.xiaohuiplayer;

import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class NEPlayer implements SurfaceHolder.Callback {
    static {
        System.loadLibrary("xiaoHuiPlayer");
    }
    //直播地址或者媒体文件路径
    private String dataSource;
    private SurfaceHolder mSurfaceHolder;

    public void setDataSource(String dataSource) {
        this.dataSource = dataSource;
    }

    /**
     * 播放准备工作
     * */
    public void prepare() {
        prepareNative(dataSource);
    }

    /**
     * 开始播放
     */
    public void start() {
        startNative();
    }



    /**
     *供native反射调用
     * 表示播放器准备好了可以开始播放了
     */
    public void onPrepared(){
        if(mOnPreparedListener!=null){
            mOnPreparedListener.onPrepared();
        }
    }

    /**
     * 供native反射调用
     * 表示出错了
     * @param errorCode
     */
    public void onError(int errorCode){
        if(mOnErrorListener!=null){
            mOnErrorListener.onError(errorCode);
        }
    }


    private native void prepareNative(String dataSource);
    private native void startNative();
    private native void setSurfaceNative(Surface surface);

    public void setOnPreparedListener(OnPreparedListener onPreparedListener) {
        mOnPreparedListener = onPreparedListener;
    }

    public void setOnErrorListener(OnErrorListener onErrorListener) {
        mOnErrorListener = onErrorListener;
    }

    private OnPreparedListener mOnPreparedListener;
    private OnErrorListener mOnErrorListener;

    public void setSurfaceView(SurfaceView surfaceView) {
        if(mSurfaceHolder!=null){
            mSurfaceHolder.removeCallback(this);
        }
        mSurfaceHolder=surfaceView.getHolder();
        mSurfaceHolder.addCallback(this);
    }

    /**
     * 画布创建回调
     * @param holder
     */
    @Override
    public void surfaceCreated(SurfaceHolder holder) {

    }

    /**
     * 画布刷新
     * @param holder
     * @param format
     * @param width
     * @param height
     */
    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        setSurfaceNative(holder.getSurface());
    }


    /**
     * 画布销毁回调
     * @param holder
     */
    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }


    interface OnPreparedListener{
        void onPrepared();
    }
    interface OnErrorListener{
        void onError(int errorCode);
    }
}
