//
// Created by zxh17 on 2019-09-03.
//



#include "NEFFmpeg.h"



NEFFmpeg::NEFFmpeg(JavaCallHelper *javaCallHelper,char *dataSource) {
    this->javaCallHelper=javaCallHelper;
//    this->dataSource=dataSource;//悬空指针
    //内存拷贝，自己管理它的内存
    //strlen获取字符串的长度，strcpy:拷贝字符串
    //java :"hello"
    //c字符串以\0结尾 ：“hello\0” 所以字符串长度需要加1
    this->dataSource = new char[strlen(dataSource)+1];
    strcpy(this->dataSource,dataSource);
    pthread_mutex_init(&seekMutex,0);

}

NEFFmpeg::~NEFFmpeg() {
    DELETE(this->dataSource);
    DELETE(this->javaCallHelper);
    pthread_mutex_destroy(&seekMutex);
}
/**
 *准备线程pid_prepare真正执行的函数
 * @param args
 * @return
 */
void *task_prepare(void *args){
    NEFFmpeg *ffmpeg = static_cast<NEFFmpeg *>(args);
    ffmpeg->_prepare();
    return 0;//一定要返回0！
}
void *task_start(void *args){
    NEFFmpeg *ffmpeg = static_cast<NEFFmpeg *>(args);
    ffmpeg->_start();
    return 0;//一定要返回0！
}
/**
 * friend 利用友元函数访问其他类的私有成员
 * @param args
 * @return
 */
void *task_stop(void *args){
    NEFFmpeg *ffmpeg = static_cast<NEFFmpeg *>(args);
    ffmpeg->isPlaying=0;
    //在主线程，要保证prepare方法执行完在释放。
    //pthread_join ：这里调用了后会阻塞主线程，可能遇到ANR
    pthread_join(ffmpeg->pid_prepare,0);//等pid_prepare这个线程执行完，下面的代码才会执行
//    ffmpeg->_stop();

    if(ffmpeg->formatContext){
        avformat_close_input(&ffmpeg->formatContext);
        avformat_free_context(ffmpeg->formatContext);
        ffmpeg->formatContext = 0;
    }

    DELETE(ffmpeg->videoChannel);
    DELETE(ffmpeg->audioChannel);
    DELETE(ffmpeg);

    return 0;//一定要返回0！
}
void NEFFmpeg::_prepare() {
    //打开输入
    formatContext= avformat_alloc_context();
    AVDictionary *options=0;
    av_dict_set(&options,"timeout","1000000",0);//注意时间单位为：微妙
    //1.打开媒体
    int ret = avformat_open_input(&formatContext,dataSource,0,0);
    av_dict_free(&options);//释放options
    if(ret){
        //失败，回调给java层。
        LOGE("打开媒体失败：%s",av_err2str(ret));
//        javaCallHelper jni回调java
    //可能java层需要根据errcode来更新UI
        if(javaCallHelper){
            javaCallHelper->onError(THREAD_CHILD,FFMPEG_CAN_NOT_OPEN_URL);
        }
        return;
    }
    //2.查找媒体中音/视频流信息
   ret = avformat_find_stream_info(formatContext,0);
    if(ret<0){

        if(javaCallHelper){
            javaCallHelper->onError(THREAD_CHILD,FFMPEG_CAN_NOT_FIND_STREAMS);
        }
        return;
    }
   duration= formatContext->duration/AV_TIME_BASE;
    //遍历流
    //这里的i就是 packet->stream_index
    for (int i = 0; i < formatContext->nb_streams; ++i) {
        //获取媒体流（音频或者视频）
        AVStream *avStream= formatContext->streams[i];
        //获取编解码这段流的参数
        AVCodecParameters *codecparParamters= avStream->codecpar;
        //3.通过解码方式查找相应解码器,也就是通过参数中的id(编解码的方式)，来查找当前流的解码器
        AVCodec * codec = avcodec_find_decoder(codecparParamters->codec_id);
        // 如果ffmpeg不支持某种编解码方式，所以需要判断
            if(!codec){

                if(javaCallHelper){
                    javaCallHelper->onError(THREAD_CHILD,FFMPEG_FIND_DECODER_FAIL);
                }
                return;
            }
            //6.创建解码器上下文
         AVCodecContext * codecContext = avcodec_alloc_context3(codec);
            //7.设置解码器上下文的参数
           ret = avcodec_parameters_to_context(codecContext,codecparParamters);
           if(ret<0){

               if(javaCallHelper){
                   javaCallHelper->onError(THREAD_CHILD,FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL);
               }
               return;
           }
           //8.打开解码器
          ret =avcodec_open2(codecContext,codec,0);
        if(ret<0){

            if(javaCallHelper){
                javaCallHelper->onError(THREAD_CHILD,FFMPEG_OPEN_DECODER_FAIL);
            }
            return;
        }

        AVRational time_base = avStream->time_base;
        //判断流类型（音频还是视频？）
       if( codecparParamters->codec_type == AVMEDIA_TYPE_AUDIO){
            //AudioChannel
            audioChannel= new AudioChannel(i, codecContext, time_base, javaCallHelper);
       } else if( codecparParamters->codec_type == AVMEDIA_TYPE_VIDEO){
            //VideoChannel
            //帧率
           AVRational  frame_rate= avStream->avg_frame_rate;
//            int fps = frame_rate.num/frame_rate.den;
            int fps=av_q2d(frame_rate);
            videoChannel= new VideoChannel(i, codecContext, fps, time_base, javaCallHelper);
            videoChannel->setRenderCallback(renderCallback);
       }

    }//end for
    if(!audioChannel && !videoChannel ){
        //既没有音频也没有视频

        if(javaCallHelper){
            javaCallHelper->onError(THREAD_CHILD,FFMPEG_NOMEDIA);
        }
        return;
    }
    //准备好了，通知java层。
    if(javaCallHelper){
        LOGE("播放准备好了");
        javaCallHelper->onPrepared(THREAD_CHILD);
    }

}
/**
 * 播放准备
 */
