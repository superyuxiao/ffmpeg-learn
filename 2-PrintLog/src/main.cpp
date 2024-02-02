//
// Created by yuxiao on 24-2-2.
//
extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/log.h>
}

#include <cstdio>
#include <cstdlib>

int main()
{
    auto aversion = av_version_info();

    // 常用log日志级别
    // AV_LOG_ERROR, AV_LOG_WARNING, AV_LOG_INFO, AV_LOG_DEBUG
    av_log_set_level(AV_LOG_DEBUG);
    av_log(nullptr, AV_LOG_INFO, "version: %s\n", aversion);

    av_log(nullptr, AV_LOG_ERROR, "This is a ERROR log\n");
    av_log(nullptr, AV_LOG_WARNING, "This is a WARNING log\n");
    av_log(nullptr, AV_LOG_INFO, "This is a INFO log\n");
    av_log(nullptr, AV_LOG_DEBUG, "This is a DEBUG log\n");

    return 0;
}