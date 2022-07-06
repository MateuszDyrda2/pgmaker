/** @file */

#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "video.h"
#include <libpgmaker/pg_types.h>

#include "effect.h"
#include <functional>
#include <memory>
#include <string>

namespace libpgmaker {
/** Class for operations with video files */
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
        video_copier(effect* ef);
        ~video_copier();
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
            std::string muxer_opt_key;
            std::string muxer_opt_value;
            std::string video_codec;
            std::string codec_priv_key;
            std::string codec_priv_value;
        };
        clip_params inParams, outParams;
        streaming_params streamingParams;
        effect* ef;
        AVFrame *inBuffer, *outBuffer;
        GLFWwindow* handle;
        size_t nFrames{ 0 };
    };

  public:
    /** @brief Loads a video file
     * @param path path to a file
     * @return handle to the loader
     */
    static video_handle load_file(const std::string& path);
    static void export_clip(
        class clip* c,
        const std::string& path);
    /** @brief Copies a video clip to a new file with effect attached
     * @param in path to input video file
     * @param out path to output
     * @param ef pointer to effect
     */
    static void copy_with_effect(
        const std::string& in,
        const std::string& out,
        effect* ef);
};
} // namespace libpgmaker
