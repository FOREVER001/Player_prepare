#include <jni.h>
#include <string>
#include "NEFFmpeg.h"

extern "C"{
#include <libavutil/avutil.h>
}


//extern "C" JNIEXPORT jstring JNICALL
//Java_com_tianzhuan_xiaohuiplayer_MainActivity_stringFromJNI(
//        JNIEnv *env,
//        jobject /* this */) {
//    std::string hello = "Hello from C++";
//    return env->NewStringUTF(av_version_info());
//}
JavaVM * _vm = 0;
JavaCallHelper *javaCallHelper=0;
jint JNI_OnLoad(JavaVM* vm, void* reserved){
    _vm = vm;
    return JNI_VERSION_1_4;
}
NEFFmpeg *ffmpeg=0;
extern "C"
JNIEXPORT void JNICALL
Java_com_tianzhuan_xiaohuiplayer_NEPlayer_prepareNative(JNIEnv *env, jobject instance,
                                                        jstring dataSource_) {
    const char *dataSource = env->GetStringUTFChars(dataSource_, 0);
    javaCallHelper=new JavaCallHelper(_vm,env,instance);
    ffmpeg=new NEFFmpeg(javaCallHelper, const_cast<char *>(dataSource));
    ffmpeg ->prepare();
    env->ReleaseStringUTFChars(dataSource_, dataSource);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_tianzhuan_xiaohuiplayer_NEPlayer_startNative(JNIEnv *env, jobject instance) {

    if(ffmpeg){
        ffmpeg ->start();
    }

}