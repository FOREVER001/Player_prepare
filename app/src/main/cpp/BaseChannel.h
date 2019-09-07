//
// Created by zxh17 on 2019-09-03.
//

#ifndef XIAOHUIPLAYER_BASECHANNEL_H
#define XIAOHUIPLAYER_BASECHANNEL_H
extern "C" {
    #include <libavcodec/avcodec.h>
};

#include "safe_queue.h"
/**
 * VideoChannel和AudioChannel的父类
 */
class BaseChannel {
public:
    BaseChannel(int id):id(id) {
        packets.setReleaseCallback(releaseAVPacket);
        frames.setReleaseCallback(releaseAVFrame);
    }

    virtual ~BaseChannel() {
        packets.clear();
        frames.clear();
    }


    /**
     * 释放AVPacket
     * @param packet T类型的指针，因为T是指针类型，所以这里的参数是二级指针
     */
    static void releaseAVPacket(AVPacket **packet){
        if(packet){
            av_packet_free(packet);
            *packet=0;
        }
    }


    /**
     * 释放AVFrame
     * @param frame T类型的指针，因为T是指针类型，所以这里的参数是二级指针
     */
    static void releaseAVFrame(AVFrame **frame){
        if(frame){
            av_frame_free(frame);
            *frame=0;
        }
    }

    //纯虚函数（抽象函数）
    virtual void start() = 0;
    virtual void stop() = 0;
    SafeQueue <AVPacket *> packets;
    SafeQueue <AVFrame *> frames;
    int id;
};


#endif //XIAOHUIPLAYER_BASECHANNEL_H
