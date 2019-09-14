//
// Created by zxh17 on 2019-09-03.
//

#ifndef XIAOHUIPLAYER_AUDIOCHANNEL_H
#define XIAOHUIPLAYER_AUDIOCHANNEL_H


#include "BaseChannel.h"
// for native audio
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "macro.h"
extern "C"{
    #include <libswresample/swresample.h>
};

class AudioChannel : public BaseChannel{
public:
    AudioChannel(int id,AVCodecContext *codecContext,AVRational time_base);

     ~AudioChannel();
    void start();
    void stop();

    void audio_decode();

    void video_play();

    int getPCM();
    uint8_t *out_buffers=0;
    int out_channels;
    int out_sampleSize;
    int out_sampleRate;
    int out_buffers_size;

private:
    SwrContext *swrContext;
    pthread_t pid_audio_decode;
    pthread_t pid_audio_play;
    //引擎
    SLObjectItf engineObject=0;
    //引擎接口
    SLEngineItf engineInterface=0;
    //混音器
    SLObjectItf outputMixObject=0;
    //播放器
    SLObjectItf bqPlayerObject=0;
    //播放器接口
    SLPlayItf bqPlayerPlay=0;
    //播放器队列接口
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue=0;
};


#endif //XIAOHUIPLAYER_AUDIOCHANNEL_H
