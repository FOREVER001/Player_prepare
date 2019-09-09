//
// Created by zxh17 on 2019-09-03.
//



#include "VideoChannel.h"

VideoChannel::VideoChannel(int id,  AVCodecContext * codecContext):BaseChannel(id,codecContext) {

}

VideoChannel::~VideoChannel() {

}
/**
 *准备线程pid_video_decode真正执行的函数
 * @param args
 * @return
 */
void *task_video_decode(void *args){
    VideoChannel *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->video_decode();
    return 0;//一定要返回0！
}
/**
 *准备线程pid_video_play真正执行的函数
 * @param args
 * @return
 */
void *task_video_play(void *args){
    VideoChannel *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->video_play();
    return 0;//一定要返回0！
}
void VideoChannel::start() {
    isPlaying=1;
   //设置队列状态为工作状态
    packets.setWork(1);
    frames.setWork(1);
    //开始进行解码播放
    //解码
    pthread_create(&pid_video_decode,0,task_video_decode,this);
    //播放
    pthread_create(&pid_video_play,0,task_video_play,this);
}

void VideoChannel::stop() {

}
/**
 * 真正的视频解码
 * 从队列中循环的读packet
 */
void VideoChannel::video_decode() {
    AVPacket *packet=0;
    while (isPlaying){
        int ret = packets.pop(packet);
        if(!isPlaying){
            //如果停止播放，跳出循环
            break;
        }
        if(!ret){
            //取数据包失败
            continue;
        }
        //拿到了视频数据包（编码压缩了的），需要把数据包给解码器进行解码
     ret = avcodec_send_packet(codecContext,packet);

        if(ret){
            //往解码器发送数据失败，跳出循环
            break;
        }
        releaseAVPacket(&packet); //释放packet,后面不需要了。
        AVFrame *avFrame=av_frame_alloc();
        ret = avcodec_receive_frame(codecContext,avFrame);
        if(ret == AVERROR(EAGAIN)){
            //重来
            continue;
        } else if(ret !=0){
            break;
        }
//        ret==0 数据收发正常，成功获取到我们解码后的视频原始数据包AVFrame,格式是yuv
    //对frame进行处理（渲染播放）直接写？
    frames.push(avFrame);
    }//end while
    releaseAVPacket(&packet);
}
/**
 * 真正的视频播放
 * 从队列中循环的读frame
 */
void VideoChannel::video_play() {
    AVFrame *frame=0;
    /* create scaling context */
    //yuv 400x800 >rgba:400x8004
    uint8_t *dst_data[4];
    int  dst_linesize[4];
    SwsContext *sws_ctx = sws_getContext(codecContext->width, codecContext->height, codecContext->pix_fmt,
                                         codecContext->width, codecContext->height, AV_PIX_FMT_RGBA,
                             SWS_BILINEAR, NULL, NULL, NULL);

    //给dst_data dst_linesize申请内存空间
    /* allocate source and destination image buffers */
    av_image_alloc(dst_data, dst_linesize,
                   codecContext->width, codecContext->height, AV_PIX_FMT_RGBA, 1);

    //要注意对原始数据进行格式转换，yuv>rgba
    while (isPlaying){
        int ret = frames.pop(frame);
        if(!isPlaying){
            //如果停止播放，跳出循环
            break;
        }
        if(!ret){
            //取数据包失败
            continue;
        }
        //取到了yuv原始数据，下面要进行格式转换

        /* convert to destination format */
        sws_scale(sws_ctx, frame->data,
                  frame->linesize, 0, codecContext->height, dst_data, dst_linesize);

        //dst_data ：AV_PIX_FMT_RGBA格式的数据
        //渲染，数据回调出去>native-lib里面
        //渲染一幅图像，需要什么信息？
        //宽高--》图像尺寸
        //图像的内容!(数据)》图像怎么画
        //需要：1.data 2.linesize 3.with 4.height
            renderCallback(dst_data[0],dst_linesize[0],codecContext->width, codecContext->height);
            releaseAVFrame(&frame);
    }//end while

    releaseAVFrame(&frame);
    isPlaying=0;
    av_freep(&dst_data[0]);
    sws_freeContext(sws_ctx);
}

void VideoChannel::setRenderCallback(RenderCallback callback) {
    this->renderCallback=callback;
}
