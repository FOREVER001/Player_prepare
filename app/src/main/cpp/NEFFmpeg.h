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
    friend void *task_stop(void *args);
public:
    NEFFmpeg(JavaCallHelper *javaCallHelper,char *dataSource);
    ~NEFFmpeg();

    void prepare();
    void _prepare();

    void start();

    void _start();
    void setRenderCallback( RenderCallback renderCallback);

    void stop();

private:
    JavaCallHelper *javaCallHelper=0;
    AudioChannel *audioChannel=0;
    VideoChannel *videoChannel=0;
    char *dataSource;
    pthread_t pid_prepare;
    pthread_t pid_start;
    pthread_t pid_stop;
    bool isPlaying;
    AVFormatContext *formatContext=0;
    RenderCallback renderCallback;
    int duration;
    pthread_mutex_t seekMutex;
public:
    int getDuration() const;

public:
    void setDuration(int duration);
//总播放时长

    void seekTo(int i);
};


#endif //XIAOHUIPLAYER_NEFFMPEG_H
