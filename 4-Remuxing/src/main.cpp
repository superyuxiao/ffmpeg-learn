//
// Created by yuxiao on 24-2-5.
// 重新封装是将文件从一种格式转换为另一种格式。例如：我们可以非常容易地利用 FFmpeg 将 MPEG-4 格式的视频 转换成 MPEG-TS 格式。
//
extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
#include <libavutil/timestamp.h>
}

#include <cstdio>
#include <cstdlib>


// based on https://ffmpeg.org/doxygen/trunk/remuxing_8c-example.html
static char av_error[10240] = {0};
#define av_err2str(errnum) av_make_error_string(av_error, AV_ERROR_MAX_STRING_SIZE, errnum)

int main()
{
    // 相当于命令行 ffmpeg test.mp4 -c copy test.ts
    // mp4与ts格式都支持h264和acc编码，所以可以直接转换封装格式
    // 若要转换为其他编码的封装格式，就需要重新编码
    AVFormatContext *input_format_context = nullptr, *output_format_context = nullptr;
    AVPacket packet;
    const char in_filename[] = "/root/test.mp4";
    const char out_filename[] = "/root/test.ts";
    int ret, i;
    int stream_index = 0;
    int *streams_list = nullptr;
    int number_of_streams = 0;

    // 打开视频文件，输入
    if ((ret = avformat_open_input(&input_format_context, in_filename, NULL, NULL)) < 0) {
        fprintf(stderr, "Could not open input file '%s'", in_filename);
        return -1;
    }

    // 获取媒体信息
    if ((ret = avformat_find_stream_info(input_format_context, NULL)) < 0) {
        fprintf(stderr, "Failed to retrieve input stream information");
        return -1;
    }

    // 创建输出流
    avformat_alloc_output_context2(&output_format_context, nullptr, nullptr, out_filename);
    if (!output_format_context) {
        fprintf(stderr, "Could not create output context\n");
        ret = AVERROR_UNKNOWN;
        return -1;
    }

    // 流
    number_of_streams = input_format_context->nb_streams;
    streams_list = static_cast<int *>(av_mallocz_array(number_of_streams, sizeof(*streams_list)));

    if (!streams_list) {
        ret = AVERROR(ENOMEM);
        return -1;
    }

    for (i = 0; i < input_format_context->nb_streams; i++) {
        AVStream *out_stream;
        AVStream *in_stream = input_format_context->streams[i];
        AVCodecParameters *in_codecpar = in_stream->codecpar;
        // 只处理视频流、音频流和字幕流
        if (in_codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE) {
            streams_list[i] = -1;
            continue;
        }
        streams_list[i] = stream_index++;
        // 创建新的流
        out_stream = avformat_new_stream(output_format_context, nullptr);
        if (!out_stream) {
            fprintf(stderr, "Failed allocating output stream\n");
            ret = AVERROR_UNKNOWN;
            return -1;
        }
        // 在不同的上下文中复制编解码器参数
        ret = avcodec_parameters_copy(out_stream->codecpar, in_codecpar);
        if (ret < 0) {
            fprintf(stderr, "Failed to copy codec parameters\n");
            return -1;
        }
    }

    // 打印输入媒体文件的格式信息
    av_dump_format(input_format_context, 0, in_filename, 0);
    // 打印输出媒体文件的格式信息
    av_dump_format(output_format_context, 0, out_filename, 1);

    if (!(output_format_context->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&output_format_context->pb, out_filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            fprintf(stderr, "Could not open output file '%s'", out_filename);
            return -1;
        }
    }
    AVDictionary* opts = nullptr;

    // 写入输出文件的文件头信息
    ret = avformat_write_header(output_format_context, &opts);
    if (ret < 0) {
        fprintf(stderr, "Error occurred when opening output file\n");
        return -1;
    }
    while ((ret = av_read_frame(input_format_context, &packet)) >= 0) {
        AVStream *in_stream, *out_stream;
//        ret = av_read_frame(input_format_context, &packet);
//        if (ret < 0)
//            break;
        in_stream  = input_format_context->streams[packet.stream_index];
        if (packet.stream_index >= number_of_streams || streams_list[packet.stream_index] < 0) {
            av_packet_unref(&packet);
            continue;
        }
        packet.stream_index = streams_list[packet.stream_index];
        out_stream = output_format_context->streams[packet.stream_index];
        /* copy packet */
        // 将一个有理数（Rational）按比例缩放到另一个有理数的范围内。用于处理时间戳、帧率等情况，将一个有理数按照指定的分辨率进行缩放
        packet.pts = av_rescale_q_rnd(packet.pts, in_stream->time_base, out_stream->time_base,
                                      static_cast<AVRounding>(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        packet.dts = av_rescale_q_rnd(packet.dts, in_stream->time_base, out_stream->time_base,
                                      static_cast<AVRounding>(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        packet.duration = av_rescale_q(packet.duration, in_stream->time_base, out_stream->time_base);
        // https://ffmpeg.org/doxygen/trunk/structAVPacket.html#ab5793d8195cf4789dfb3913b7a693903
        packet.pos = -1;

        //https://ffmpeg.org/doxygen/trunk/group__lavf__encoding.html#ga37352ed2c63493c38219d935e71db6c1
        // 向封装格式的输出文件写入音视频帧的函数
        ret = av_interleaved_write_frame(output_format_context, &packet);
        if (ret < 0) {
            fprintf(stderr, "Error muxing packet\n");
            break;
        }
        av_packet_unref(&packet);
    }
    //https://ffmpeg.org/doxygen/trunk/group__lavf__encoding.html#ga7f14007e7dc8f481f054b21614dfec13
    av_write_trailer(output_format_context);

    avformat_close_input(&input_format_context);
    /* close output */
    if (output_format_context && !(output_format_context->oformat->flags & AVFMT_NOFILE))
        avio_closep(&output_format_context->pb);
    avformat_free_context(output_format_context);
    av_freep(&streams_list);
    if (ret < 0 && ret != AVERROR_EOF) {
        fprintf(stderr, "Error occurred: %s\n", av_err2str(ret));
        return 1;
    }
    return 0;
}
