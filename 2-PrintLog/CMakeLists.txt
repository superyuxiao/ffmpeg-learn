# 设置最低的CMake版本要求
cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(ffmpeg-PrintLog)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加可执行文件
add_executable(ffmpeg-PrintLog)

# 递归将指定目录下的所有源文件添加到可执行文件
aux_source_directory(src SOURCES)

# 将源文件添加到可执行文件
target_sources(ffmpeg-PrintLog PRIVATE ${SOURCES})

# 指定FFmpeg库的路径
set(FFMPEG_LIBRARY_PATH "/usr/local/lib")

# 指定链接库的路径
link_directories(ffmpeg-PrintLog PRIVATE ${FFMPEG_LIBRARY_PATH})

# 添加链接的库，如果有的话
target_link_libraries(ffmpeg-PrintLog PUBLIC pthread)
target_link_libraries(ffmpeg-PrintLog PUBLIC z)
target_link_libraries(ffmpeg-PrintLog PUBLIC bz2)

target_link_libraries(ffmpeg-PrintLog PUBLIC avfilter)
target_link_libraries(ffmpeg-PrintLog PUBLIC avformat)
target_link_libraries(ffmpeg-PrintLog PUBLIC avcodec)
target_link_libraries(ffmpeg-PrintLog PUBLIC avutil)
target_link_libraries(ffmpeg-PrintLog PUBLIC swresample)
target_link_libraries(ffmpeg-PrintLog PUBLIC swscale)
target_link_libraries(ffmpeg-PrintLog PUBLIC postproc)
target_link_libraries(ffmpeg-PrintLog PUBLIC libx264.so)
target_link_libraries(ffmpeg-PrintLog PUBLIC libx265.so)
target_link_libraries(ffmpeg-PrintLog PUBLIC fdk-aac)

# 如果有其他头文件路径，也可以添加
# target_include_directories(ffmpeg-PrintLog PRIVATE include)
