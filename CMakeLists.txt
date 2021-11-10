cmake_minimum_required(VERSION 3.17)
project(yolov5)
SET(CMAKE_CXX_FLAGS "-O3 -mfpu=neon-vfpv4 -mfloat-abi=softfp")
set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_BUILD_TYPE "Release")
#OpenCV
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/opencv/inc)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(OpenCV_Lib opencv_core opencv_imgcodecs opencv_imgproc)

#hisdk
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/hiSDK/inc)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/hiSDK/nnie)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/hiSDK/lib)
set(hisdk_Lib nnie)
#svp
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/svp/inc/common
        ${CMAKE_CURRENT_SOURCE_DIR}/svp/inc/nnie)
file(GLOB svp
        ${CMAKE_CURRENT_SOURCE_DIR}/svp/src/common/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/svp/src/nnie/*.c)
add_library(yolov5 SHARED Yolov5.cpp ImageResize.cpp ${svp})
target_link_libraries(yolov5 ${OpenCV_Lib} ${hisdk_Lib})