//
// Created by zxh17 on 2019-09-04.
//

#ifndef XIAOHUIPLAYER_MACRO_H
#define XIAOHUIPLAYER_MACRO_H

#include <android/log.h>
//if(this->dataSource){
//delete  this->dataSource;
//this->dataSource=0;
//}
//定义释放宏函数
#define DELETE(object) if(object){delete object;object=0;}
//定义日志打印宏函数
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "NEFFMPEG",__VA_ARGS__)
//1.主线程 2.子线程
#define THREAD_MAIN 1
#define THREAD_CHILD 2
#endif //XIAOHUIPLAYER_MACRO_H
