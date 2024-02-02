//
// Created by yuxiao on 24-2-2.
//
extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
}

#include <cstdio>
#include <cstdlib>

static char av_error[10240] = {0};
#define av_err2str(errnum) av_make_error_string(av_error, AV_ERROR_MAX_STRING_SIZE, errnum)

static double _r2d(AVRational r)
{
    return r.den == 0 ? 0 : (double)r.num / (double)r.den;
}

int main()
{
    AVFormatContext *ic = nullptr;
    char path[] = "/root/test.mp4";

    // 1. 打开媒体文件
    int ret = avformat_open_input(&ic, path, 0, 0);
    if (ret != 0) {
        av_log(nullptr, AV_LOG_ERROR, "avformat_open_input() called failed: %s\n", av_err2str(ret));
        return -1;
    }
    av_log(nullptr, AV_LOG_INFO, "avformat_open_input() called success. \n");
    // 获取媒体总时长（单位为毫秒）及流的数量
    av_log(nullptr, AV_LOG_INFO, "duration is: %ld, nb_stream is: %d \n", ic->duration, ic->nb_streams);

    // 2. 探测获取流信息
    if (avformat_find_stream_info(ic, 0) >= 0) {
        av_log(nullptr, AV_LOG_INFO, "duration is: %ld, nb_stream is: %d \n", ic->duration, ic->nb_streams);
    }

    /**帧率*/
    int fps = 0;
    int videoStream = 0;
    int audioStream = 1;

    for (int i = 0; i < ic->nb_streams; i++) {
        AVStream *as = ic->streams[i];
        // 3.1 查找视频流
        if (as->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            // videoStream = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
            av_log(nullptr, AV_LOG_INFO, "video stream.............\n");
            videoStream = i;
            fps = (int)_r2d(as->avg_frame_rate);
            av_log(nullptr, AV_LOG_INFO, "fps = %d, width = %d, height = %d, codecid = %d, format = %d\n",
                   fps,
                   as->codecpar->width,
                   as->codecpar->height,
                   as->codecpar->codec_id,
                   as->codecpar->format);
        }
            // 3.2 查找音频流
        else if (as->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            av_log(nullptr, AV_LOG_INFO, "audio stream.............\n");
            // audioStream = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
            audioStream = i;
            av_log(nullptr, AV_LOG_INFO, "sample_rate = %d, channels = %d, sample_format = %d\n",
                   as->codecpar->sample_rate,
                   as->codecpar->channels,
                   as->codecpar->format);
        }
    }

    // 4. 关闭并释放资源
    avformat_close_input(&ic);

    return 0;
}