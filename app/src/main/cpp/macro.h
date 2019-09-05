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
#endif //XIAOHUIPLAYER_MACRO_H
