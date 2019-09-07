//
// Created by zxh17 on 2019-09-03.
//

#ifndef XIAOHUIPLAYER_VIDEOCHANNEL_H
#define XIAOHUIPLAYER_VIDEOCHANNEL_H


#include "BaseChannel.h"

class VideoChannel : public BaseChannel{
public:
    VideoChannel(int id);

     ~VideoChannel();
     void start();
     void stop();
};


#endif //XIAOHUIPLAYER_VIDEOCHANNEL_H
