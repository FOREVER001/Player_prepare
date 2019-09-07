package com.tianzhuan.xiaohuiplayer;

public class NEPlayer {
    static {
        System.loadLibrary("xiaoHuiPlayer");
    }
    //直播地址或者媒体文件路径
    private String dataSource;

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


    private native void prepareNative(String dataSource);
    private native void startNative();

    public void setOnPreparedListener(OnPreparedListener onPreparedListener) {
        mOnPreparedListener = onPreparedListener;
    }

    private OnPreparedListener mOnPreparedListener;

    

    interface OnPreparedListener{
        void onPrepared();
    }
}
