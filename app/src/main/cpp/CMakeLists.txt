cmake_minimum_required(VERSION 3.4.1)
include_directories(include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/../../../libs/${ANDROID_ABI}")
file(GLOB src_file *.cpp)
add_library(
             xiaoHuiPlayer
             SHARED
             ${src_file} )
target_link_libraries(
        xiaoHuiPlayer
        -Wl,--start-group
        avfilter avformat avcodec avutil swresample swscale
        -Wl,--end-group
        z
        log
        android)