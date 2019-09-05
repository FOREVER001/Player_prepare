//
// Created by zxh17 on 2019-09-03.
//

#ifndef XIAOHUIPLAYER_NEFFMPEG_H
#define XIAOHUIPLAYER_NEFFMPEG_H


#include "JavaCallHelper.h"
#include "AudioChannel.h"
#include "VideoChannel.h"
#include "../../../../../../android_sdk/ndk-bundle/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/include/string.h"
#include "macro.h"
#include <pthread.h>
extern  "C" {
#include <libavformat/avformat.h>
};
class NEFFmpeg {
public:
    NEFFmpeg(JavaCallHelper *javaCallHelper,char *dataSource);
    ~NEFFmpeg();

    void prepare();
    void _prepare();

private:
    JavaCallHelper *javaCallHelper=0;
    AudioChannel *audioChannel=0;
    VideoChannel *videoChannel=0;
    char *dataSource;
    pthread_t pid_prepare;
};


#endif //XIAOHUIPLAYER_NEFFMPEG_H
