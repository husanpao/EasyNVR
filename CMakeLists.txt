cmake_minimum_required(VERSION 3.21)
set(APPNAME EasyNVR)
project(${APPNAME})

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)

set(OpenCV_DIR D:\\SDK_CPP\\opencv\\newbuild\\install)
find_package(OpenCV REQUIRED)
set(Torch_DIR D:\\SDK_CPP\\libtorch-win-shared-with-deps-debug-1.10.1+cu102\\libtorch\\share\\cmake\\Torch)
find_package(Torch REQUIRED)

set(CMAKE_CXX_STANDARD 17)
#设置FFMPEG头文件目录
set(FFMPEG D:\\SDK_CPP\\ffmpeg-master-latest-win64-gpl-shared\\include)

file(GLOB LIBNAMES ${CMAKE_CURRENT_SOURCE_DIR}/libs/*.lib)

#file(GLOB luascripts "${CMAKE_CURRENT_SOURCE_DIR}/scripts/*.lua")
#file(COPY ${luascripts} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin/scripts)

#引入项目头文件
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
#引入FFMPEG
include_directories(${FFMPEG})
#引入库文件
link_directories(libs)
#定义源文件
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRCS)
#可执行文件
add_executable(${APPNAME} ${SRCS})
#链接第三方库
target_link_libraries(${APPNAME} PUBLIC ${OpenCV_LIBS} ${LIBNAMES} ${TORCH_LIBRARIES})
#target_link_libraries(${APPNAME} PRIVATE co libssl libcrypto avutil avformat avcodec avdevice avfilter swresample swscale)