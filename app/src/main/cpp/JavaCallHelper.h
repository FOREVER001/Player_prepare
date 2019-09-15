//
// Created by zxh17 on 2019-09-03.
//

#ifndef XIAOHUIPLAYER_JAVACALLHELPER_H
#define XIAOHUIPLAYER_JAVACALLHELPER_H


#include <jni.h>
#include "macro.h"
class JavaCallHelper {
public:
    JavaCallHelper(JavaVM *javaVM,JNIEnv *env,  jobject instance_);
    ~JavaCallHelper();

    void onPrepared(int threadMode);

    void onError(int i, int i1);
    void onProgress(int threadMode, int progress);
private:
        JavaVM *javaVM;
        JNIEnv *env;
       jobject instance;
       jmethodID jmd_prepared;
       jmethodID jmd_onError;
       jmethodID jmd_onProgress;

};


#endif //XIAOHUIPLAYER_JAVACALLHELPER_H
