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
     *
     */
    public void onProgress(int progress){
        if(onProgressListener!=null){
            onProgressListener.onProgress(progress);
        }
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




    public void setOnPreparedListener(OnPreparedListener onPreparedListener) {
        this.mOnPreparedListener = onPreparedListener;
    }
    public void setOnProgressListener(OnProgressListener onProgressListener) {
        this.onProgressListener = onProgressListener;
    }

    public void setOnErrorListener(OnErrorListener onErrorListener) {
        this.mOnErrorListener = onErrorListener;
    }

    private OnPreparedListener mOnPreparedListener;
    private OnErrorListener mOnErrorListener;
    private OnProgressListener onProgressListener;

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

    /**
     * 资源释放
     */
    public void release() {
        mSurfaceHolder.removeCallback(this);
        releaseNative();
    }

    /**
     * 停止播放
     */
    public void stop() {
        stopNative();
    }

    /**
     * 获取总的播放时长
     * @return
     */
    public int getDuration(){
        return getDurationNative();
    }

    /**
     * 播放进度跳转
     * @param playProgress
     */
    public void seekTo(final int playProgress) {
        new Thread(){
            @Override
            public void run() {
                seekToNative(playProgress);
            }
        }.start();

    }


    interface OnPreparedListener{
        void onPrepared();
    }
    interface OnErrorListener{
        void onError(int errorCode);
    }
    interface OnProgressListener{
        void onProgress(int progress);
    }
    private native void prepareNative(String dataSource);
    private native void startNative();
    private native void setSurfaceNative(Surface surface);
    private native void releaseNative();
    private native void stopNative();
    private native int getDurationNative();
    private native void seekToNative(int playProgress);
}
