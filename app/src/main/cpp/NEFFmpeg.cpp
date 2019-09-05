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

}

NEFFmpeg::~NEFFmpeg() {
    DELETE(this->dataSource);
    DELETE(this->javaCallHelper);
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
void NEFFmpeg::_prepare() {
    //打开输入
    AVFormatContext *formatContext= avformat_alloc_context();
    AVDictionary *options=0;
    av_dict_set(&options,"timeout","1000000",0);//注意时间单位为：微妙
    int ret = avformat_open_input(&formatContext,dataSource,0,0);
    av_dict_free(&options);//释放options
    if(ret){
        //失败，回调给java层。
        LOGE("打开媒体失败：%s",av_err2str(ret));
//        javaCallHelper jni回调java
    //可能java层需要根据errcode来更新UI
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
