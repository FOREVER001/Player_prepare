//
// Created by zxh17 on 2019-09-03.
//

#include "JavaCallHelper.h"


JavaCallHelper::JavaCallHelper(  JavaVM *javaVM,JNIEnv *env,  jobject instance_){
    this->javaVM=javaVM;
    this->env=env;
    //一旦涉及到jobject跨方法，跨线程，需要创建全局引用。
//    this->instance=instance_;//不能直接赋值
    this->instance=env->NewGlobalRef(instance_);//不能直接赋值
    jclass clazz = env->GetObjectClass(instance);
    this->jmd_prepared=env->GetMethodID(clazz,"onPrepared","()V");

}
JavaCallHelper::~JavaCallHelper(){
    javaVM=0;
    env->DeleteGlobalRef(instance);
    instance=0;
}

void JavaCallHelper::onPrepared(int threadMode) {
        if(threadMode == THREAD_MAIN){
            //主线程
            env->CallVoidMethod(instance,jmd_prepared);
        } else {
            //子线程
            // 不能用之前的env,需要拿到当前子线程的Env
            JNIEnv *env_chid;
            javaVM->AttachCurrentThread(&env_chid,0);
             env_chid->CallVoidMethod(instance,jmd_prepared);
            javaVM->DetachCurrentThread();
        }
}

