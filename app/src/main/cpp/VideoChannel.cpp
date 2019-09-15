//
// Created by zxh17 on 2019-09-03.
//




#include "VideoChannel.h"


/**
 * 丢包（AVPacket）
 */
void dropAVPacket(queue<AVPacket *> &q){
    while(!q.empty()){
        AVPacket *avPacket=q.front();
        //I帧 B帧 P 帧
        //不能丢I帧 AV_PKT_FLAG_KEY:I帧（关键帧）
        if(avPacket->flags!=AV_PKT_FLAG_KEY){
            //丢弃非I帧
            BaseChannel::releaseAVPacket(&avPacket);
            q.pop();
        } else{
            break;
        }
    }
}
/**
 * 丢包（AVFrame）
 */
void dropAVFrame(queue<AVFrame *> &q){
    if(!q.empty()){
        AVFrame *avFrame=q.front();
        BaseChannel::releaseAVFrame(&avFrame);
        q.pop();
    }
}

VideoChannel::VideoChannel(int id, AVCodecContext *codecContext, int fps, AVRational time_base,
                           JavaCallHelper *javaCallHelper) : BaseChannel(id, codecContext, time_base,javaCallHelper) {
    this->fps=fps;
    packets.setSyncHandle(dropAVPacket);
    frames.setSyncHandle(dropAVFrame);
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
    /**
     * 内存泄漏点2
     * 控制frames队列
     */
    while (isPlaying && frames.size()>100){
        av_usleep(10*1000);
        continue;
    }
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
    //根据fps（传入的流的平均帧率来控制每一帧的延时时间）
    //sleep :fps转换成时间 （fps表示每秒多少帧）
    double delay_time_per_frame=1.0/fps;//每一帧多少秒（单位是秒）
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

        //进行休眠
        //每一帧还有自己的额外的延时时间
        //extra_delay = repeat_pict / (2*fps)
        double extra_delay = frame->repeat_pict/(2*fps);
        //每一帧真正的延时时间=平均帧率的延时时间+每一帧自己的额外的延时时间
        double real_delay=delay_time_per_frame+extra_delay;
        //单位是微妙
//        av_usleep(real_delay*1000000);

        //获取视频的播放时间
        double  video_time=frame->best_effort_timestamp *av_q2d(time_base);
        if(!audioChannel){
            //没有音频(类似GIF)
            av_usleep(real_delay*1000000);
            if(javaCallHelper){
                javaCallHelper ->onProgress(THREAD_CHILD,video_time);
            }
        } else{
            double  audio_time =   audioChannel->audio_time;
            //获取音视频播放的时间差
            double  time_diff = video_time-audio_time;
            if (time_diff>0){
                LOGE("视频比音频快：%1f",time_diff);
                //视频比音频快 sleep
                //自然播放状态下，time_diff的值不会很大
                //但是，seek后 time_diff的值可能会很大，导致视频休眠太久。
                //  av_usleep((real_delay+time_diff)*1000000); //TODO seek后测试
                if(time_diff>1){//大于1s,认为有seek动作
                    //等音频慢慢赶上
                    av_usleep(real_delay * 2 *1000000);
                } else{
                    av_usleep((real_delay+time_diff)*1000000);
                }

            } else if(time_diff <0){
                LOGE("音频比视频快：%1f",time_diff);
                //音频比视频快 ：追音频（尝试丢视频包）
                //视频包：packet,frame
                if(fabs(time_diff)>=0.05){
                    //时间差大于0.05，有明显的延迟感觉
//                    packets.sync();
                    frames.sync();
                    //丢帧，就不需要渲染，接着下绘制下一帧
                    continue;
                }
            } else{
                LOGE("音视频完美同步");
            }
        }
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

void VideoChannel::setAudioChannel(AudioChannel *audioChannel) {
    this->audioChannel=audioChannel;
}

void VideoChannel::stop() {
    isPlaying=0;
    javaCallHelper = 0;
    //设置队列状态为非工作状态
    packets.setWork(0);
    frames.setWork(0);

    pthread_join(pid_video_decode,0);
    pthread_join(pid_video_play,0);
}

