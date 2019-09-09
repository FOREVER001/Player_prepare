//
// Created by zxh17 on 2019-09-03.
//

#ifndef XIAOHUIPLAYER_AUDIOCHANNEL_H
#define XIAOHUIPLAYER_AUDIOCHANNEL_H


#include "BaseChannel.h"

class AudioChannel : public BaseChannel{
public:
    AudioChannel(int id,AVCodecContext *codecContext);

     ~AudioChannel();
    void start();
    void stop();
};


#endif //XIAOHUIPLAYER_AUDIOCHANNEL_H
