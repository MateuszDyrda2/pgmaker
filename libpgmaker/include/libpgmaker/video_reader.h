#pragma once

#include "video.h"
#include <libpgmaker/pg_types.h>

#include <functional>
#include <memory>
#include <string>

namespace libpgmaker {
class video_reader
{
  public:
    struct video_handle
    {
        video_handle& load_metadata();
        video_handle& load_thumbnail();
        std::shared_ptr<video> get();

        ~video_handle();

      private:
        AVFormatContext* pFormatContext;
        AVCodecContext* pCodecContext;
        int vsIndex;
        video_info info;
        std::unique_ptr<std::uint8_t[]> thumbnail;

      private:
        friend class video_reader;
        video_handle(AVFormatContext* ctx, const std::string& path);
    };

    struct video_copier
    {
        video_copier(const std::function<void(void*, void*, int, int)>& effect);
        void open_input(const std::string& path);
        void open_output(const std::string& path);
        void process();
        void transcode_video(AVPacket* inputPacket, AVFrame* inputFrame);
        void encode_video(AVFrame* inputFrame);
        void remux(AVPacket* inputPacket, AVRational inTimebase, AVRational outTimebase);
        void prepare_video_encoder();
        void prepare_audio_encoder();

      private:
        struct clip_params
        {
            AVFormatContext* avfm;
            AVStream *videoStream, *audioStream;
            const AVCodec *videoCodec, *audioCodec;
            AVCodecContext *videoCodecCtx, *audioCodecCtx;
            SwsContext* swsCtx;
            int videoIndex, audioIndex;
        };
        struct streaming_params
        {
            char* muxer_opt_key;
            char* muxer_opt_value;
            char* video_codec;
            char* codec_priv_key;
            char* codec_priv_value;
        };
        clip_params inParams, outParams;
        streaming_params streamingParams;
        std::function<void(void*, void*, int, int)> effect;
        uint8_t *inBuffer, *outBuffer;
    };

  public:
    static video_handle load_file(const std::string& path);

    static void copy_with_effect(
        const std::string& in,
        const std::string& out,
        const std::function<void(void*, void*, int, int)>& effect);
};
} // namespace libpgmaker