void NEFFmpeg::prepare() {
    //可以直接来进行解码api调用吗？
    //pthread_create ：创建子线程
    //int pthread_create(pthread_t* __pthread_ptr, pthread_attr_t const* __attr, void* (*__start_routine)(void*), void*);
    //void* (*__start_routine)(void*)函数指针，就相当于java中线程中的run方法
    pthread_create(&pid_prepare,0,task_prepare,this);
}

/**
 * 开始播放
 */
void NEFFmpeg::start() {
    isPlaying=1;
    if(videoChannel){
        videoChannel->start();
        videoChannel->setAudioChannel(audioChannel);
    }
    if(audioChannel){
        audioChannel->start();
    }
    pthread_create(&pid_start,0,task_start,this);
}
/**
 *真正执行解码播放(循环解码)
 */
void NEFFmpeg::_start() {

    while (isPlaying){
        /**
         * 内存泄漏点1
         * 控制视频packets队列
       */
        if(videoChannel && videoChannel->packets.size() >100){
            av_usleep(10*1000);
            continue;
        }
        /**
       * 内存泄漏点2
       * 控制音频packets队列
     */
        if(audioChannel && audioChannel->packets.size() >100){
            av_usleep(10*1000);
            continue;
        }
        AVPacket *packet=av_packet_alloc();
        pthread_mutex_lock(&seekMutex);
       int ret = av_read_frame(formatContext,packet);
       pthread_mutex_unlock(&seekMutex);
       if(!ret){
           //要判断流类型，是视频还是音频
         if(videoChannel && packet->stream_index == videoChannel->id){
             //往视频编码数据包队列中添加数据
             videoChannel->packets.push(packet);
         } else  if(audioChannel && packet->stream_index == audioChannel->id){
             //往音频编码数据包队列中添加数据
             audioChannel->packets.push(packet);
         }
       } else if(ret == AVERROR_EOF){
           //表示读完了
           //要考虑读完了，是否播完的情况
           //如何判断有没有播放完？判断队列是否为空
           if(videoChannel->packets.empty() && videoChannel->frames.empty()
            && audioChannel->packets.empty() && audioChannel->frames.empty()){
               //播放完
               av_packet_free(&packet);
               break;
           }

       } else{
           LOGE("没有音视频数据包失败");
           if (javaCallHelper) {
               javaCallHelper->onError(THREAD_CHILD, FFMPEG_READ_PACKETS_FAIL);
           }
           av_packet_free(&packet);
           break;
       }

    }//end while
    isPlaying=0;
    //停止解码播放（音频和视频）
    videoChannel->stop();
    audioChannel->stop();
}

void NEFFmpeg::setRenderCallback( RenderCallback renderCallback){
    this->renderCallback=renderCallback;
}
/**
 * 停止播放
 */
void NEFFmpeg::stop() {

//    isPlaying=0;
    javaCallHelper = 0;//prepare阻塞中停止了，还是会回调java准备好了


    //既然在主线程会引发ANR,那么在我们的子线程中去释放
    pthread_create(&pid_stop,0,task_stop,this);//创建stop子线程

//    if(formatContext){
//        avformat_close_input(&formatContext);
//        formatContext = 0;
//    }
//
//    if(videoChannel){
//        videoChannel->stop();
//    }
//    if(audioChannel){
//        audioChannel->stop();
//    }
}

int NEFFmpeg::getDuration() const {
    return duration;
}

void NEFFmpeg::seekTo(int playProgress) {
    //必须要在可用范围
    if(playProgress <0 || playProgress>duration){
        return;
    }
    //防止audioChannel,videoChannel被释放掉
    if(!audioChannel && !videoChannel){
        return;
    }
    //防止formatContext被释放掉
    if(!formatContext){
        return;
    }
    //1.上下文
    //2.流索引，-1 表示选择的是默认值
    //3.要seek到的时间戳
    //AVSEEK_FLAG_BACKWARD表示seek到请求时间戳之前的最靠近的一个关键帧。
    //AVSEEK_FLAG_BYTE 基于字节位置seek
    //AVSEEK_FLAG_ANY 任意帧 (可能不是关键帧，会花屏)
    //AVSEEK_FLAG_FRAME 基于帧数seek
    pthread_mutex_lock(&seekMutex);
    int ret =  av_seek_frame(formatContext,-1,playProgress * AV_TIME_BASE,AVSEEK_FLAG_BACKWARD);
    if (ret <0){
        if (javaCallHelper) {
            javaCallHelper->onError(THREAD_CHILD, ret);
        }
        return;
    }
    if(audioChannel){
        //先让队列不工作
        audioChannel->packets.setWork(0);
        audioChannel->frames.setWork(0);
        //清空队列
        audioChannel->packets.clear();
        audioChannel->frames.clear();
        //清空数据之后，让队列恢复工作
        audioChannel->packets.setWork(1);
        audioChannel->frames.setWork(1);
    }

    if(videoChannel){
        //先让队列不工作
        videoChannel->packets.setWork(0);
        videoChannel->frames.setWork(0);
        //清空队列
        videoChannel->packets.clear();
        videoChannel->frames.clear();
        //清空数据之后，让队列恢复工作
        videoChannel->packets.setWork(1);
        videoChannel->frames.setWork(1);
    }

    pthread_mutex_unlock(&seekMutex);

}




