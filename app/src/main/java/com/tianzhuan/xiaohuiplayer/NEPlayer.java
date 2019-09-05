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

    private native void prepareNative(String dataSource);
}
